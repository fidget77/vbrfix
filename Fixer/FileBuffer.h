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

#include <fstream>
#include <deque>

class FileBuffer
{
	public:
		FileBuffer(const std::string &fileName);
		virtual ~FileBuffer();

		unsigned char operator [] (unsigned int i) const;

		unsigned long GetFromBigEndianToNative(unsigned int iStartingfromByte = 0) const;

		bool CanRead(unsigned int iCount) const;

		bool proceed(unsigned int i);

		bool setPostion(unsigned long iPos);
		bool readIntoBuffer(unsigned char * pBuffer, unsigned int iSize);

		bool DoesSay(std::string sText, int iStartingfromByte = 0) const;

		bool isDataLeft() const;

		void reopen();

		unsigned long position() const;

		unsigned long GetLength() const {return m_Length;}
	private:
		mutable std::ifstream m_Stream;
		mutable std::deque<unsigned char> m_InternalBuffer;
		const std::string m_FileName;
		unsigned long m_Length;
};

#endif
