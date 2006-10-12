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

#ifndef MP3FRAME_H
#define MP3FRAME_H

#include "Mp3FileObject.h"
#include <memory>
#include "Mp3Header.h"

class Mp3Header;

class Mp3Frame : public Mp3Object
{
	public:
		
		Mp3Frame(const Mp3Header &header);
		virtual ~Mp3Frame();

		static Mp3Frame* Check(CheckParameters & rParams);

		virtual unsigned long size() const;

		virtual Mp3ObjectType GetObjectType() const {return Mp3ObjectType(Mp3ObjectType::FRAME);}

		const Mp3Header& GetMp3Header() const;
		Mp3Header& GetMp3Header();

		virtual bool HasLameInfo() const {return false;} // this could change if we handle the info tag
		
	protected:
		Mp3Frame(unsigned long oldFilePosition, const Mp3Header &header);
		Mp3Header m_Header;
	private:
		Mp3Frame(const Mp3Frame& rOther);
		void operator = (const Mp3Frame& rOther);

};

#endif
