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

#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <iosfwd>
#include <fstream> // still needed for std::istream::streamoff
#include <memory>
#include <deque>
#include <string>

class FileBuffer
{
	public:
		typedef std::istream::streamoff pos_type;
		typedef std::istream::streamoff off_type;
		
		FileBuffer(const std::string &fileName);
		virtual ~FileBuffer();

		unsigned char operator [] (off_type i) const;

		unsigned long GetFromBigEndianToNative(off_type iStartingfromByte = 0) const;
		unsigned long GetFromLitleEndianToNative( off_type iStartingfromByte = 0) const;

		bool CanRead(off_type iCount) const;

		bool proceed(off_type i);

		bool setPosition(pos_type iPos);
		bool readIntoBuffer(unsigned char * pBuffer, off_type iSize);

		bool DoesSay(const std::string& sText, off_type iStartingfromByte = 0) const;

		bool isDataLeft() const;

		void reopen();

		pos_type position() const;

		pos_type GetLength() const {return m_Length;}
		
	private:
		const std::auto_ptr<std::ifstream> m_Stream;
		mutable std::deque<unsigned char> m_InternalBuffer;
		const std::string m_FileName;
		pos_type m_Length;
};

#endif
