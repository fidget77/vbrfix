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

#include "Mp3FileObject.h"
#include "FileBuffer.h"
#include <algorithm>
#include <functional>

void Mp3Object::writeToFile( FileBuffer & originalFile, std::ofstream & rOutFile ) const
{
	assert(IsFromFile());
	if(IsFromFile())
	{
		const unsigned long iObjectSize = size();
		const unsigned long iObjectStartPos = getOldFilePosition();
		unsigned char* buffer[iObjectSize];
		originalFile.setPostion(iObjectStartPos);
		originalFile.readIntoBuffer(reinterpret_cast<unsigned char*>(&buffer), iObjectSize);
		rOutFile.write(reinterpret_cast<char*>(&buffer), iObjectSize);
	}
}

Mp3Object::Mp3Object( )
	: m_IsFromFile(false)
	, m_OldFilePosition(0)
{
}

Mp3Object::Mp3Object( unsigned long iFromFilePosition )
	: m_IsFromFile(true)
	, m_OldFilePosition(iFromFilePosition)
{
}

bool Mp3Object::IsFromFile( ) const
{
	return m_IsFromFile;
}

unsigned long Mp3Object::getOldFilePosition( ) const
{
	assert(IsFromFile());
	return m_OldFilePosition;
}

unsigned long Mp3Object::getOldEndOfObjectFilePosition( ) const
{
	return getOldFilePosition() + size();
}

Mp3Object::~ Mp3Object( )
{
}

bool Mp3ObjectType::operator < ( const Mp3ObjectType & rOther ) const
{
	return m_Type < rOther.m_Type;
}

Mp3ObjectType::Mp3ObjectType( ObjectId type )
	: m_Type(type)
{
}

/*Mp3ObjectType::Mp3ObjectType( const Mp3ObjectType & rOther )
	: m_Type(rOther.m_Type)
{
}*/

bool Mp3ObjectType::IsTypeOfFrame( ) const
{
	return (m_Type == FRAME || m_Type == XING_FRAME);
}

bool Mp3ObjectType::IsTypeOfTag( ) const
{
	return (m_Type == ID3V1_TAG || m_Type == ID3V2_TAG);
}

bool Mp3ObjectType::IsUnknown( ) const
{
	return (m_Type == UNKNOWN_DATA);
}

const Mp3ObjectType::Set & Mp3ObjectType::GetFrameTypes( )
{
	static Set types;
	if(types.empty())
	{
		const Set& allTypes = GetTypes();
		std::remove_copy_if(allTypes.begin(), allTypes.end(), inserter(types, types.begin()), std::not1(std::mem_fun_ref(&Mp3ObjectType::IsTypeOfFrame)));
	}
	return types;
}

const Mp3ObjectType::Set & Mp3ObjectType::GetTagTypes( )
{
	static Set types;
	if(types.empty())
	{
		const Set& allTypes = GetTypes();
		std::remove_copy_if(allTypes.begin(), allTypes.end(), inserter(types, types.begin()), std::not1(std::mem_fun_ref(&Mp3ObjectType::IsTypeOfTag)));
	}
	return types;
}

const Mp3ObjectType::Set & Mp3ObjectType::GetTypes( )
{
	static Set types;
	if(types.empty())
	{
		types.insert(FRAME);
		types.insert(ID3V1_TAG);
		types.insert(ID3V2_TAG);
		types.insert(UNKNOWN_DATA);
	}
	return types;
}
