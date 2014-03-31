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

#ifndef MP3READER_H
#define MP3READER_H

#include "Mp3FileObject.h"

#include <list>
#include <istream>
#include <vector>
#include <set>
#include "Mp3Header.h"

class FileBuffer; class Mp3ObjectCheckerInterface; class FeedBackInterface; class ReadSettings;

class Mp3Reader
{
	public:
		class ReadProgressDetails
		{
			typedef std::istream::pos_type pos_type;
			typedef std::istream::off_type off_type;
		
			public:
				ReadProgressDetails();
				int GetPercentUnderstood() const;
				unsigned long GetFrameCount() const;
				int GetAverageBitrate() const;
				bool IsVbr() const;

				const Mp3ObjectType::Set& GetFoundObjectTypes() const;

				void foundObject(const Mp3Object * object);
				void setPercentOfRead(int i) {m_iPercentOfRead = i;}
				void setFileSize(pos_type iFileSize) {m_FileSize = iFileSize;}
			protected:
				int m_iPercentOfRead;
				Mp3ObjectType::Set m_foundObjects;
				unsigned long m_iFrames;
				off_type m_iUnknownData;
				bool m_Cbr; // constant bitrate or variable bitrate
				long m_TotalBitrate;
				long m_LastBitrate;
				pos_type m_FileSize;
				std::vector<Mp3Header::MpegVersion> m_MpegTypes;
				std::vector<Mp3Header::MpegVersion> m_Layers;
		};
		typedef std::list<const Mp3Object*> ConstMp3ObjectList;

		Mp3Reader(FileBuffer & mp3FileBuffer, FeedBackInterface &feedBack, ReadProgressDetails& progessDetails, const ReadSettings & readSettings);
		virtual ~Mp3Reader();

		bool ReadMp3();

		const ConstMp3ObjectList& GetMp3Objects() const;
		
	private:
		typedef std::vector<Mp3ObjectCheckerInterface*> ObjectCheckerList;
		typedef std::list<Mp3Object*> Mp3ObjectList;
		
		Mp3ObjectList m_Mp3Objects;
		ObjectCheckerList m_ObjectCheckers;

		void CheckForUnknownData();

		FileBuffer & m_rMp3FileBuffer;
		FeedBackInterface & m_rFeedBack;
		ReadProgressDetails& m_rProgessDetails;
		const ReadSettings& m_readSettings;
};

#endif
