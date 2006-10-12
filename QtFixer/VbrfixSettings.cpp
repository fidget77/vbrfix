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
#include "ComboManager.h"
#include "Options.h"
#include <QtGui>

VbrfixSettings::VbrfixSettings(QWidget *parent, Options &options)
	: QDialog(parent)
	, Ui_Settings()
	, options(options)
	, m_LameComboManager( new LameComboManager() )
	, m_XingCrcComboManager( new XingCrcComboManager() )
{
	setupUi(this);
	m_LameComboManager->SetComboBox(lameOption);
	m_XingCrcComboManager->SetComboBox(xingCrcFrameOption);
	
	m_LameComboManager->AddItem(FixerSettings::LAME_REMOVE, "Remove");
	m_LameComboManager->AddItem(FixerSettings::LAME_KEEP, "Keep VBR Tag Lame Info");
	m_LameComboManager->AddItem(FixerSettings::LAME_KEEP_CALC_TAG_CRC, "Keep VBR Tag Lame Info(recalculate the CRC) - experimental");
	
	m_XingCrcComboManager->AddItem(FixerSettings::CRC_REMOVE, "No CRC For Xing Frame");
	m_XingCrcComboManager->AddItem(FixerSettings::CRC_KEEP_IF_CAN, "CRC For Xing Frame if Supported and MP3 has CRC");
	m_XingCrcComboManager->AddItem(FixerSettings::CRC_KEEP, "CRC For Xing Frame if MP3 has CRC");
	
	syncGuiTo();
}

void VbrfixSettings::syncGuiTo()
{
	m_LameComboManager->SetSelection(options.LameInfoOption());
	m_XingCrcComboManager->SetSelection(options.GetXingFrameCrcOption());
	
	alwaysSkip->setChecked(options.AlwaysSkip());
	minPercentUnderstood->setValue(options.MinimumPercentUnderStood());
	
	treatFreeFormatAsUnkown->setChecked(options.getTreatFreeFormatFramesAsUnknownData());

	removeXingTags->setChecked(options.RemoveType(Mp3ObjectType::XING_FRAME));
	removeVbriTags->setChecked(options.RemoveType(Mp3ObjectType::VBRI_FRAME));
	removeId3v1Tags->setChecked(options.RemoveType(Mp3ObjectType::ID3V1_TAG));
	removeId3v2Tags->setChecked(options.RemoveType(Mp3ObjectType::ID3V2_TAG));
	removeApeTags->setChecked(options.RemoveType(Mp3ObjectType::APE_TAG));
	removeUnknownData->setChecked(options.RemoveType(Mp3ObjectType::UNKNOWN_DATA));
	skipNonVbr->setChecked(options.skippingNonVbr());

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
	
	allowEmptyExtensions->setChecked(!options.keepSuffixIfNotSpecified());
}

void VbrfixSettings::syncFromGui()
{
	const FixerSettings defaults;
	options.SetLameInfoOption(m_LameComboManager->GetSelection(defaults.LameInfoOption()));
	options.setXingFrameCrcOption(m_XingCrcComboManager->GetSelection(defaults.GetXingFrameCrcOption()));

	options.SetAlwaysSkip(alwaysSkip->isChecked());
	options.SetMinimumPercentUnderStood(minPercentUnderstood->value());
	
	options.setTreatFreeFormatFramesAsUnknownData(treatFreeFormatAsUnkown->isChecked());

	options.SetRemoveType(Mp3ObjectType::XING_FRAME, removeXingTags->isChecked());
	options.SetRemoveType(Mp3ObjectType::VBRI_FRAME, removeVbriTags->isChecked());
	options.SetRemoveType(Mp3ObjectType::ID3V1_TAG, removeId3v1Tags->isChecked());
	options.SetRemoveType(Mp3ObjectType::ID3V2_TAG, removeId3v2Tags->isChecked());
	options.SetRemoveType(Mp3ObjectType::APE_TAG, removeApeTags->isChecked());
	options.SetRemoveType(Mp3ObjectType::UNKNOWN_DATA, removeUnknownData->isChecked());

	options.setSkippingNonVbr(skipNonVbr->isChecked());

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
	
	options.setKeepSuffixIfNotSpecified(!allowEmptyExtensions->isChecked());
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

VbrfixSettings::~ VbrfixSettings( )
{
	
}
