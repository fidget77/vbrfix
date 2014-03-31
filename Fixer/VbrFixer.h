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

#ifndef VBRFIXER_H
#define VBRFIXER_H

#include <string>
#include "Mp3Reader.h"

class FeedBackInterface; class FixerSettings;

namespace FixState
{
	enum State { NOTSTARTED, INITIALISING, READING, PROCESSING, WRITING, FIXED, SKIPPED, CANCELLED, ERROR};
};

class VbrFixer
{
	public:
		class ProgressDetails : public Mp3Reader::ReadProgressDetails
		{
			public:
				ProgressDetails();
				int GetTotalPercent() const;
				FixState::State GetState() const;
				void SetState(FixState::State eState);

				void setPercentOfProcessing(int iPer){m_iPercentOfProcessing = iPer;}
				void setPercentOfWriting(int iPer){m_iPercentOfWrite = iPer;}
			private:
				int m_iPercentOfWrite;
				int m_iPercentOfProcessing;
				FixState::State m_eState;
		};
		
		VbrFixer(FeedBackInterface & rFeedBackInterface, const FixerSettings & fixerSettings);
		virtual ~VbrFixer();

		void Fix(const std::string &sInFileName, const std::string &sOutFileName);

		const ProgressDetails & GetProgressDetails() const {return m_ProgressDetails;}
		
		static std::string GetFixerVersion();

	private:
		FeedBackInterface & m_rFeedBackInterface;
		const FixerSettings &m_rFixerSettings;

		ProgressDetails m_ProgressDetails;

		bool ShouldSkipMp3(const Mp3Reader::ConstMp3ObjectList &frames) const;
};

#endif
