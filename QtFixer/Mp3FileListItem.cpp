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


#include "Mp3FileListItem.h"

Mp3FileListItem::Mp3FileListItem( QTreeWidget * parent, QTreeWidgetItem * after, const QString & fileName )
	: QTreeWidgetItem(parent, after)
	, fileName(fileName)
	, m_State(FixState::NOTSTARTED)
	, m_bCopySucceded(false)
	, m_bError(false)
{
}

Mp3FileListItem::Mp3FileListItem(QTreeWidget *parent, const QString &fileName)
	: QTreeWidgetItem(parent)
	, fileName(fileName)
	, m_State(FixState::NOTSTARTED)
	, m_bCopySucceded(false)
{
}


Mp3FileListItem::~Mp3FileListItem()
{
}

QString Mp3FileListItem::GetStatusString( ) const
{
	using namespace FixState;
	switch(m_State)
	{
		case NOTSTARTED: return "Unfixed";
		case INITIALISING: return "Initialising";
		case READING: return "Reading";
		case PROCESSING: return "Processing";
		case WRITING: return "Writing";
		case SKIPPED: return "Skipped";
		case CANCELLED: return "Cancelled";
		case ERROR: return "Failed";
		case FIXED:
		{
			return m_bCopySucceded ? "Fixed" : "Copy Failed";
		}
	}
	return "Unknown";
}

bool Mp3FileListItem::SuitableToStartFix( ) const
{
	return m_State == FixState::NOTSTARTED;
}

void Mp3FileListItem::SetError( )
{
	m_bError = true;
	for(int i = 0; i < columnCount(); ++i)
	{
		setTextColor(i, Qt::red);
	}
}

void Mp3FileListItem::SetWarning( )
{
	if(!m_bError)
	{
		for(int i = 0; i < columnCount(); ++i)
		{
			setTextColor(i, Qt::darkYellow);
		}
	}
}


