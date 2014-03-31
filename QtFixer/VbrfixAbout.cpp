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

#include "VbrfixAbout.h"
#include <string>
#include <sstream>
#include "VbrFixer.h"

namespace
{
	std::string gGuiVersionString = "-0";
	
	void GetBuildDate(std::string& month, int & day, int & year)
	{
		month = "Jan";
		day = 1;
		year = 2006;
		#ifdef __DATE__
			std::stringstream ss;
			ss << __DATE__;
			ss >> month >> day >> year;
		#endif
	}
}


VbrfixAbout::VbrfixAbout(QWidget *parent)
	: QDialog(parent)
	, Ui_AboutDialog()
{
	setupUi(this);
	std::string month;
	int day = 0, year = 0;
	GetBuildDate(month, day, year);
	BuildDate->setText(QString("%1 %2 %3").arg(day).arg(month.c_str()).arg(year));
	FullVersion->setText(GetFullVersion().c_str());
	FixerVersion->setText(VbrFixer::GetFixerVersion().c_str());
	QFile licence(":/vbrfix/text/copying.text");
	licence.open(QIODevice::ReadOnly);
	licenceBrowser->setText(licence.readAll());
}


VbrfixAbout::~VbrfixAbout()
{
}

std::string VbrfixAbout::GetFullVersion( )
{
	return VbrFixer::GetFixerVersion() + gGuiVersionString;
}


