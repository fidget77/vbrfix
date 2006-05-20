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

#include "VbrfixMain.h"
#include "Mp3FileListItem.h"
#include "VbrfixThread.h"
#include "VbrfixSettings.h"

#include "VbrFixer.h"
#include "FeedBackInterface.h"

VbrfixMain::VbrfixMain(QWidget *parent)
        : QMainWindow(parent)
        , m_pFixThread(NULL)
        , m_pCurrentFixItem(NULL)
{
	setupUi(this);
	QTreeWidgetItem* pHeaderItem = fixList->headerItem();
	pHeaderItem->setText(C_Status, tr("Status"));
	pHeaderItem->setText(C_File, tr("File"));
	pHeaderItem->setText(C_Percent, tr("%"));
	pHeaderItem->setText(C_BitRate, tr("BitRate"));
	pHeaderItem->setText(C_Tags, tr("Tags"));
	pHeaderItem->setText(C_PercentUnderstood, tr("% Understood"));
	pHeaderItem->setText(C_Frames, tr("Frames"));

	m_Options.Load();
	updateButtons();
}

void VbrfixMain::addToFixListMp3s(const QStringList& mp3s)
{
	for(QStringList::const_iterator iter = mp3s.constBegin(); iter != mp3s.constEnd(); ++iter)
	{
		Mp3FileListItem *pAddItem = new Mp3FileListItem(fixList, *iter);
		updateListItem(pAddItem, true);
	}
	fixList->resizeColumnToContents(C_File);
	updateProgress();
	updateButtons();
}

void VbrfixMain::on_actionSettings_triggered()
{
	Options options = m_Options;
	
	VbrfixSettings settingsDlg(this, options);
	if(QDialog::Accepted ==  settingsDlg.exec())
	{
		m_Options = options;
	}
}

void VbrfixMain::on_actionAdd_Mp3_s_triggered()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select one or more Mp3 Files to fix"), QString::null, tr("MPEG Layer 3(*.mp3)"));
	if(!fileNames.empty())
	{
		addToFixListMp3s(fileNames);
	}
}

void VbrfixMain::on_goButton_clicked()
{
	if(m_pFixThread && m_pFixThread->isRunning())
	{
		//already running
		Q_ASSERT(false);
	}
	else
	{
		updateButtons();
		fixAnother();
	}
}

void VbrfixMain::fixAnother()
{
	Mp3FileListItem* fixItem = getNextItemToBeFixed();
	if(fixItem)
	{
		m_pCurrentFixItem = fixItem;
		if(m_pFixThread)
		{
			delete m_pFixThread;
			m_pFixThread = NULL;
		}
		m_pFixThread = new VbrfixThread(m_Options, fixItem->getFileName());
		bool bOK = connect(m_pFixThread, SIGNAL(guiEvent(int)), this, SLOT(threadGuiEvent(int)));
		assert(bOK); // something has gone wring with the gui
		
		fixList->setCurrentItem(fixItem);
		m_pFixThread->start();
		updateProgress();
	}
	updateButtons();
}

void VbrfixMain::updateButtons()
{
	bool fixing = m_pFixThread && m_pFixThread->isRunning();
	bool toFix = (getNextItemToBeFixed() != NULL);
	goButton->setEnabled(toFix && !fixing);
	stopButton->setEnabled(fixing);
	actionClear_Log->setEnabled(!logText->toPlainText().isEmpty());
	actionClear_Fix_List->setEnabled(!fixing && fixList->topLevelItemCount());
}

Mp3FileListItem* VbrfixMain::getNextItemToBeFixed()
{
	// search for next unfixed item in the fix list
	for(int idx = 0; idx < fixList->topLevelItemCount(); ++idx)
	{
		Mp3FileListItem* fixItem = dynamic_cast<Mp3FileListItem*>(fixList->topLevelItem(idx));
		if(fixItem && fixItem->SuitableToStartFix())
		{
			return fixItem;
		}
	}
	return NULL;
}

void VbrfixMain::on_stopButton_clicked()
{
	if(m_pFixThread)
	{
		safeStop();
	}
	else
	{
		Q_ASSERT(false);
	}
}

// stops the fixing thread
void VbrfixMain::safeStop()
{
	if(m_pFixThread)
	{
		m_pFixThread->CancelFix();
		m_pFixThread->wait(5000);
	}
	updateButtons();
}

void VbrfixMain::commonAddDir(bool recursively)
{
	// Ask user for the directory to add
	QString title(tr("Choose a directory of mp3s to add "));
	title += (recursively ? tr("recursively") : tr("non-recursively"));
	QString dir = QFileDialog::getExistingDirectory(this, title, QString::null, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	
	if(dir != QString::null)
	{
		//create a progress dialog to allow the user to cancel / know that we havent hanged.
		QProgressDialog progress(tr("Reading Directory Structure..."), tr("Abort"), 0, 100, this);
	
		// list the mp3s in the directory and add to our fixList
		QStringList mp3s;
		getDirContents(&progress, mp3s, dir, recursively, "*.mp3", true);
		addToFixListMp3s(mp3s);
	}
}


// Recursive function to add all files from a sub dir matching the extension
void VbrfixMain::getDirContents(QProgressDialog *progress, QStringList &outList, const QString &dir, bool recursively, const QString& extension, bool firstRecursion)
{
	// change progress bar information
	if(progress) progress->setLabelText(tr("Reading Directory Structure...") + dir);

	QDir tmpdir(dir, extension);
	tmpdir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::AllDirs);

	QFileInfoList list = tmpdir.entryInfoList();

	for (int i = 0; i < list.size(); ++i)
	{
	 	// update the progress bar
		if(firstRecursion && progress)
		{
		    progress->setValue((100 * i) / list.size());
		}

		// Avoid program hanging and handle cancel being presssed on progress dialog
		QApplication::processEvents();
		if(progress && progress->wasCanceled()) break;

	 	QFileInfo fileInfo = list.at(i);
	 	if(fileInfo.isDir())
	 	{
			if(recursively) getDirContents(progress, outList, fileInfo.absoluteFilePath(), true, extension, false);
		}
		else if (fileInfo.isFile())
		{
		 	outList << fileInfo.absoluteFilePath();
		}
	}

	// if canceled cancel everything
	if(progress && progress->wasCanceled()) outList.clear();
}

void VbrfixMain::on_actionAdd_Dir_triggered()
{
	commonAddDir(true);
}

void VbrfixMain::updateListItem(Mp3FileListItem* fixItem, bool firstTime)
{
	Q_CHECK_PTR(fixItem);
	if(fixItem)
	{
		if(firstTime)
		{
 		 	fixItem->setText(C_File, fixItem->getFileName());
		}
		else
		{
			Q_CHECK_PTR(m_pFixThread);
			if(m_pFixThread)
			{
				fixItem->setStatus(m_pFixThread->GetFixProgressDetails().GetState());
				fixItem->setText(C_Status, fixItem->GetStatusString());

				VbrFixer::ProgressDetails progress = m_pFixThread->GetFixProgressDetails();

				typedef Mp3ObjectType::Set::const_iterator FndObjIter;
				QString tags;
				const Mp3ObjectType::Set& fndObjTypes = progress.GetFoundObjectTypes();
				for(FndObjIter iter = fndObjTypes.begin(); iter != fndObjTypes.end(); ++iter)
				{
					if(iter->IsTypeOfTag())
					{
						if(tags.length() != 0) tags += " ";
						tags +=	GetGuiNameForMp3ObjectType(*iter);
					}
				}
				fixItem->setText(C_Percent, QString("%1%").arg(progress.GetTotalPercent()));
				fixItem->setText(C_BitRate, QString("%1 kbps %2").arg(progress.GetAverageBitrate()).arg(progress.IsVbr() ? "(VBR)" : "(CBR)"));
				fixItem->setText(C_Tags, tags);
				fixItem->setText(C_PercentUnderstood, QString("%1%").arg(progress.GetPercentUnderstood()));
				fixItem->setText(C_Frames, QString("%1").arg(progress.GetFrameCount()));
			}
		}
	}
}

void VbrfixMain::closeEvent( QCloseEvent * ce )
{
	if(m_pFixThread && m_pFixThread->isRunning())
	{
		switch(QMessageBox::information(this, tr("Warning still fixing mp3s"), tr("Are you sure you want to stop and close vbrfix"), QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape))
		{
			case QMessageBox::Yes:
			{
				safeStop();
				ce->accept();
				break;
			}
			case QMessageBox::No:
			{
				ce->ignore();
				break;
			}
		}
	}
	else
	{
		ce->accept();
	}
}

void VbrfixMain::threadGuiEvent(int eEvent)
{
	VbrfixThread::VbrfixEvents et = (VbrfixThread::VbrfixEvents)eEvent;
	if(et == VbrfixThread::UPDATEFASTFIXINFO || et == VbrfixThread::UPDATESLOWFIXINFO)
	{
		if(m_pFixThread && fixList->topLevelItemCount())
			updateListItem(m_pCurrentFixItem, false);
	}
	else if(et == VbrfixThread::UPDATELOGEVENT)
	{
		if(m_pFixThread)
		{
			Log::LogItem logItem;
			while(m_pFixThread->takeLogItem(logItem))
			{
				switch(logItem.GetImportance())
				{
					case Log::LOG_ERROR:
						logText->setTextColor(Qt::red);
						if(m_pCurrentFixItem) m_pCurrentFixItem->SetError();
						break;
					case Log::LOG_WARNING:
						logText->setTextColor(Qt::darkYellow);
						if(m_pCurrentFixItem) m_pCurrentFixItem->SetWarning();
						break;
					default:
						logText->setTextColor(Qt::black);
						break;
				}
				logText->append(QString::fromStdString(logItem.GetText()));
			}
		}
	}
	else if(et == VbrfixThread::FINISHEDFIX)
	{
		finishedFixing();
	}
}


// move temp file depending on output options
void VbrfixMain::finishedFixing()
{
	updateListItem(m_pCurrentFixItem, false);
	Q_CHECK_PTR(m_pFixThread);
	if(m_pFixThread)
	{
		if(m_pFixThread->GetFixProgressDetails().GetState() == FixState::FIXED)
		{
			// Finish up mp3
			QString destination = m_Options.getDestionationFile(this, m_pCurrentFixItem->getFileName());
			if(destination != QString::null)
			{
				QFileInfo fi(destination);
				if(fi.exists() || !fi.isWritable() || fi.isHidden())
				{
					//deal with it. on windows need to delete the file first if exists
					// set deleteTempFile = false if we do
					#warning "TODO readonly files"
					QFile::remove(destination);

				}

				bool result = QFile::copy(m_pFixThread->getTempFileName(), destination);

				m_pCurrentFixItem->setCopySucceeded(result);
				if(!result)
				{
					Q_ASSERT(false);
					m_pFixThread->addLogMessage(Log::LogItem(Log::LOG_ERROR, "Copy Failed"));
				}
			}
			else
			{
				// can happen when someone presses cancel when promped for where to save the fixed version too
				m_pCurrentFixItem->setCopySucceeded(false);
				m_pFixThread->addLogMessage(Log::LogItem(Log::LOG_ERROR, "Couldn't choose destination for file"));
			}
		}
	}
	
	updateListItem(m_pCurrentFixItem, false);
	updateProgress();
	
	if(m_pFixThread && m_pFixThread->wait(3000))
	{
		if(!m_pFixThread->HasUserCancelled())
		{
			fixAnother(); //make sure old thread finished and wait for it for upto 3 secs then start another
		}
	}
}

VbrfixMain::~VbrfixMain()
{
	m_Options.Save();
}

void VbrfixMain::on_actionAbout_triggered()
{
	QMessageBox::about(this, "WAP VBRFIX", QString(tr("visit %1 for more information")).arg("www.willwap.co.uk"));
}

void VbrfixMain::on_actionAbout_QT_triggered()
{
	QMessageBox::aboutQt(this);
}

void VbrfixMain::on_actionLicense_triggered()
{
	QMessageBox::information(this, "WAP VBRFIX", tr("Licenced under the GPL (http://www.gnu.org/copyleft/gpl.html), see the COPYING file that should have come with this."), QMessageBox::Ok);

}

void VbrfixMain::updateProgress( )
{
	int per = 0;
	int items = fixList->topLevelItemCount();
	if(items && m_pCurrentFixItem)
	{
		int index = fixList->indexOfTopLevelItem(m_pCurrentFixItem);
		per = (100 * index) / items;
	}
	
	totalProgress->setValue(per);
}

void VbrfixMain::on_actionClear_Log_triggered( )
{
	logText->clear();
	updateButtons();
}

void VbrfixMain::on_actionClear_Fix_List_triggered( )
{
	fixList->clear();
	updateButtons();
}

void VbrfixMain::dragEnterEvent(QDragEnterEvent *event)
{
	assert(event);
	if(event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void VbrfixMain::dropEvent(QDropEvent *event)
{
	assert(event);
	const QList<QUrl>& dropedUrls = event->mimeData()->urls();

	QStringList mp3s;
	//create a progress dialog to allow the user to cancel / know that we havent hanged.
	QProgressDialog progress(tr("Reading Directory Structure..."), tr("Abort"), 0, 100, this);

	for(QList<QUrl>::const_iterator iter = dropedUrls.begin(); iter != dropedUrls.end(); ++iter)
	{
		QFileInfo fileInfo(iter->toLocalFile());
		if(fileInfo.isDir())
		{
			getDirContents(&progress, mp3s, fileInfo.filePath(), true, "*.mp3", true);
		}
		else if(fileInfo.isFile())
		{
			if(fileInfo.suffix() == "mp3")
			{
				mp3s << fileInfo.filePath();
			}
		}
		if(progress.wasCanceled()) break;
	}
	if(!mp3s.empty() && !progress.wasCanceled())
	{
		addToFixListMp3s(mp3s);
		event->acceptProposedAction();
	}
}

QString VbrfixMain::GetGuiNameForMp3ObjectType(Mp3ObjectType eType)
{
	switch(eType.GetObjectId())
	{
		case Mp3ObjectType::FRAME: return tr("Frame");
		case Mp3ObjectType::ID3V1_TAG: return tr("Id3v1");
		case Mp3ObjectType::ID3V2_TAG: return tr("Id3v2");
		case Mp3ObjectType::UNKNOWN_DATA: return tr("Unknown");
		default : break;
	}
	assert(0);
	return "";
}

