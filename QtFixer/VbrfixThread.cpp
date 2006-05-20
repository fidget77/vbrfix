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


#include "VbrfixThread.h"
#include "VbrFixer.h"
#include <QtCore>

VbrfixThread::VbrfixThread(const FixerSettings & fixSettings, const QString &inFile)
	: QThread()
	, m_FixerSettings(fixSettings)
	, m_inFile(inFile)
	, m_outFile(QDir::tempPath() +  QDir::separator() + "vbrfix.tmp")
	, m_UserHasCancelled(false)
	, m_Fixer(new VbrFixer(*this, m_FixerSettings))
{
}


VbrfixThread::~VbrfixThread()
{
	// remove the temp file
	QFile::remove(m_outFile);
}

void VbrfixThread::postEventToGui(VbrfixEvents eEvent)
{
	emit guiEvent(eEvent);
}

void VbrfixThread::run()
{
	fix();
}


// main fix funciton
void VbrfixThread::fix()
{
	update();
	m_Fixer->Fix(m_inFile.toStdString(), m_outFile.toStdString());
	SyncProgresDetails();
	
	postEventToGui(FINISHEDFIX);
}

QString VbrfixThread::getTempFileName( ) const
{
	return m_outFile;
}

bool VbrfixThread::HasUserCancelled( ) const
{
	QReadLocker lock(&m_ThreadLock);
	return m_UserHasCancelled;
}

void VbrfixThread::CancelFix( )
{
	QWriteLocker lock(&m_ThreadLock);
	m_UserHasCancelled = true;
}

void VbrfixThread::addLogMessage( const Log::LogItem log )
{
	{
		QWriteLocker lock(&m_ThreadLock);
		m_LogItems.push_back(log);
	}
	postEventToGui(UPDATELOGEVENT);
}

bool VbrfixThread::takeLogItem( Log::LogItem & rLog )
{
	QWriteLocker lock(&m_ThreadLock);
	if(!m_LogItems.empty())
	{
		rLog = m_LogItems.front();
		m_LogItems.pop_front();
		return true;
	}
	else return false;
}

void VbrfixThread::update( )
{
	QTime t = QTime::currentTime();
	if(t > m_LastGuiUpdate.addMSecs(50))
	{
		SyncProgresDetails();
		postEventToGui(UPDATESLOWFIXINFO);
		msleep(0);
		m_LastGuiUpdate = t;
	}
}

VbrFixer::ProgressDetails VbrfixThread::GetFixProgressDetails( ) const
{
	QWriteLocker lock(&m_ThreadLock);
	return m_FixProgressDetails;
}

void VbrfixThread::SyncProgresDetails( )
{
	const VbrFixer::ProgressDetails & prog = m_Fixer->GetProgressDetails();
	{
		QWriteLocker lock(&m_ThreadLock);
		m_FixProgressDetails = prog;
	}
}
