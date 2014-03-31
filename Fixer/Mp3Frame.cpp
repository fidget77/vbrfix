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

#include "Mp3Frame.h"
#include "FileBuffer.h"
#include "Mp3Header.h"
#include "FeedBackInterface.h"
#include "ReadSettings.h"
#include "XingFrame.h"
#include "VbriFrame.h"
#include <cassert>

Mp3Frame * Mp3Frame::Check(CheckParameters & rParams)
{
	if(rParams.m_mp3FileBuffer.CanRead(Mp3Header::HEADER_SIZE_IN_BYTES))
	{
		assert(sizeof(unsigned long) >= Mp3Header::HEADER_SIZE_IN_BYTES); // so we can store the header

		unsigned long uHeader = rParams.m_mp3FileBuffer.GetFromBigEndianToNative();
		Mp3Header testMp3Header(uHeader);

		if(testMp3Header.IsValid())
		{
			if(testMp3Header.IsFreeBitrate())
			{
				if(!rParams.m_readSettings.getTreatFreeFormatFramesAsUnknownData())
				{
					throw "Frame uses free bitrate, not supported";
				}
				else
				{
					return NULL;
				}
			}
			else
			{
				if(rParams.m_mp3FileBuffer.CanRead( testMp3Header.GetFrameSize()))
				{
					// what type of frame is it
					Mp3Frame * pFrame = XingFrame::Check(rParams);
					if(!pFrame) pFrame = VbriFrame::Check(rParams);
					if(!pFrame)
					{
						pFrame = new Mp3Frame(rParams.m_mp3FileBuffer.position(), testMp3Header);
					}
					return pFrame;
				}
				else
				{
					rParams.m_feedBack.addLogMessage( Log::LOG_WARNING, "A Frame Runs off the end of the file, treating as unknown data");
				}
			}
		}
	}
	return NULL;
}

Mp3Frame::Mp3Frame(unsigned long oldFilePosition, const Mp3Header &header)
	: Mp3Object(oldFilePosition)
	, m_Header(header)
{
}

Mp3Frame::Mp3Frame( const Mp3Header & header )
	: Mp3Object()
	, m_Header(header)
{
}


Mp3Frame::~Mp3Frame()
{
}

unsigned long Mp3Frame::size( ) const
{
	return m_Header.GetFrameSize();
}

const Mp3Header & Mp3Frame::GetMp3Header( ) const
{
 	return m_Header;
}

Mp3Header & Mp3Frame::GetMp3Header( )
{
	return m_Header;
}
