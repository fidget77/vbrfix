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

#ifndef __VBRFIXMAIN_H
#define __VBRFIXMAIN_H

#include "ui_vbrfixui.h"
#include <QtGui>

#include "VbrfixThread.h"
#include "Options.h"
#include <string>

class VbrfixThread; class Mp3FileListItem; class QDir; class QStringList; class QProgressDialog;

class VbrfixMain : public QMainWindow, private Ui_VbrfixMain
{
	Q_OBJECT

	public:
		VbrfixMain(QWidget *parent = 0);
		virtual ~VbrfixMain();

	private:

		enum Columns{C_Status = 0, C_File, C_Percent, C_BitRate, C_Tags, C_PercentUnderstood, C_Frames};

		void addToFixListMp3s(const QStringList& mp3s);
		void updateListItem(Mp3FileListItem* fixItem, bool firstTime);
		void getDirContents(QProgressDialog *progress, QStringList &outList, const QString &dir, bool recursively, const QString& extension, bool firstRecursion);
		void commonAddDir(bool recursively);
		void fixAnother();
		void safeStop();
		VbrfixThread *m_pFixThread;

		Mp3FileListItem* getNextItemToBeFixed();

		Mp3FileListItem* m_pCurrentFixItem;
		Options m_Options;
		void closeEvent( QCloseEvent * ce );

		void finishedFixing();
		
		void updateProgress();

		void dragEnterEvent(QDragEnterEvent *event);

		void dropEvent(QDropEvent *event);

		static std::string GetGuiNameForMp3ObjectType(Mp3ObjectType eType);

	private slots:

		void on_actionAdd_Mp3_s_triggered();
		void on_actionAdd_Dir_triggered();
		void on_actionSettings_triggered();
		void on_actionClear_Log_triggered();
		void on_actionClear_Fix_List_triggered();
		void on_actionAbout_triggered();
		void on_actionAbout_QT_triggered();

		void on_goButton_clicked();
		void on_stopButton_clicked();

		void threadGuiEvent(int);
		void updateButtons();

};

#endif
