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

#ifndef ID3V1TAG_H
#define ID3V1TAG_H

#include "Mp3FileObject.h"

class Id3v1Tag : public Mp3Object
{
	private:
		enum {ID3V1_TAG_SIZE = 128};
	public:
		virtual ~Id3v1Tag();
		static Id3v1Tag* Check(CheckParameters & rParams);

		virtual unsigned long size() const {return ID3V1_TAG_SIZE;}
		virtual Mp3ObjectType GetObjectType() const {return Mp3ObjectType(Mp3ObjectType::ID3V1_TAG);}
	protected:
		Id3v1Tag(unsigned long oldFilePosition);

};

class Id3v2Tag : public Mp3Object
{
	public:
		virtual ~Id3v2Tag();
		static Id3v2Tag* Check(CheckParameters & rParams);

		virtual unsigned long size() const;
		virtual Mp3ObjectType GetObjectType() const {return Mp3ObjectType(Mp3ObjectType::ID3V2_TAG);}
	protected:
		Id3v2Tag(unsigned long oldFilePosition, unsigned long Size);
		
	private:
		const unsigned long m_Size;

};

#endif
