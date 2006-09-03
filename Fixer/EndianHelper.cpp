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

#include "EndianHelper.h"

namespace EndianHelper
{
	bool IsBigEndian()
	{
		const unsigned long uLong = 1;
		const unsigned char * const pLongPtr = reinterpret_cast<const unsigned char*>(&uLong);
		return (*pLongPtr == 0);
	}

	unsigned long ConvertToBigEndian(unsigned long uLong)
	{
		static const bool BigEndian = IsBigEndian();
		if(BigEndian)
		{
			return uLong; // Already Big Endian 
		}
		else
		{
			const unsigned long newLong = 0 |
				((uLong & 0x000000FF) << (8 * 3)) |
				((uLong & 0x0000FF00) << 8) |
				((uLong & 0x00FF0000) >> 8) |
				((uLong & 0xFF000000) >> (8 * 3));
			return newLong;
		}
	}

	void WriteToFileAsBigEndian( std::ofstream & file, unsigned long uData )
	{
		file << static_cast<unsigned char>(((uData & 0xFF000000) >> 24));
		file << static_cast<unsigned char>(((uData & 0x00FF0000) >> 16));
		file << static_cast<unsigned char>(((uData & 0x0000FF00) >> 8));
		file << static_cast<unsigned char>((uData & 0x000000FF));
	}

	unsigned long ConvertToNativeFromBigEndian(unsigned char* pBuffer )
	{
		unsigned long uResult;
		uResult = pBuffer[0];
		uResult <<= 8;
		uResult |= pBuffer[1];
		uResult <<= 8;
		uResult |= pBuffer[2];
		uResult <<= 8;
		uResult |= pBuffer[3];
		return uResult;
	}
	
	unsigned long ConvertToNativeFromLittleEndian( unsigned char * pBuffer )
	{
		unsigned long uResult;
		uResult = pBuffer[3];
		uResult <<= 8;
		uResult |= pBuffer[2];
		uResult <<= 8;
		uResult |= pBuffer[1];
		uResult <<= 8;
		uResult |= pBuffer[0];
		return uResult;
	}

	std::vector< unsigned char > ConvertToBigEndianBytes( unsigned long uData )
	{
		std::vector<unsigned char> bytes(4);
		bytes[0] = static_cast<unsigned char>(((uData & 0xFF000000) >> 24));
		bytes[1] = static_cast<unsigned char>(((uData & 0x00FF0000) >> 16));
		bytes[2] = static_cast<unsigned char>(((uData & 0x0000FF00) >> 8));
		bytes[3] = static_cast<unsigned char>((uData & 0x000000FF));
		return bytes;
	}
}
