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

#include "LyricsTag.h"
#include "FeedBackInterface.h"
#include "FileBuffer.h"
#include "BitReader.h"
#include <string>
#include <cassert>

Lyrics3Tag::Lyrics3Tag(unsigned long oldFilePosition, unsigned long Size)
	: Mp3Object(oldFilePosition)
	, m_Size(Size)
{
}


Lyrics3Tag::~Lyrics3Tag()
{
}

Lyrics3Tag * Lyrics3Tag::Check(CheckParameters & rParams)
{
	const FileBuffer& mp3FileBuffer(rParams.m_mp3FileBuffer);
	const std::string sStartIdentifier = "LYRICSBEGIN";
	const std::string sEndIdentifier = "LYRICSEND";
	const std::string sEndIdentifier200 = "LYRICS200";
	const int maxOldTagSizeSize = 5100;
	const int maxNewTagSize = 999999 + sEndIdentifier200.size();

	if(mp3FileBuffer.DoesSay(sStartIdentifier))
	{
		for(int i = sStartIdentifier.size(); i < maxNewTagSize; ++i) // goes a little further than it needs
		{
			if(!mp3FileBuffer.CanRead(i))
			{
				rParams.m_feedBack.addLogMessage(Log::LOG_WARNING, "Partial LYRICSTAG detected, treating as unknown data");
				return NULL;
			}
			if(mp3FileBuffer[i] == 255)
			{
				rParams.m_feedBack.addLogMessage(Log::LOG_WARNING, "Partial LYRICSTAG detected(byte value 255 found in the data), treating as unknown data");
				return NULL;
			}
			if((i < maxOldTagSizeSize) && mp3FileBuffer.DoesSay(sEndIdentifier, i)) // goes a little further than it needs
			{
				rParams.m_feedBack.addLogMessage(Log::LOG_INFO, "Found Lytics3 v1 Tag");
				return new Lyrics3Tag(mp3FileBuffer.position(), i + sEndIdentifier.size());
			}
			if(mp3FileBuffer.DoesSay(sEndIdentifier200, i))
			{
				rParams.m_feedBack.addLogMessage(Log::LOG_INFO, "Found Lyrics3 v2 Tag");
				return new Lyrics3Tag(mp3FileBuffer.position(), i + sEndIdentifier200.size());
			}
		}
	}
	return NULL;
}

unsigned long Lyrics3Tag::size() const
{
	return m_Size;
}
