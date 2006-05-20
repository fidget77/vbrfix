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

#include "Id3Tags.h"
#include "FileBuffer.h"
#include "FeedBackInterface.h"
#include <sstream>

Id3v1Tag::Id3v1Tag(unsigned long oldFilePosition)
	: Mp3Object(oldFilePosition)
{
}


Id3v1Tag::~Id3v1Tag()
{
}

Id3v1Tag * Id3v1Tag::Check( const FileBuffer & mp3FileBuffer, FeedBackInterface & feedBack)
{
	const std::string sTagIdentifier = "TAG";
	if(mp3FileBuffer.CanRead(sTagIdentifier.size()))
	{
		if(mp3FileBuffer.DoesSay(sTagIdentifier))
		{
			if(mp3FileBuffer.CanRead( ID3V1_TAG_SIZE ))
			{
				feedBack.addLogMessage(Log::LOG_INFO, "Found Id3v1 Tag");
				return new Id3v1Tag(mp3FileBuffer.position());
			}
			else
			{
				feedBack.addLogMessage(Log::LOG_WARNING, "Found an Id3v1 Tag over the end of the file, treating as unknown data");
			}
		}
	}
	return NULL;
}

Id3v2Tag * Id3v2Tag::Check( const FileBuffer & mp3FileBuffer, FeedBackInterface & feedBack)
{
	const std::string sTagIdentifier = "ID3";
	const unsigned long headerBytesMustBeLessThan[] =
	{
		sTagIdentifier[0] + 1, sTagIdentifier[1] + 1,  sTagIdentifier[2] + 1,
		255, 255, 256 /* flags could be better*/, 128, 128, 128, 128
	};
	const unsigned int headerBytesMustBeLessThanSize = sizeof(headerBytesMustBeLessThan) / sizeof(unsigned long);
	if(mp3FileBuffer.CanRead(sTagIdentifier.size() + headerBytesMustBeLessThanSize))
	{
		if(mp3FileBuffer.DoesSay(sTagIdentifier))
		{
			bool bOk = true;
			for(unsigned int i = 0; i < headerBytesMustBeLessThanSize; ++i)
			{
				bOk &= (mp3FileBuffer[i] < headerBytesMustBeLessThan[i]);
			}
			if(bOk)
			{
				unsigned long id3Size = 0;
				id3Size += mp3FileBuffer[6] * 128 * 128 * 128;
				id3Size += mp3FileBuffer[7] * 128 * 128;
				id3Size += mp3FileBuffer[8] * 128;
				id3Size += mp3FileBuffer[9];
				if(id3Size > 10) // TODO check we should have this 
				{
					id3Size += 10; // add header length
					unsigned char uFlags = mp3FileBuffer[5];
					if((uFlags & 32) != 0) // is footer present
					{
						id3Size += 10; // add footer length
					}
					if(mp3FileBuffer.CanRead( id3Size ))
					{
						feedBack.addLogMessage(Log::LOG_INFO, "Found Id3v2 Tag");
						return new Id3v2Tag(mp3FileBuffer.position(), id3Size);
					}
					else
					{
						feedBack.addLogMessage(Log::LOG_WARNING, "Found an Id3v2 Tag over the end of the file, treating as unknown data");
					}
				}
			}
		}
	}
	return NULL;
}

Id3v2Tag::~ Id3v2Tag( )
{
}

Id3v2Tag::Id3v2Tag( unsigned long oldFilePosition, unsigned long Size )
	: Mp3Object(oldFilePosition)
	, m_Size(Size)
{
}

unsigned long Id3v2Tag::size( ) const
{
	return m_Size;
}


