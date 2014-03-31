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

#include "Mp3Reader.h"
#include "Mp3FileObjectChecker.h"
#include "Mp3FileObject.h"
#include "Mp3Frame.h"
#include "Mp3Header.h"
#include "FeedBackInterface.h"
#include "FileBuffer.h"
#include "UnknownDataObject.h"
#include "Id3Tags.h"
#include "ApeTag.h"
#include "LyricsTag.h"
#include <sstream>
#include <cassert>
#include <algorithm>

namespace
{
	struct SDelete
	{
		template <typename T>
		void operator() (T* pPtr)
		{
			delete pPtr;
		}
	};
}

Mp3Reader::Mp3Reader(FileBuffer & mp3FileBuffer, FeedBackInterface &feedBack, ReadProgressDetails& progressDetails, const ReadSettings & readSettings)
	: m_rMp3FileBuffer(mp3FileBuffer)
	, m_rFeedBack(feedBack)
	, m_rProgessDetails(progressDetails)
	, m_readSettings(readSettings)
{
	m_ObjectCheckers.push_back(new Mp3ObjectChecker<Mp3Frame>());
	m_ObjectCheckers.push_back(new Mp3ObjectChecker<Id3v1Tag>());
	m_ObjectCheckers.push_back(new Mp3ObjectChecker<Id3v2Tag>());
	m_ObjectCheckers.push_back(new Mp3ObjectChecker<ApeTag>());
	m_ObjectCheckers.push_back(new Mp3ObjectChecker<Lyrics3Tag>());
}


Mp3Reader::~Mp3Reader()
{
	std::for_each(m_Mp3Objects.begin(), m_Mp3Objects.end(), SDelete());
	std::for_each(m_ObjectCheckers.begin(), m_ObjectCheckers.end(), SDelete());
}

void Mp3Reader::CheckForUnknownData()
{
	unsigned long lastUnderstoodPosition = 0;
	if(!m_Mp3Objects.empty())
	{
		lastUnderstoodPosition = m_Mp3Objects.back()->getOldEndOfObjectFilePosition();
	}
	const unsigned long iPos = m_rMp3FileBuffer.position();
	if(lastUnderstoodPosition != iPos)
	{
		const unsigned long size = iPos - lastUnderstoodPosition;
		UnknownDataObject * pUnknownData = new UnknownDataObject(lastUnderstoodPosition, size);
		m_Mp3Objects.push_back(pUnknownData);
		std::stringstream ss;
		m_rMp3FileBuffer.setPosition(lastUnderstoodPosition);
		ss << "unknown data " << lastUnderstoodPosition << " to " << iPos << " , size = " << size << " (";
		for(unsigned int i = 0; i < std::min(size, 10UL); ++i) ss << m_rMp3FileBuffer[i];
		if(size > 10) 
		{
			ss << "  ...  ";
			for(unsigned int i = std::max(0UL, size - 10); i < size; ++i) ss << m_rMp3FileBuffer[i];
		}
		ss << ")";
		m_rMp3FileBuffer.setPosition(iPos);

		m_rFeedBack.addLogMessage(Log::LogItem(Log::LOG_WARNING, ss.str()));
		m_rProgessDetails.foundObject(pUnknownData);
	}
}

bool Mp3Reader::ReadMp3( )
{
	bool bOk = true;
	const unsigned long FileSize = m_rMp3FileBuffer.GetLength();
	m_rProgessDetails.setFileSize( FileSize);
	while(m_rMp3FileBuffer.isDataLeft() && !m_rFeedBack.HasUserCancelled())
	{
		Mp3Object *pFoundObject = NULL;
		for(ObjectCheckerList::const_iterator iter = m_ObjectCheckers.begin(); iter != m_ObjectCheckers.end(); ++iter)
		{
			// can we identify the object at the position in the filebuffer
			CheckParameters params(m_rMp3FileBuffer, m_rFeedBack, m_readSettings);
			pFoundObject = (*iter)->Check(params);
			if(pFoundObject)
			{
				m_rProgessDetails.foundObject(pFoundObject);
				break;
			}
		}

		m_rProgessDetails.setPercentOfRead(((100 * m_rMp3FileBuffer.position()) / FileSize));

		if(pFoundObject)
		{
			CheckForUnknownData();
			m_Mp3Objects.push_back(pFoundObject);
			m_rMp3FileBuffer.proceed(pFoundObject->size());
			m_rFeedBack.update();
		}
		else
		{
			m_rMp3FileBuffer.proceed(1); // move along the file buffer
		}
	}
	CheckForUnknownData();
	m_rProgessDetails.setPercentOfRead( 100);
	return bOk;
}

const Mp3Reader::ConstMp3ObjectList & Mp3Reader::GetMp3Objects( ) const
{
	return reinterpret_cast<const ConstMp3ObjectList&>(m_Mp3Objects);
}

Mp3Reader::ReadProgressDetails::ReadProgressDetails( )
	: m_iPercentOfRead(0)
	, m_iFrames(0)
	, m_iUnknownData(0)
	, m_Cbr(true)
	, m_TotalBitrate(0)
	, m_LastBitrate(0)
	, m_FileSize(0)
{
}

void Mp3Reader::ReadProgressDetails::foundObject( const Mp3Object * object )
{
	const Mp3ObjectType& eType = object->GetObjectType();
	m_foundObjects.insert(eType);
	if(eType.IsTypeOfFrame())
	{
		m_iFrames ++;
		const Mp3Frame * pFrame = static_cast<const Mp3Frame* >(object);
		const int bitrate = pFrame->GetMp3Header().GetKBitRate();
		m_TotalBitrate += bitrate;
		if(m_iFrames > 1) // we need more than one frame for vbr
		{
			m_Cbr &= (m_LastBitrate == bitrate);
		}
		m_LastBitrate = bitrate;
	}
	else if(eType.IsTypeOfTag())
	{
	}
	else if(eType.IsUnknown())
	{
		m_iUnknownData += object->size();
	}
	
}

int Mp3Reader::ReadProgressDetails::GetPercentUnderstood( ) const
{
	int percent = 0;
	if(m_FileSize > 0)
	{
		assert(m_iUnknownData <= m_FileSize);
		percent = int(((100.0 * (m_FileSize - m_iUnknownData)) / m_FileSize) + 0.5);
		assert(percent >= -20 && percent <= 120); // ball park
		percent = std::max(std::min(percent, 100), 0); // limit 0 - 100
	}
	return percent;
}

unsigned long Mp3Reader::ReadProgressDetails::GetFrameCount( ) const
{
	return m_iFrames;
}

bool Mp3Reader::ReadProgressDetails::IsVbr( ) const
{
	return !m_Cbr;
}

int Mp3Reader::ReadProgressDetails::GetAverageBitrate( ) const
{
	if(m_iFrames == 0) return 0;
	return m_TotalBitrate / m_iFrames;
}

const Mp3ObjectType::Set & Mp3Reader::ReadProgressDetails::GetFoundObjectTypes( ) const
{
	return m_foundObjects;
}
