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

#include <QtWidgets>
#include "VbrfixMain.h"

class Mp3FileListItem : public QTreeWidgetItem
{
	public:
		enum CopyState {NOT_COPIED, COPY_OK, COPY_FAIL};
		Mp3FileListItem(QTreeWidget *parent, const QString &fileName);
		Mp3FileListItem(QTreeWidget *parent, QTreeWidgetItem * after, const QString &fileName);

		virtual ~Mp3FileListItem();
		const QString& getFileName() const {return fileName;}
		const FixState::State& getState() const {return m_State;}
		void setStatus(FixState::State newState) {m_State = newState;}
		CopyState copyState() const {return m_CopyState;}
		void setCopyState(CopyState value)  { m_CopyState = value;}
		bool hasBeenProcessed() const {return m_bProcessed;}
		void setHasBeenProcessed(bool value) { m_bProcessed = value;}

		QString GetStatusString() const;

		bool SuitableToStartFix() const;

		void SetError();
		void SetWarning();
	private:
		QString fileName;
		FixState::State m_State;
		CopyState m_CopyState;
		bool m_bError;
		bool m_bProcessed;
};

#endif

