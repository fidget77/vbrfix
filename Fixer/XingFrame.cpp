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
#include "CrcCalc.h"
#include <cmath>
#include <iostream>
#include <iterator>
#include <cassert>

namespace
{
	const unsigned long FRAMES_FLAG = 0x0001;
	const unsigned long BYTES_FLAG = 0x0002;
	const unsigned long TOC_FLAG = 0x0004;
	const unsigned long VBR_SCALE_FLAG = 0x0008;

	const unsigned int HEADER_BYTES = 4;
	const int INVALID_XING_OFFSET = -1;

	const std::string XingIdentifier = "Xing";

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

	void AddAsBigEndian(std::deque<unsigned char> &collection, unsigned long uData)
	{
		std::vector<unsigned char> bytes = EndianHelper::ConvertToBigEndianBytes(uData);
		collection.insert(collection.end(), bytes.begin(), bytes.end());		
	}
}

void XingFrame::writeToFile( FileBuffer & originalFile, std::ofstream & rOutFile ) const
{
	std::deque<unsigned char> buffer;

	if(IsFromFile())
	{
		return Mp3Frame::writeToFile(originalFile, rOutFile);
	}
	using namespace EndianHelper;
	const int iXingOffset = GetXingHeaderOffset(m_Header);
	assert(iXingOffset >= 0);
	if(iXingOffset < 0) throw "Error Creating Xing Tag";

	AddAsBigEndian(buffer, m_Header.GetHeader());
	buffer.insert(buffer.end(), iXingOffset - HEADER_BYTES, '\0');
	buffer.insert(buffer.end(), XingIdentifier.begin(), XingIdentifier.end());
	AddAsBigEndian(buffer, m_Flags);
	AddAsBigEndian(buffer, m_FrameCount);
	AddAsBigEndian(buffer, m_StreamSize);
	assert(100 == m_Toc.size());
	buffer.insert(buffer.end(), m_Toc.begin(), m_Toc.end());
	
	AddAsBigEndian(buffer, m_VbrScale);

	if(HasLameInfo())
	{
		const unsigned long lamePos = buffer.size();
		// Lame Info
		buffer.insert(buffer.end(), m_LameData.begin(), m_LameData.end());

		const unsigned long iCrcPos = lamePos + 0xBE - 0x9C;

		if(m_bReCalculateLameCrc)
		{
			if(lamePos != 0x9C || iCrcPos != 0xBE)
			{
				throw "This type of Lame Info is unsupported, Recalculation of LameCrc cannot be performed. Try turrning off Lame CRC recalculation or logging a bug.";
			}
			
			// Calculate the CRC
			assert(buffer.size() >= iCrcPos);
			if(buffer.size() < iCrcPos)
			{
				throw "Error Calculating Lame Crc";
			}
			int crc = 0;
			for(unsigned long iByte = 0; iByte < iCrcPos; ++iByte)
			{
				int val = buffer[iByte];
				crc = CrcHelper::CRC_update_lookup(val, crc);
			}
			std::vector<unsigned char> bigEndian = EndianHelper::ConvertToBigEndianBytes(crc);
			assert((bigEndian[0] + bigEndian[1]) == 0);
			if((bigEndian[0] + bigEndian[1]) != 0)
			{
				throw "Error Caclulation Lame CRC";
			}
			
			buffer[iCrcPos] = bigEndian[2];
			buffer[iCrcPos + 1] = bigEndian[3];
		}
	}
	
	// rest of space of frame
	const unsigned long iRest = size() - buffer.size();
	buffer.insert(buffer.end(), iRest, '\0');

	std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<unsigned char>(rOutFile));
}

XingFrame::XingFrame(const Mp3Header & header)
	: Mp3Frame(header)
	, m_Flags(0)
	, m_FrameCount(0)
	, m_StreamSize(0)
	, m_VbrScale(0)
	, m_bReCalculateLameCrc(false)
{
	assert(header.IsValid());
}

void XingFrame::Setup(const Mp3ObjectList & finalObjectList)
{
	unsigned int minSize = GetXingHeaderOffset(m_Header) + 120;
	if(HasLameInfo())
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
		iNewFileSize += (*iter)->size();
	}

	m_Toc.clear();
	m_Toc.reserve(100);

	// Generate TOC
	for(int iPercent = 0; iPercent < 100; ++iPercent)
	{
		int iFrame = static_cast<int>((iPercent / 100.0) * framePositions.size());
		m_Toc.push_back(((256 * framePositions[iFrame]) / iNewFileSize));
	}
	assert(m_Toc.size() == 100);

	// Xing Flags
	m_Flags = FRAMES_FLAG | BYTES_FLAG | TOC_FLAG | VBR_SCALE_FLAG;

	// Frame Count
	m_FrameCount = framePositions.size() - 1; // - 1 as we don't include this Xing frame, just the music data frames

	// Stream Size
	m_StreamSize = iNewFileSize;
}

XingFrame * XingFrame::Check( const FileBuffer & mp3FileBuffer, FeedBackInterface & feedBack )
{
	// must be called from Mp3Frame::Check() or the Mp3Header might not have been verified and things like that`
	Mp3Header header(mp3FileBuffer.GetFromBigEndianToNative());
	const int iXingHeaderPos = GetXingHeaderOffset(header);
	if(mp3FileBuffer.DoesSay( XingIdentifier, iXingHeaderPos))
	{
		const unsigned long uXingFlags = mp3FileBuffer.GetFromBigEndianToNative( iXingHeaderPos + XingIdentifier.size());
		const unsigned long uLamePosition = GetLameInfoPosition(iXingHeaderPos, uXingFlags);
		const bool bContainsLameInfo = mp3FileBuffer.DoesSay("LAME", uLamePosition);
		XingFrame *pNewFrame = new XingFrame(mp3FileBuffer.position(), header);
		if(bContainsLameInfo)
		{
			const int LAME_LENGTH = (header.GetFrameSize() - uLamePosition);
			std::vector< unsigned char > lameInfo(LAME_LENGTH);
			for(int i = 0 ; i < LAME_LENGTH; ++i)
			{
				lameInfo[i] = mp3FileBuffer[uLamePosition + i];
			}
			pNewFrame->SetLameData(lameInfo);
		}
		if(uXingFlags & VBR_SCALE_FLAG)
		{
			int iQualPos = iXingHeaderPos + XingIdentifier.size() + 4; // +4 is the size of the flags
			if(uXingFlags & FRAMES_FLAG) iQualPos += 4;
			if(uXingFlags & BYTES_FLAG) iQualPos += 4;
			if(uXingFlags & TOC_FLAG) iQualPos += 100;
			int quality = mp3FileBuffer.GetFromBigEndianToNative( iQualPos );
			pNewFrame->SetQuality(quality);
		}

		std::string sMsg = "Found Xing Header Frame";
		if(bContainsLameInfo) sMsg += " with LAME info";
		feedBack.addLogMessage( Log::LOG_INFO, sMsg);
		
		return pNewFrame;
	}
	return NULL;
}

XingFrame::XingFrame( unsigned long iOldFilePos, const Mp3Header & header)
	: Mp3Frame(iOldFilePos, header)
	, m_VbrScale(NO_QUALITY)
{
}

bool XingFrame::HasLameInfo( ) const
{
	return (!m_LameData.empty());
}

void XingFrame::SetQuality( int quality )
{
	m_VbrScale = quality;
}

void XingFrame::SetLameData( const std::vector< unsigned char > & lameData )
{
	m_LameData = lameData;
}

const std::vector< unsigned char > & XingFrame::GetLameData( ) const
{
	return m_LameData;
}


