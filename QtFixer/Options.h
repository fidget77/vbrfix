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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "FixerSettings.h"
#include <QtWidgets>

class QWidget;

class Options : public FixerSettings
{
	public:
		Options();
		virtual ~Options();
		QString getDestionationFile(QWidget* parentDialog, const QString& original) const;

		enum OutputFileMethod {PROMPT, REPLACE_ORIGINAL, MP3_FIXED, DIRECTORY, NO_OUTPUT};

		void Load();
		void Save();

		void setOutputMethod(OutputFileMethod method){outputMethod = method;}
		OutputFileMethod getOutputMethod() const {return outputMethod;}
		QString getOutputDir() const {return outputDir;}
		void setOutputDir(const QString& dir) {outputDir = dir;}
		void setKeepSuffixIfNotSpecified(bool bKeep);
		bool keepSuffixIfNotSpecified() const;

	private:
		OutputFileMethod outputMethod;
		QString outputDir;
		bool m_bKeepSuffixIfNotSpecified;

		void SaveLoad(bool bSave);

};

#endif
