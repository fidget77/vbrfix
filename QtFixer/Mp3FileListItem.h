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

#ifndef MP3FILELISTITEM_H
#define MP3FILELISTITEM_H

#include <QtGui>
#include "VbrfixMain.h"

class Mp3FileListItem : public QTreeWidgetItem
{
	public:
		Mp3FileListItem(QTreeWidget *parent, const QString &fileName);
		Mp3FileListItem(QTreeWidget *parent, QTreeWidgetItem * after, const QString &fileName);

		~Mp3FileListItem();
		const QString& getFileName() const {return fileName;}
		const FixState::State& getState() const {return m_State;}
		void setStatus(FixState::State newState) {m_State = newState;}
		bool copySucceeded() const {return m_bCopySucceded;}
		void setCopySucceeded(bool value)  { m_bCopySucceded = value;}

		QString GetStatusString() const;

		bool SuitableToStartFix() const;

		void SetError();
		void SetWarning();
	private:
		QString fileName;
		FixState::State m_State;
		bool m_bCopySucceded;
		bool m_bError;
};

#endif

