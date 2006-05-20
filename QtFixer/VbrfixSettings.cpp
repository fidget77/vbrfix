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


#include "VbrfixSettings.h"
#include "Options.h"
#include <QtGui>

VbrfixSettings::VbrfixSettings(QWidget *parent, Options &options)
    : QDialog(parent)
    , Ui_Settings()
    , options(options)
{
	setupUi(this);
	syncGuiTo();
}

void VbrfixSettings::syncGuiTo()
{
	keepLameInfo->setChecked(options.KeepLameInfo());
	alwaysSkip->setChecked(options.AlwaysSkip());
	minPercentUnderstood->setValue(options.MinimumPercentUnderStood());

	// output method
	QRadioButton* selected = NULL;
	Options::OutputFileMethod outMethod = options.getOutputMethod();
	if(outMethod == Options::PROMPT) selected = promptForEach;
	else if(outMethod == Options::REPLACE_ORIGINAL) selected = replaceOriginals;
	else if(outMethod == Options::MP3_FIXED) selected = appendVbrfixed;
	else if(outMethod == Options::DIRECTORY) selected = outputToDir;
	selected->setChecked(true);

	//output dir
	outputDir->setText(options.getOutputDir());
}

void VbrfixSettings::syncFromGui()
{
	options.SetKeepLameInfo(keepLameInfo->isChecked());
	options.SetAlwaysSkip(alwaysSkip->isChecked());
	options.SetMinimumPercentUnderStood(minPercentUnderstood->value());

	// output method
	Options::OutputFileMethod outMethod = Options::PROMPT;

	if(promptForEach->isChecked()) outMethod = Options::PROMPT;
	else if(replaceOriginals->isChecked()) outMethod = Options::REPLACE_ORIGINAL;
	else if(appendVbrfixed->isChecked()) outMethod = Options::MP3_FIXED;
	else if(outputToDir->isChecked()) outMethod = Options::DIRECTORY;

	options.setOutputMethod(outMethod);

	//output dir
	if(outMethod == Options::DIRECTORY)
	{
		options.setOutputDir(outputDir->text());
	}
}

void VbrfixSettings::on_okButton_clicked()
{
	syncFromGui();
	done(QDialog::Accepted);
}

void VbrfixSettings::on_defaultButton_clicked()
{
	options = Options();
	syncGuiTo();
}

void VbrfixSettings::on_cancelButton_clicked()
{
	done(QDialog::Rejected);
}
void VbrfixSettings::on_browseOutputDir_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select an Ouput Directory"), outputDir->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if(dir != QString::null)
	{
		outputDir->setText(dir);
	}
}
