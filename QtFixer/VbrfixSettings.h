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


#ifndef VBRFIXSETTINGS_H
#define VBRFIXSETTINGS_H

#include "ui_settings.h"

class Options;
#include <vector>
#include "FixerSettings.h"

class VbrfixSettings: public QDialog, public Ui_Settings
{
	Q_OBJECT

	public:
		VbrfixSettings(QWidget *parent, Options &options);
		virtual ~VbrfixSettings() {}

	private slots:
		void on_okButton_clicked();
		void on_defaultButton_clicked();
		void on_cancelButton_clicked();
		void on_browseOutputDir_clicked();

	private:
		void syncGuiTo();
		void syncFromGui();

		Options& options;

		std::vector<FixerSettings::LameOption> m_LameInfoIndex;

};

#endif
