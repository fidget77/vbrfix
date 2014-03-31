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

#include "ApeTag.h"
#include "FeedBackInterface.h"
#include "FileBuffer.h"
#include "BitReader.h"
#include <string>
#include <cassert>

ApeTag::ApeTag(unsigned long oldFilePosition, unsigned long Size)
	: Mp3Object(oldFilePosition)
	, m_Size(Size)
{
}


ApeTag::~ApeTag()
{
}

ApeTag * ApeTag::Check(CheckParameters & rParams)
{
	const FileBuffer& mp3FileBuffer(rParams.m_mp3FileBuffer);
	
	// TODO introduce a iterator wrapper around the FileBuffer object to improve the code here
	const std::string sStartIdentifier = "APETAGEX";
	
	const unsigned long versionDataByteNo = 4, sizeDataByteNo = 4, itemCountDataByteNo = 4, flagsDataByteNo = 4;
	const unsigned long versionDataOffset = sStartIdentifier.size();
	const unsigned long sizeDataOffset = versionDataOffset + versionDataByteNo;
	const unsigned long flagsDataOffset = sizeDataOffset + sizeDataByteNo + itemCountDataByteNo;
	
	if(mp3FileBuffer.CanRead(sizeDataOffset + sizeDataByteNo))
	{
		if(mp3FileBuffer.DoesSay(sStartIdentifier))
		{
			const unsigned long version = mp3FileBuffer.GetFromLitleEndianToNative(versionDataOffset);
			if(version == 1000 || version == 2000)
			{
				unsigned long size = mp3FileBuffer.GetFromLitleEndianToNative(sizeDataOffset);
				
				if(mp3FileBuffer.CanRead(flagsDataOffset + flagsDataByteNo))
				{
					assert(sizeof(unsigned long) >= flagsDataByteNo);
					
					unsigned long uFlags = mp3FileBuffer.GetFromLitleEndianToNative(flagsDataOffset);
					const IndexMask footerHeaderBit(29, 1);
					if(footerHeaderBit.IsOn( uFlags ))
					{
						size += 32; // add size of header as the read size excludes the header 
						rParams.m_feedBack.addLogMessage(Log::LOG_INFO, "Found Ape Tag");
						return new ApeTag(mp3FileBuffer.position(), size);
					}
					else
					{
						throw "Vbrfix doesn't currently support headerless ApeTags, treating as unknown data";
					}
				}
				else
				{
					rParams.m_feedBack.addLogMessage( Log::LOG_WARNING, "APE tag goes off end of the file, treating as unknown data");
				}
			}
			else
			{
				rParams.m_feedBack.addLogMessage( Log::LOG_WARNING, "APE tag with unknown version found, treating as unknown data");
			}
		}
	}
	return NULL;
}

unsigned long ApeTag::size() const
{
	return m_Size;
}
