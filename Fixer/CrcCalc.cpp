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

// Note this code was adapted from code examples found on newsgroups / the internet

#include "CrcCalc.h"

#include <vector>

namespace
{
	typedef std::vector< unsigned short > Crc16Tab;
	void CreateCrcTab(Crc16Tab & tab)
	{
		tab.resize(256);
		for( unsigned int crc_i = 0; crc_i < tab.size(); crc_i++)
		{
			unsigned short crc_data = (crc_i << 1);
			unsigned short crc_crc = 0;
			for(int j = 8; j > 0; j--)
			{
				crc_data >>= 1;
				if( (crc_data ^ crc_crc) & 0x0001)
				{
					crc_crc = (crc_crc >> 1) ^ 0xA001;
				}
				else
				{
					crc_crc >>= 1;
				}
			}
			tab[crc_i] = crc_crc;
		}
		
		return;
	}
}

int CrcHelper::CRC_update_lookup(int value, int crc)
{
	static Crc16Tab tab;
	if(tab.empty())
	{
		CreateCrcTab(tab);
	}
	int tmp = crc ^ value;
	crc = (crc >> 8) ^ tab[tmp & 0xff];
	return crc;
}

int CrcHelper::CrcMp3FrameUpdate(int value, int crc)
{
	const int CRC16_POLYNOMIAL = 0x8005;
	value <<= 8;
	for (int i = 0; i < 8; i++)
	{
		value <<= 1;
		crc <<= 1;

		if (((crc ^ value) & 0x10000))
		{
			crc ^= CRC16_POLYNOMIAL;
		}
	}
	return crc;
}
