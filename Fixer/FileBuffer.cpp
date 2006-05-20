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

#include "FileBuffer.h"
#include <iterator>
#include <cmath>
#include "EndianHelper.h"

FileBuffer::FileBuffer(const std::string& fileName)
	: m_FileName(fileName)
{
	m_Stream.open(m_FileName.c_str(), std::ios::in | std::ios::binary);
	if(!m_Stream.is_open())
	{
		std::string sError = "Failed To Open File : ";
		sError += m_FileName;
		throw(sError);
	}
	// get length of file:
	m_Stream.seekg(0, std::ios::end);
	m_Length = m_Stream.tellg();
	m_Stream.seekg(0, std::ios::beg);
}


FileBuffer::~FileBuffer()
{
	m_Stream.close();
}

unsigned char FileBuffer::operator [ ]( unsigned int i ) const
{
	const unsigned int iCurrentBufferSize = m_InternalBuffer.size();
	//if this is after our buffer then read more data from the stream
	if(i >= iCurrentBufferSize)
	{
		unsigned char buffer[i - iCurrentBufferSize + 1];
		m_Stream.read(reinterpret_cast<char*>(&buffer), sizeof(buffer));
		m_InternalBuffer.insert(m_InternalBuffer.end(), buffer, buffer + sizeof(buffer));
		if(m_Stream.fail()) throw ("File Read error");
	}
	return m_InternalBuffer[i];
}

bool FileBuffer::proceed( unsigned int i )
{
	const unsigned int iCurrentBufferSize = m_InternalBuffer.size();
	if(i < iCurrentBufferSize)
	{ 
		m_InternalBuffer.erase(m_InternalBuffer.begin(), m_InternalBuffer.begin() + i);
	}
	else
	{
		m_InternalBuffer.clear();
		m_Stream.seekg(i - iCurrentBufferSize, std::ios_base::cur);
	}
	return true;
}


unsigned long FileBuffer::position( ) const
{
	return (static_cast<int>(m_Stream.tellg()) - m_InternalBuffer.size());
}

bool FileBuffer::isDataLeft( ) const
{
	return (!m_InternalBuffer.empty() || (!m_Stream.eof() && (m_Stream.tellg() < m_Length)));
}

bool FileBuffer::setPostion( unsigned long iPos )
{
	m_Stream.seekg(iPos, std::ios_base::beg);
	m_InternalBuffer.clear();
	return true;
}

bool FileBuffer::readIntoBuffer( unsigned char * pBuffer, unsigned int iSize )
{
	assert(pBuffer);
	m_Stream.read(reinterpret_cast<char*>(pBuffer), iSize);
	return true;
}

void FileBuffer::reopen( )
{
	m_Stream.close();
	m_Stream.open(m_FileName.c_str(), std::ios::in | std::ios::binary);
}

bool FileBuffer::CanRead( unsigned int iCount ) const
{
	return (m_Length >= (position() + iCount));
}

bool FileBuffer::DoesSay( std::string text , int iStartingfromByte) const
{
	assert(text.size() > 0);
	for(unsigned int i = 0; i < text.size(); ++i)
	{
		if(text[i] != (*this)[i + iStartingfromByte]) return false;
	}
	return true;
}

unsigned long FileBuffer::GetFromBigEndianToNative( unsigned int iStartingfromByte ) const
{
	unsigned char buffer[4];
	for(unsigned int i = 0; i < sizeof(buffer); ++i)
	{
		buffer[i] = (*this)[iStartingfromByte + i];
	}
	return EndianHelper::ConvertToNativeFromBigEndian(buffer);
}
