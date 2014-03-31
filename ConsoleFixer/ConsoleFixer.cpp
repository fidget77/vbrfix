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
#include "CommandReader.h"

namespace
{
	std::string gConsoleVersionString = "-0";
}

ConsoleFixer::~ConsoleFixer()
{
}

ConsoleFixer::ConsoleFixer(const CommandList& args)
	: m_Args(args)
{
}

void ConsoleFixer::addLogMessage(const Log::LogItem sMsg)
{
	std::cout << sMsg.GetText() << std::endl;
}

bool ConsoleFixer::Run( )
{
	const std::string ConsoleVersion = VbrFixer::GetFixerVersion() + gConsoleVersionString;
	std::cout << "Vbrfix Console version " << ConsoleVersion << std::endl;
	FixerSettings settings;
	VbrFixer fixer(*this, settings);
	CommandReader cmdReader(m_Args);
	if ((cmdReader.GetParameterList().size() == 2) &&
	    GetFixerSettingsFromOptions(settings, cmdReader.GetOptionList()))
	{
		const std::string& inFile = cmdReader.GetParameterList().front();
		const std::string& outFile = cmdReader.GetParameterList().back();

		std::cout << "Fixing " << inFile << "->" << outFile << std::endl;
		fixer.Fix(inFile, outFile);
		std::cout << "Finished Fixing" << std::endl;
	} 
	else 
	{
		std::cout << "Usage :" << std::endl;
		std::cout << "./vbrfix [--option] [--option] in.mp3 out.mp3" << std::endl;
		std::cout <<
			"options (case sensitive):" << std::endl <<
			"--removeId3v1" << std::endl <<
			"--removeId3v2" << std::endl <<
			"--removeUnknown" << std::endl <<
			"--removeLame" << std::endl <<
			"--keepLame" << std::endl <<
			"--keepLameUpdateCrc" << std::endl <<
			"--XingFrameCrcProtectIfCan" << std::endl;
	}
	return true;
}

bool ConsoleFixer::GetFixerSettingsFromOptions(FixerSettings &settings,
					       const CommandReader::OptionList &optionList)
{
	for (CommandReader::OptionList::const_iterator iter = optionList.begin();
	     iter != optionList.end(); ++iter)
	{
		const std::string & option = *iter;
		if (option == "removedId3v1")
		{
			settings.SetRemoveType(Mp3ObjectType::ID3V1_TAG, true);
			std::cout << "Remove Id3v1 on" << std::endl;
		}
		else if (option == "removeId3v2")
		{
			settings.SetRemoveType(Mp3ObjectType::ID3V2_TAG, true);
			std::cout << "Remove Id3v2 on" << std::endl;
		}
		else if (option == "removeUnknown")
		{
			settings.SetRemoveType(Mp3ObjectType::UNKNOWN_DATA, true);
			std::cout << "Remove Unknown on" << std::endl;
		}
		else if (option == "removeLame")
		{
			settings.SetLameInfoOption(FixerSettings::LAME_REMOVE);
			std::cout << "Remove Lame on" << std::endl;
		}
		else if (option == "keepLame")
		{
			settings.SetLameInfoOption(FixerSettings::LAME_KEEP);
			std::cout << "KeepLame on" << std::endl;
		}
		else if (option == "keepLameUpdateCrc")
		{
			settings.SetLameInfoOption(FixerSettings::LAME_KEEP_CALC_TAG_CRC);
			std::cout << "Keep Lame Update Crc on" << std::endl;
		}
		else if (option == "XingFrameCrcProtectIfCan")
		{
			settings.setXingFrameCrcOption(FixerSettings::CRC_KEEP_IF_CAN);
			std::cout << "Xing Frame Crc protection if possible is on" << std::endl;
		}
		else
		{
			std::cout << "Option <" << option << "> not understood" << std::endl;
			return false;
		}
	}
	return true;
}
