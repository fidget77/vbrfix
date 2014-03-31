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
#include "FixerSettings.h"
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
	
	const int maxLameBodySize = 208;
	const int lameHeadSize = 20;

	const unsigned int TOC_SIZE = 100;
	const int XING_DATA_SIZE = HEADER_BYTES + HEADER_BYTES + TOC_SIZE + HEADER_BYTES + HEADER_BYTES + HEADER_BYTES;

	const std::string XingIdentifier = "Xing";

	bool TocSimilar(const unsigned char& a, const unsigned char & b)
	{
		return ((std::max(a, b) - std::min(a, b)) < 3);
	}

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
		// TODO I don't think this is correct but should always be the same size regardless of the flags?
		unsigned long size = HEADER_BYTES + HEADER_BYTES; // Xing + Flags sizes
		if(uXingFlags & FRAMES_FLAG) size += HEADER_BYTES;
		if(uXingFlags & BYTES_FLAG) size += HEADER_BYTES;
		if(uXingFlags & TOC_FLAG) size += TOC_SIZE;
		if(uXingFlags & VBR_SCALE_FLAG) size += HEADER_BYTES;
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
	
	unsigned long CalculateFrameCrc(int sideInfoLen, const std::deque<unsigned char> &frameData)
	{
		int crc = 0xffff;
		crc = CrcHelper::CrcMp3FrameUpdate(frameData[2], crc);
		crc = CrcHelper::CrcMp3FrameUpdate(frameData[3], crc);
		for (int i = 6; i < sideInfoLen; i++) 
		{
			crc = CrcHelper::CrcMp3FrameUpdate(frameData[i], crc);
		}
		
		return crc;
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
	assert(TOC_SIZE == m_Toc.size());
	buffer.insert(buffer.end(), m_Toc.begin(), m_Toc.end());
	
	AddAsBigEndian(buffer, m_VbrScale);
	
	// frame crc if crc is on
	if(m_Header.IsProtectedByCrc())
	{
		if(IsCrcUpdateSupported(m_Header))
		{
			const int sideInfoLen = iXingOffset + (m_Header.IsProtectedByCrc() ? 2 : 0);
			const unsigned long uFrameCrc = CalculateFrameCrc(sideInfoLen, buffer);
			const std::vector<unsigned char> bytes = EndianHelper::ConvertToBigEndianBytes(uFrameCrc);
			// put into the buffer after the frame header (only last 2 bytes) as it's 16-bit
			buffer[HEADER_BYTES] = bytes[2];
			buffer[HEADER_BYTES + 1] = bytes[3];
		}
		else
		{
			throw "Xing Frame Crc update not supported on this type of mpeg audio file, please alter the options";
		}
	}

	if(HasLameInfo())
	{
		const unsigned long lamePos = buffer.size();
		
		const unsigned long iCrcPos = lamePos +  0xBE - 0x9C;
		
		// Lame Info

		buffer.insert(buffer.end(), m_LameData.begin(), m_LameData.end());
		if(m_bReCalculateLameMusicCrc)
		{
			if(lamePos != 0x9C || iCrcPos != 0xBE)
			{
				throw "This type of Lame Info is unsupported, Recalculation of LameCrc cannot be performed. Try turrning off Lame CRC recalculation or logging a bug.";
			}

			std::vector<unsigned char> bigEndian = EndianHelper::ConvertToBigEndianBytes(m_musicCRC);
			assert((bigEndian[0] + bigEndian[1]) == 0);
			if((bigEndian[0] + bigEndian[1]) != 0)
			{
				throw "Error Caclulation Lame Music CRC";
			}
			
			buffer[iCrcPos - 2] = bigEndian[2];
			buffer[iCrcPos - 1] = bigEndian[3];
		}

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
	if(size() < buffer.size())
	{
		throw "Internal error with Xing/Lame writing tag";
	}
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
	, m_bReCalculateLameMusicCrc(false)
	, m_musicCRC(0)
{
	assert(header.IsValid());
}

void XingFrame::Setup(const Mp3ObjectList & finalObjectList, const XingFrame* pOriginalFrame, const FixerSettings &rFixerSettings, FileBuffer & mp3FileBuffer)
{
	// Xing Flags
	m_Flags = FRAMES_FLAG | BYTES_FLAG | TOC_FLAG;
	
	if(pOriginalFrame)
	{
		if(pOriginalFrame->m_Flags & VBR_SCALE_FLAG) // only do scale if it existed previously
		{
			m_Flags |= VBR_SCALE_FLAG;
			m_VbrScale = pOriginalFrame->m_VbrScale;
		}

		if(rFixerSettings.KeepLameInfo())
		{
			SetLameData( pOriginalFrame->GetLameData() );
			if(rFixerSettings.recalculateLameTagHeaderCrc())
			{
				m_bReCalculateLameCrc = true;
				if(rFixerSettings.recalculateLameTagHeaderCrcMusic())
				{
					m_bReCalculateLameMusicCrc = true;

					int crc = 0;
					FileBuffer::pos_type oldPos = mp3FileBuffer.position(); // TODO this isn't ideal, some sort of iterator would be good
					// calculate music CRC
					for(Mp3ObjectList::const_iterator iter = finalObjectList.begin(); iter != finalObjectList.end(); ++iter)
					{
						if(((*iter)->GetObjectType().GetObjectId() != Mp3ObjectType::XING_FRAME) && (*iter)->GetObjectType().IsTypeOfFrame() )
						{
							mp3FileBuffer.setPosition((*iter)->getOldFilePosition());
							// TODO progress? This takes some time
							mp3FileBuffer[(*iter)->size() - 1]; // speed up the buffering
								
							for(unsigned long iByte = 0; iByte < (*iter)->size(); ++iByte)
							{
								int val = mp3FileBuffer[iByte];
								crc = CrcHelper::CRC_update_lookup(val, crc);
							}
						}
					}
					mp3FileBuffer.setPosition(oldPos);
					m_musicCRC = crc;
				}
			}
		}
	}
	
	if(GetMp3Header().IsProtectedByCrc())
	{
		bool bRemoveCrc = false;
		switch(rFixerSettings.GetXingFrameCrcOption())
		{
			case FixerSettings::CRC_REMOVE: 
				bRemoveCrc = true; 
				break;
			case FixerSettings::CRC_KEEP_IF_CAN:
				bRemoveCrc = !IsCrcUpdateSupported(GetMp3Header());
				break;
			case FixerSettings::CRC_KEEP: 
				break;
		}

		if(bRemoveCrc)
		{
			GetMp3Header().RemoveCrcProtection();
		}
	}
	

	unsigned int minSize = GetXingHeaderOffset(m_Header) + XING_DATA_SIZE;
	if(HasLameInfo()) minSize += m_LameData.size();
	
	// make sure the frame is big enough 
	while(size() < minSize)
	{
		if(!m_Header.IncreaseBitrate())
			throw ("Can't create an Xing Header Big enough to encapsulate the data");
	}
	
	typedef std::deque<unsigned long> FramePositions;
	FramePositions framePositions;
	unsigned long iNewStreamSize = 0;
	
	// count frames
	for(Mp3ObjectList::const_iterator iter = finalObjectList.begin(); iter != finalObjectList.end(); ++iter)
	{
		if((*iter)->GetObjectType().IsTypeOfFrame())
		{
			framePositions.push_back(iNewStreamSize);
			iNewStreamSize += (*iter)->size(); // only count size of frames, not tags
		}
	}

	m_Toc.clear();
	m_Toc.reserve(TOC_SIZE);

	// Generate TOC
	for(unsigned int iPercent = 0; iPercent < TOC_SIZE; ++iPercent)
	{
		const int iFrame = static_cast<int>((iPercent / double(TOC_SIZE)) * framePositions.size());
		const double dValue = 256 * (framePositions[iFrame] / (1.0 * iNewStreamSize));
		int iBytePercent = int(dValue + 0.5);
		if(iBytePercent > 255) iBytePercent = 255;
		assert(iBytePercent >= 0);
		m_Toc.push_back(iBytePercent);
	}
	assert(m_Toc.size() == TOC_SIZE);

	// Frame Count
	m_FrameCount = framePositions.size() - 1; // - 1 as we don't include this Xing frame, just the music data frames

	// Stream Size
	m_StreamSize = iNewStreamSize;
}

bool XingFrame::isOriginalCorrect(const XingFrame* originalFrame)
{
	bool differ = false;
	differ |= (originalFrame->m_FrameCount != m_FrameCount);
	differ |= (originalFrame->m_Flags != m_Flags);

	// by real we mean excluding the Xing Frame
	const unsigned long originalRealStreamSize = (originalFrame->m_StreamSize - originalFrame->size());
	const unsigned long realStreamSize = (m_StreamSize - size());
	differ |= (originalRealStreamSize != realStreamSize);
	differ |= (originalFrame->m_VbrScale != m_VbrScale);
	if(m_Toc.size() == originalFrame->m_Toc.size())
	{
		differ |= !std::equal(m_Toc.begin(), m_Toc.end(), originalFrame->m_Toc.begin(), TocSimilar);
	}
	else differ = true;

	return !differ;
}

XingFrame * XingFrame::Check(CheckParameters & rParams)
{
	const FileBuffer& mp3FileBuffer(rParams.m_mp3FileBuffer);
	
	// must be called from Mp3Frame::Check() or the Mp3Header might not have been verified and things like that`
	Mp3Header header(mp3FileBuffer.GetFromBigEndianToNative());
	const int iXingHeaderPos = GetXingHeaderOffset(header);
	if(mp3FileBuffer.DoesSay("Info", iXingHeaderPos))
		throw "VBRFix doesn't currently support Lame 'Info' tags as this is usually associated with CBR files";
	if(mp3FileBuffer.DoesSay(XingIdentifier, iXingHeaderPos))
	{
		const unsigned long uXingFlags = mp3FileBuffer.GetFromBigEndianToNative( iXingHeaderPos + XingIdentifier.size());
		const unsigned long uLamePosition = GetLameInfoPosition(iXingHeaderPos, uXingFlags);
		const unsigned long testLamePos = iXingHeaderPos + XING_DATA_SIZE;
		if(testLamePos != uLamePosition)
		{
			if(mp3FileBuffer.DoesSay("LAME", testLamePos))
				throw "Unknown error with LAME info";
		}
		const bool bContainsLameInfo = mp3FileBuffer.DoesSay("LAME", uLamePosition);
		XingFrame *pNewFrame = new XingFrame(mp3FileBuffer.position(), header);
		pNewFrame->m_Flags = uXingFlags;
		if(bContainsLameInfo)
		{
			const int lameLength = (header.GetFrameSize() - uLamePosition);
			
			std::vector< unsigned char > lameInfo(lameLength);
			for(int i = 0 ; i < lameLength; ++i)
			{
				lameInfo[i] = mp3FileBuffer[uLamePosition + i];
			}
			const int maxLameSize = (maxLameBodySize + lameHeadSize);
			for(int i = maxLameSize; i < lameLength; ++i)
			{
				if(lameInfo[i] != 0) 
					throw "Unexpectedly long LAME info tag";
			}

			pNewFrame->SetLameData(lameInfo);
		}
		int iQualPos = iXingHeaderPos + XingIdentifier.size() + sizeof(uXingFlags);
		if(uXingFlags & FRAMES_FLAG) 
		{
			pNewFrame->m_FrameCount = mp3FileBuffer.GetFromBigEndianToNative(iQualPos);
			iQualPos += HEADER_BYTES;
		}
		if(uXingFlags & BYTES_FLAG)
		{
			pNewFrame->m_StreamSize = mp3FileBuffer.GetFromBigEndianToNative(iQualPos);
			iQualPos += HEADER_BYTES;
		}
		if(uXingFlags & TOC_FLAG)
		{
			pNewFrame->m_Toc.resize(TOC_SIZE);
			mp3FileBuffer.getData(pNewFrame->m_Toc, iQualPos);
			iQualPos += TOC_SIZE;
		}
		if(uXingFlags & VBR_SCALE_FLAG)
		{
			pNewFrame->m_VbrScale = mp3FileBuffer.GetFromBigEndianToNative( iQualPos );
		}

		std::string sMsg = "Found Xing Header Frame";
		if(bContainsLameInfo) sMsg += " with LAME info";
		rParams.m_feedBack.addLogMessage( Log::LOG_INFO, sMsg);
		
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

bool XingFrame::IsCrcUpdateSupported( const Mp3Header & header )
{
	return (header.GetLayerVersion() == Mp3Header::LAYER_VERSION_3);
}


