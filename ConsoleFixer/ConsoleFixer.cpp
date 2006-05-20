/*//////////////////////////////////////////////////////////////////////////////////
// copyright : (C) 2006  by William Pye
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

#include "ConsoleFixer.h"
#include "VbrFixer.h"
#include "FixerSettings.h"
#include <iostream>

ConsoleFixer::~ConsoleFixer()
{
}

ConsoleFixer::ConsoleFixer( std::vector< std::string > args)
	: m_Args(args)
{
}

void ConsoleFixer::addLogMessage( const Log::LogItem sMsg )
{
	std::cout << sMsg.GetText() << std::endl;
}

bool ConsoleFixer::Run( )
{
	std::cout << "Vbrfix Console v0.7" << std::endl;
	FixerSettings settings;
	VbrFixer fixer(*this, settings);
	if(m_Args.size() == 3)
	{
		std::string inFile = m_Args[1];
		std::string outFile = m_Args[2];
		std::cout << "Fixing " << inFile << "->" << outFile << std::endl;
		fixer.Fix( inFile, outFile);
		std::cout << "Finished Fixing" << std::endl;
	}
	else
	{
		std::cout << "usage :" << std::endl;
		std::cout << "./vbrfix in.mp3 out.mp3" << std::endl;
	}
	return true;
}


