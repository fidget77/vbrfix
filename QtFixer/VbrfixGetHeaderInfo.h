/*//////////////////////////////////////////////////////////////////////////////////
// copyright : (C) 2009  by William Pye
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

#ifndef VBRFIXGETHEADERINFO_H
#define VBRFIXGETHEADERINFO_H

#include <QtWidgets>
#include "ui_mp3headerInfo.h"

class VbrfixGetHeaderInfo: public QDialog, public Ui_GetMp3HeaderInfo
{
	Q_OBJECT

	public:
		VbrfixGetHeaderInfo(QWidget *parent);
	private slots:
		void on_getHeaderInfoButton_clicked();
};

#endif
