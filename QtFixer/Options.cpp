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

#include "Options.h"
#include <QtGui>

Options::Options( )
	: FixerSettings()
	, outputMethod(PROMPT)
{
}

Options::~Options()
{
}

QString Options::getDestionationFile(QWidget* parentDialog, const QString & original ) const
{
	QString dest = QString::null;
	switch(outputMethod)
	{
		case PROMPT:
		{
			dest = QFileDialog::getSaveFileName(parentDialog, "Save mp3 file to...", original, "Mpeg Layer 3(*.mp3)");
			break;
		}
		case REPLACE_ORIGINAL:
		{
			dest = original;
			break;
		}
		case MP3_FIXED:
		{
			QFileInfo fi(original);
			dest = fi.absolutePath() + "/" + fi.baseName() + "(vbrfixed)." + fi.completeSuffix();
			break;
		}
		case DIRECTORY:
		{
			QFileInfo fi(original);
			dest = outputDir + "/" + fi.fileName();
			break;
		}
		case NO_OUTPUT:
		{
			dest = QString::null;
			break;
		}
	}
	return dest;
}

namespace
{
	template<class T >
	void SaveLoadHelper(bool bSave, QSettings& settings, T& value, const QString& desc)
	{
		if(bSave)
		{
			settings.setValue("settings/" + desc, value);
		}
		else
		{
			QVariant tmp = settings.value("settings/" + desc, QVariant(value));
			value = tmp.value<T>();
		}
	}

	template<class T >
	void EnumSaveLoadHelper(bool bSave, QSettings& settings, T& value, const QString& desc)
	{
		int tmpValue = static_cast<T >(value);
		SaveLoadHelper<int >(bSave, settings, tmpValue, desc);
		value = static_cast<T >(tmpValue);
	}
}

void Options::Load()
{
	SaveLoad(false);
}

void Options::Save()
{
	SaveLoad(true);
}

void Options::SaveLoad(bool bSave)
{
	std::auto_ptr<QSettings> settings(NULL);
	bool bUseSettingsFileIfFoundInAppDir = false;
	
	#ifdef Q_OS_WIN32
		bUseSettingsFileIfFoundInAppDir = true;
	#endif
	
	if(bUseSettingsFileIfFoundInAppDir)
	{
		QString appDir = QCoreApplication::applicationDirPath();
		QString settingsFile = appDir + "//vbrfix.ini";
		if(QFile(settingsFile).exists())
		{
			settings.reset(new QSettings(settingsFile, QSettings::IniFormat));
		}
	}

	if(settings.get() == NULL)
	{
		settings.reset(new QSettings("Vbrfix", "WAP Vbrfix"));
	}
	
	SaveLoadHelper<QString >(bSave, *settings, outputDir, "Output Directory");
	EnumSaveLoadHelper<OutputFileMethod >(bSave, *settings, outputMethod, "Output Method");
	
	EnumSaveLoadHelper<FixerSettings::LameOption >(bSave, *settings, m_LameInfoOption, "Lame Info Option");
	SaveLoadHelper<bool >(bSave, *settings, m_AlwaysSkip, "Always Skip");
	SaveLoadHelper<bool >(bSave, *settings, m_bSkipNonVbr, "Skip Non VBR");
	SaveLoadHelper<int >(bSave, *settings, m_MinPercentUnderstood, "Minimum Understood Percent");
	
}



