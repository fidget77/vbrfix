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

#include "VbriFrame.h"
#include <cassert>
#include "FileBuffer.h"

VbriFrame::VbriFrame(const Mp3Header & header)
	: Mp3Frame(header)
{
	assert(header.IsValid());
}

VbriFrame::VbriFrame(unsigned long oldFilePosition, const Mp3Header &header)
	: Mp3Frame(oldFilePosition, header)
{
}

VbriFrame::~VbriFrame()
{
}

VbriFrame * VbriFrame::Check(CheckParameters & rParams)
{
	const FileBuffer& mp3FileBuffer(rParams.m_mp3FileBuffer);
	
	// must be called from Mp3Frame::Check() or the Mp3Header might not have been verified and things like that
	
	// TODO Should this be the same as GetXingHeaderOffset
	const unsigned int vbritagPosition = 32 + 4;
	if(mp3FileBuffer.DoesSay("VBRI", vbritagPosition))
	{
		Mp3Header header(mp3FileBuffer.GetFromBigEndianToNative());
		return new VbriFrame(mp3FileBuffer.position(), header);
	}
	return NULL;
}




