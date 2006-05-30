/*//////////////////////////////////////////////////////////////////////////////////
// copyright : (C) 2006  by William Pye
// contact   : www.willwap.co.uk
///////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
///////////////////////////////////////////////////////////////////////////////////*/

#include "XingFrame.h"
#include <deque>
#include "Mp3Header.h"
#include "EndianHelper.h"
#include "FileBuffer.h"
#include "FeedBackInterface.h"

namespace
{
	const unsigned long FRAMES_FLAG = 0x0001;
	const unsigned long BYTES_FLAG = 0x0002;
	const unsigned long TOC_FLAG = 0x0004;
	const unsigned long VBR_SCALE_FLAG = 0x0008;

	const unsigned int HEADER_BYTES = 4;
	const int INVALID_XING_OFFSET = -1;

	int GetXingHeaderOffset(Mp3Header header)
	{
		if(header.IsValid())
		{
			const bool bMono = (header.GetChannelMode() == Mp3Header::CHAN_MONO);
			switch(header.GetMpegVersion())
			{
				case Mp3Header::MPEG_VERSION_1:
					return (bMono ? (17 + 4) : (32 + 4));
					break;
				case Mp3Header::MPEG_VERSION_2:
				case Mp3Header::MPEG_VERSION_2_5:
					return (bMono ? (9 + 4) : (17 + 4));
					break;
				case Mp3Header::MPEG_VERSION_RESERVED:
					break;
			}
		}
		return INVALID_XING_OFFSET;
	}
	unsigned long GetXingDataSize(unsigned long uXingFlags)
	{
		unsigned long size = 8; // Xing(4) + Flags(4) sizes
		if(uXingFlags & FRAMES_FLAG) size += 4;
		if(uXingFlags & BYTES_FLAG) size += 4;
		if(uXingFlags & TOC_FLAG) size += 100;
		if(uXingFlags & VBR_SCALE_FLAG) size += 4;
		return size;
	}
	unsigned long GetLameInfoPosition(int iXingHeaderPos, unsigned long uXingFlags)
	{
		assert(iXingHeaderPos > 0);
		unsigned long iPos = iXingHeaderPos;
		return iPos + GetXingDataSize(uXingFlags);
	}
}

// TODO Copy Lame Part of old tag

void XingFrame::writeToFile( FileBuffer & originalFile, std::ofstream & rOutFile ) const
{
	if(IsFromFile())
	{
		return Mp3Frame::writeToFile(originalFile, rOutFile);
	}
	using namespace EndianHelper;
	const std::streampos iOutFileStartPos = rOutFile.tellp();
	const int iXingOffset = GetXingHeaderOffset(m_Header);
	assert(iXingOffset >= 0);
	if(iXingOffset < 0) throw "Error Creating Xing Tag";

	WriteToFileAsBigEndian(rOutFile, m_Header.GetHeader());
	for(unsigned int i = 0; i < (iXingOffset - HEADER_BYTES); ++i)
	{
		rOutFile << '\0';
	}
	rOutFile << 'X' << 'i' << 'n' << 'g';
	WriteToFileAsBigEndian(rOutFile, m_Flags);
	WriteToFileAsBigEndian(rOutFile, m_FrameCount);
	WriteToFileAsBigEndian(rOutFile, m_StreamSize);
	int i = m_Toc.size();
	assert(i == 100);
	
	for(std::vector<unsigned char>::const_iterator tocIter = m_Toc.begin(); tocIter != m_Toc.end(); ++ tocIter)
	{
		rOutFile << *tocIter;
	}
	WriteToFileAsBigEndian(rOutFile, m_VbrScale);
	
	// Lame Info
	if(m_pOriginalFrame && m_pOriginalFrame->HasLameInfo())
	{
		const unsigned long lameOffset = m_pOriginalFrame->GetLameOffset();
		const unsigned long uOldFileLamePosition = m_pOriginalFrame->getOldFilePosition() + lameOffset;
		const unsigned long uOldLameSize = m_pOriginalFrame->size() - lameOffset;
		const unsigned long iRestOfSpace = size() - (rOutFile.tellp() - iOutFileStartPos);
		originalFile.setPostion(uOldFileLamePosition);
		for(unsigned long i = 0; i < iRestOfSpace && i < uOldLameSize; ++i)
		{
			rOutFile << originalFile[i];
		}
	}

	// rest of space of frame
	const unsigned long iRest = size() - (rOutFile.tellp() - iOutFileStartPos);
	for(unsigned long i = 0; i < iRest; ++i)
	{
		rOutFile << '\0';
	}
}

XingFrame::XingFrame(const Mp3Header & header)
	: Mp3Frame(header)
	, m_Flags(0)
	, m_FrameCount(0)
	, m_StreamSize(0)
	, m_VbrScale(0)
	, m_LameInfoOffset(0)
	, m_pOriginalFrame(NULL)
{
	assert(header.IsValid());
}

void XingFrame::Setup( const Mp3ObjectList & finalObjectList, const XingFrame * pXingFrame )
{
	m_pOriginalFrame = pXingFrame;

	unsigned int minSize = GetXingHeaderOffset(m_Header) + 120;
	const bool bHasLameInfo = pXingFrame && pXingFrame->HasLameInfo();
	if(bHasLameInfo)
	{
		minSize += 20 + 208;
	}
	// make sure the frame is big enough 
	while(size() < minSize)
	{
		if(!m_Header.IncreaseBitrate())
			throw ("Can't create an Xing Header Big enough to encapsulate the data");
	}
	
	typedef std::deque<unsigned long> FramePositions;
	FramePositions framePositions;
	unsigned long iNewFileSize = 0;
	
	// count frames
	for(Mp3ObjectList::const_iterator iter = finalObjectList.begin(); iter != finalObjectList.end(); ++iter)
	{
		if((*iter)->GetObjectType().IsTypeOfFrame())
		{
			framePositions.push_back(iNewFileSize);
		}
		iNewFileSize +=  (*iter)->size();
	}

	m_Toc.clear();
	m_Toc.reserve(100);

	// Generate TOC
	for(int iPercent = 0; iPercent < 100; ++iPercent)
	{
		int iFrame = static_cast<int>((iPercent / 100.0) * framePositions.size());
		m_Toc.push_back(((255 * framePositions[iFrame]) / iNewFileSize));
	}
	assert(m_Toc.size() == 100);

	// Xing Flags
	m_Flags = FRAMES_FLAG | BYTES_FLAG | TOC_FLAG | VBR_SCALE_FLAG;

	// Frame Count
	m_FrameCount = framePositions.size();

	// Stream Size
	m_StreamSize = iNewFileSize;

	// Vbr Scale / Quality 
	m_VbrScale = 0; // we can't really calclate this
	// Copy this from an old Xing tag if we found one
	if(m_pOriginalFrame)
	{
		m_VbrScale = m_pOriginalFrame->GetQuality();
	}
}

XingFrame * XingFrame::Check( const FileBuffer & mp3FileBuffer, FeedBackInterface & feedBack )
{
	// must be called from Mp3Frame::Check() or the Mp3Header might not have been verified and things like that`
	Mp3Header header(mp3FileBuffer.GetFromBigEndianToNative());
	const int iXingHeaderPos = GetXingHeaderOffset(header);
	const std::string XingIdentifier = "Xing";
	if(mp3FileBuffer.DoesSay( XingIdentifier, iXingHeaderPos))
	{
		const unsigned long uXingFlags = mp3FileBuffer.GetFromBigEndianToNative( iXingHeaderPos + XingIdentifier.size());
		const unsigned long uLamePosition = GetLameInfoPosition(iXingHeaderPos, uXingFlags);
		const bool bContainsLameInfo = mp3FileBuffer.DoesSay( "LAME", uLamePosition);
		std::string sMsg = "Found Xing Header Frame";
		if(bContainsLameInfo) sMsg += " with LAME info";
		feedBack.addLogMessage( Log::LOG_INFO, sMsg);
		unsigned long quality = NO_QUALITY;
		if(uXingFlags & VBR_SCALE_FLAG)
		{
			int iQualPos = iXingHeaderPos + XingIdentifier.size() + 4; // +4 is the size of the flags
			if(uXingFlags & FRAMES_FLAG) iQualPos += 4;
			if(uXingFlags & BYTES_FLAG) iQualPos += 4;
			if(uXingFlags & TOC_FLAG) iQualPos += 100;
			quality = mp3FileBuffer.GetFromBigEndianToNative( iQualPos);
		}
		
		return new XingFrame( mp3FileBuffer.position(), header, bContainsLameInfo ? uLamePosition : 0, quality);
	}
	return NULL;
}

XingFrame::XingFrame( unsigned long iOldFilePos, const Mp3Header & header, unsigned int LameInfoAt, unsigned long quality)
	: Mp3Frame(iOldFilePos, header)
	, m_VbrScale(quality)
	, m_LameInfoOffset(LameInfoAt)
{
}

bool XingFrame::HasLameInfo( ) const
{
	return (m_LameInfoOffset != 0);
}


