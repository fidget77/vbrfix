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

#include "VbrFixer.h"
#include "Mp3Reader.h"
#include "Mp3Header.h"
#include "FileBuffer.h"
#include "Mp3FileObject.h"
#include "FeedBackInterface.h"
#include "FixerSettings.h"
#include "XingFrame.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <map>

namespace
{
	class IsOfMp3ObjectType
	{
		private:
			const Mp3ObjectType::Set& m_Types;
			Mp3ObjectType::Set m_IntTypes;
		public:
			bool operator () (const Mp3Object *pObject)
			{
				return (m_Types.find(pObject->GetObjectType()) != m_Types.end());
			}
			IsOfMp3ObjectType(const Mp3ObjectType::Set& types)
				: m_Types(types) {}
				
			IsOfMp3ObjectType(const Mp3ObjectType::ObjectId type)
				: m_Types(m_IntTypes)
			{
				m_IntTypes.insert(type);
			}
	};

	class FindLameInfoFrame
	{
		public:
			bool operator () (const Mp3Object *pObject)
			{
				if(pObject->GetObjectType().IsTypeOfFrame())
				{
					assert(dynamic_cast<const Mp3Frame*>(pObject));
					if(static_cast<const Mp3Frame*>(pObject)->HasLameInfo())
					{
						return true;
					}
				}
				return false;
			}
	};
}

VbrFixer::VbrFixer( FeedBackInterface & rFeedBackInterface, const FixerSettings & fixerSettings )
	: m_rFeedBackInterface(rFeedBackInterface)
	, m_rFixerSettings(fixerSettings)
{
}


VbrFixer::~VbrFixer()
{
}

struct ConsistencyChecker
{
	ConsistencyChecker() : mostPopularFrameHeader(0){}
	Mp3Header mostPopularFrameHeader;
	
	void workOn(Mp3Reader::ConstMp3ObjectList& objects, bool remove, FeedBackInterface & feedBack)
	{
		threshold = objects.size() / 100;
		readMode = true;
		for(Mp3Reader::ConstMp3ObjectList::iterator iter = objects.begin(); iter != objects.end(); ++iter)
			(*this)(*iter);
		
		std::stringstream headersText; headersText << "Found MP3 headers: ";
		int maxCount = 0;
		for(std::map<Mp3Header, int>::const_iterator headerI = headers.begin(); headerI != headers.end(); ++headerI)
		{
			int count = headerI->second;
			headersText << std::dec <<"[x" << count << "]0x" << std::hex << std::uppercase << headerI->first.GetHeader() << " ";
			maxCount = std::max(count, maxCount);
			if(maxCount == count) mostPopularFrameHeader = headerI->first;
		}
		feedBack.addLogMessage(Log::LOG_DETAIL, headersText.str());
		if(layers.size() > 1)
			feedBack.addLogMessage(Log::LOG_WARNING, "Found inconsistent frame layer versions");
		if(versions.size() > 1)
			feedBack.addLogMessage(Log::LOG_WARNING, "Found inconsistent frame MPEG versions");
		if(frequencies.size() > 1)
			feedBack.addLogMessage(Log::LOG_WARNING, "Found inconsistent frame Sample Frequencies");

		if(remove)
		{
			readMode = false;
			objects.erase(std::remove_if(objects.begin(), objects.end(), *this), objects.end());
		}
	}

	std::map<Mp3Header, int> headers;
	std::map<int, int> frequencies;
	std::map<Mp3Header::MpegLayerVersion, int> layers;
	std::map<Mp3Header::MpegVersion, int> versions;
	bool readMode;
	int threshold;

	bool operator()(const Mp3Object* obj)
	{
		if(obj->GetObjectType().IsTypeOfFrame())
		{
			Mp3Header header = static_cast<const Mp3Frame&>(*obj).GetMp3Header();
			if(readMode)
			{
				headers[header]++;
				frequencies[header.GetSampleFrequency()]++;
				layers[header.GetLayerVersion()]++;
				versions[header.GetMpegVersion()]++;
			}
			else
			{
				if(frequencies[header.GetSampleFrequency()] < threshold) return true;
				if(layers[header.GetLayerVersion()] < threshold) return true;
				if(versions[header.GetMpegVersion()] < threshold) return true;
			}
		}
		return false;
	}
};

void VbrFixer::Fix( const std::string & sInFileName, const std::string & sOutFileName )
{
	try
	{
		std::stringstream ss;
		ss << "Starting Fix : " << sInFileName;
		m_rFeedBackInterface.addLogMessage(Log::LOG_INFO, ss.str());
	
		// Process the original mp3
		FileBuffer inFile(sInFileName);
		m_ProgressDetails.SetState(FixState::READING);
		Mp3Reader mp3Reader(inFile, m_rFeedBackInterface, m_ProgressDetails, m_rFixerSettings);
		mp3Reader.ReadMp3();

		m_rFeedBackInterface.addLogMessage(Log::LOG_INFO, "Finished Reading Mp3 Structure");
		m_ProgressDetails.SetState(FixState::PROCESSING);

		const Mp3Reader::ConstMp3ObjectList& OriginalMp3Objects = mp3Reader.GetMp3Objects();
		Mp3Reader::ConstMp3ObjectList Mp3Objects = OriginalMp3Objects;

		ConsistencyChecker consistencyChecker;

		consistencyChecker.workOn(Mp3Objects, m_rFixerSettings.removeInconsistentFrames(), m_rFeedBackInterface);

		m_ProgressDetails.setPercentOfProcessing(20);
		m_rFeedBackInterface.update();

		// Check if we should Skip this Mp3
		if(ShouldSkipMp3(Mp3Objects))
		{
			m_ProgressDetails.SetState(FixState::SKIPPED);
			return;
		}
	
		if(m_rFeedBackInterface.HasUserCancelled())
		{
			m_ProgressDetails.SetState(FixState::CANCELLED);
			return;
		}

		m_ProgressDetails.setPercentOfProcessing(40);
		m_rFeedBackInterface.update();
	
		// remove unwanted objects
		if(!m_rFixerSettings.GetRemovingDataTypes().empty())
		{
			Mp3Objects.erase(std::remove_if(
				Mp3Objects.begin(),
				Mp3Objects.end(),
				IsOfMp3ObjectType(m_rFixerSettings.GetRemovingDataTypes())
			), Mp3Objects.end());
		}

		m_ProgressDetails.setPercentOfProcessing(60);
		m_rFeedBackInterface.update();
	
		if(m_rFeedBackInterface.HasUserCancelled())
		{
			m_ProgressDetails.SetState(FixState::CANCELLED);
			return;
		}
	
		// insert a vbr header object if needed before the 1st frame
		std::auto_ptr<XingFrame> xingFrame;
		if(m_ProgressDetails.IsVbr())
		{
			Mp3Reader::ConstMp3ObjectList::iterator firstFrame = std::find_if(Mp3Objects.begin(), Mp3Objects.end(), IsOfMp3ObjectType(Mp3ObjectType::GetFrameTypes()));
			assert(firstFrame != Mp3Objects.end()); // as it is vbr there must be a first frame
			xingFrame.reset(new XingFrame(consistencyChecker.mostPopularFrameHeader));
			Mp3Objects.insert(firstFrame, xingFrame.get());
		}
	
		// TODO order the objects
	
		if(m_rFeedBackInterface.HasUserCancelled())
		{
			m_ProgressDetails.SetState(FixState::CANCELLED);
			return;
		}

		m_ProgressDetails.setPercentOfProcessing(100);
		m_rFeedBackInterface.update();
	
		// Create a new mp3
		inFile.reopen(); // we have to reopen the file as it reached the end and can't continue to be used
		std::ofstream outFile;
		outFile.open(sOutFileName.c_str(), std::ios::out | std::ios::binary);
	
		// Fill the Xing Frame once we know the file positions will not change again
		// This must occur dicectly before writing the Mp3
		if(xingFrame.get())
		{
			const XingFrame * pOriginalFrame = NULL;
			if(m_rFixerSettings.KeepLameInfo())
			{
				Mp3Reader::ConstMp3ObjectList::const_iterator lameFrameIter = std::find_if(OriginalMp3Objects.begin(), OriginalMp3Objects.end(), FindLameInfoFrame());
				if(lameFrameIter != OriginalMp3Objects.end())
				{
					pOriginalFrame = static_cast<const XingFrame* >(*lameFrameIter);
				}
			}
			if(!pOriginalFrame)
			{
				Mp3Reader::ConstMp3ObjectList::const_iterator xingFrameIter = std::find_if(OriginalMp3Objects.begin(), OriginalMp3Objects.end(), IsOfMp3ObjectType(Mp3ObjectType::XING_FRAME));
				if(xingFrameIter != OriginalMp3Objects.end())
				{
					pOriginalFrame = static_cast<const XingFrame* >(*xingFrameIter);
				}
			}
			
			xingFrame->Setup(Mp3Objects, pOriginalFrame, m_rFixerSettings);
			if(pOriginalFrame && m_rFixerSettings.skipIfXingTagLooksGood())
			{
				if(xingFrame->isOriginalCorrect(pOriginalFrame))
				{
					m_ProgressDetails.SetState(FixState::SKIPPED);
					m_rFeedBackInterface.addLogMessage( Log::LOG_INFO, "Original Xing Frame looks OK - skipping");
					return;
				}
			}
		}

		m_ProgressDetails.SetState(FixState::WRITING);
	
		// write the objects to the new file
		unsigned long iFileSizeTotal = 0;
		unsigned long iStreamSize = 0;
		int iObjectsWritten = 0; const int iTotalObjects = Mp3Objects.size();
		for(Mp3Reader::ConstMp3ObjectList::const_iterator iter = Mp3Objects.begin(); iter != Mp3Objects.end(); ++iter)
		{
			(*iter)->writeToFile(inFile, outFile);
			iFileSizeTotal += (*iter)->size();

			if((*iter)->GetObjectType().IsTypeOfFrame())
			{
				iStreamSize += (*iter)->size(); // also count size of stream
			}
			
			if(m_rFeedBackInterface.HasUserCancelled())
			{
				m_ProgressDetails.SetState(FixState::CANCELLED);
				return;
			}
			m_ProgressDetails.setPercentOfWriting( ++iObjectsWritten / iTotalObjects );
			m_rFeedBackInterface.update();
		}
	
		if(m_rFeedBackInterface.HasUserCancelled())
		{
			m_ProgressDetails.SetState(FixState::CANCELLED);
			return;
		}

		ss.str("");
		ss << "New File Size = " << iFileSizeTotal << ", Audio Stream Size = " << iStreamSize;
		m_rFeedBackInterface.addLogMessage(Log::LOG_INFO, ss.str());
		
		outFile.close();
	
		m_rFeedBackInterface.addLogMessage(Log::LOG_INFO, "Finished Fix");
		m_ProgressDetails.SetState(FixState::FIXED);
		m_ProgressDetails.setPercentOfWriting(100);
		m_rFeedBackInterface.update();
	}
	catch(const char* pMsg)
	{
		m_ProgressDetails.SetState(FixState::ERROR);
		m_rFeedBackInterface.addLogMessage( Log::LOG_ERROR, pMsg);
	}
	catch(std::string s)
	{
		m_ProgressDetails.SetState(FixState::ERROR);
		m_rFeedBackInterface.addLogMessage( Log::LOG_ERROR, s);
	}
	catch(...)
	{
		m_ProgressDetails.SetState(FixState::ERROR);
		m_rFeedBackInterface.addLogMessage( Log::LOG_ERROR, "Unknown Error fixing mp3");
	}
}

VbrFixer::ProgressDetails::ProgressDetails( )
	: m_iPercentOfWrite(0)
	, m_iPercentOfProcessing(0)
	, m_eState(FixState::INITIALISING)
{
}

int VbrFixer::ProgressDetails::GetTotalPercent( ) const
{
	return ((m_iPercentOfRead * 7 + m_iPercentOfProcessing * 1 + m_iPercentOfWrite * 2) / 10);
}

bool VbrFixer::ShouldSkipMp3( const Mp3Reader::ConstMp3ObjectList & /*frames*/ ) const
{
	// TODO ensure that if any LOG_ERROR messages were logged then the mp3 will fail
	
	if(m_rFixerSettings.MinimumPercentUnderStood() > m_ProgressDetails.GetPercentUnderstood())
	{
		std::stringstream ss;
		ss << "Only " << m_ProgressDetails.GetPercentUnderstood() << "% understood, minimum allowed is " << m_rFixerSettings.MinimumPercentUnderStood() << "%; skipping";
		m_rFeedBackInterface.addLogMessage( Log::LOG_ERROR, ss.str());
		return true;
	}
	if(m_rFixerSettings.AlwaysSkip())
	{
		m_rFeedBackInterface.addLogMessage( Log::LOG_INFO, "Always Skip is on; skipping file");
		return true;
	}
	if(m_rFixerSettings.skippingNonVbr() && !m_ProgressDetails.IsVbr())
	{
		m_rFeedBackInterface.addLogMessage( Log::LOG_INFO, "Skipping as the file is not vbr");
		return true;
	}
	if(m_ProgressDetails.GetFrameCount() == 0)
	{
		m_rFeedBackInterface.addLogMessage( Log::LOG_ERROR, "The mp3 has no identified frames ; skipping");
		return true;
	}
	return false;
}

void VbrFixer::ProgressDetails::SetState( FixState::State eState )
{
	m_eState = eState;
}

FixState::State VbrFixer::ProgressDetails::GetState( ) const
{
	return m_eState;
}





