/*//////////////////////////////////////////////////////////////////////////////////
// copyright : (C) 2005  by William Pye
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

#ifndef VBRFIXTHREAD_H
#define VBRFIXTHREAD_H

#include <QtWidgets>
#include "FeedBackInterface.h"
#include "FixerSettings.h"
#include "VbrFixer.h"
#include <memory>

class VbrfixThread : public QThread , public FeedBackInterface
{
	Q_OBJECT

	public:
		
		VbrfixThread(const FixerSettings & fixOptions, const QString &inFile);

		virtual ~VbrfixThread();

		void run();

		enum VbrfixEvents{UPDATEFASTFIXINFO = QEvent::User + 1, FINISHEDFIX, UPDATELOGEVENT, UPDATESLOWFIXINFO};

		QString getTempFileName() const;

		void update();
		bool HasUserCancelled() const;
		void addLogMessage(const Log::LogItem log);
		bool takeLogItem(Log::LogItem& rLog);
		void CancelFix();

		VbrFixer::ProgressDetails GetFixProgressDetails() const;

	signals:
        	void guiEvent(int);

	private:
		void fix();
		void postEventToGui(VbrfixEvents eEvent);

		const FixerSettings m_FixerSettings;

		QString m_inFile;
		QString m_outFile;

		bool m_UserHasCancelled;
		mutable QReadWriteLock m_ThreadLock;
		std::list<Log::LogItem> m_LogItems;
		QTime m_LastGuiUpdate;
		std::auto_ptr<VbrFixer> m_Fixer;
		VbrFixer::ProgressDetails m_FixProgressDetails;
		void SyncProgresDetails();
};

#endif
