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

#ifndef XINGFRAME_H
#define XINGFRAME_H

#include "Mp3Frame.h"
#include <memory>
#include <vector>
#include <list>

class FixerSettings;

class XingFrame : public Mp3Frame
{
	typedef std::list<const Mp3Object*> Mp3ObjectList;
	
	public:
		XingFrame(const Mp3Header & header);

		void Setup(const Mp3ObjectList & finalObjectList, const XingFrame* pOriginalFrame, const FixerSettings &rFixerSettings, FileBuffer & mp3FileBuffer);
		bool isOriginalCorrect(const XingFrame* originalFrame);

		virtual void writeToFile(FileBuffer & originalFile, std::ofstream & rOutFile) const;

		virtual Mp3ObjectType GetObjectType() const {return Mp3ObjectType(Mp3ObjectType::XING_FRAME);}

		static XingFrame * Check(CheckParameters & rParams);

		virtual bool HasLameInfo() const;

		int GetQuality() const {return m_VbrScale;}
		void SetQuality(int quality);

		void SetLameData(const std::vector<unsigned char> &lameData);
		const std::vector<unsigned char>& GetLameData() const;

		static bool IsCrcUpdateSupported(const Mp3Header & header);
		
	protected:
		enum {NO_QUALITY = 0};
		XingFrame(unsigned long iOldFilePos, const Mp3Header & header);

	private:
		typedef unsigned long _ul32;
		std::vector<unsigned char> m_Toc;
		_ul32 m_Flags;
		_ul32 m_FrameCount;
		_ul32 m_StreamSize;
		_ul32 m_VbrScale;

		std::vector<unsigned char> m_LameData;

		bool m_bReCalculateLameCrc;
		bool m_bReCalculateLameMusicCrc;
		int m_musicCRC;
};

#endif
