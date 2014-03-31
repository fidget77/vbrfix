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

#include "VbrfixGetHeaderInfo.h"
#include <string>
#include <sstream>

#include "Mp3Header.h"

namespace
{
	QString fromBool(bool b)
	{
		return b ? "True" : "False";
	}
}

VbrfixGetHeaderInfo::VbrfixGetHeaderInfo(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
}

void VbrfixGetHeaderInfo::on_getHeaderInfoButton_clicked()
{
	Mp3Header header(inputMp3Header->text().toULong(0, radioHex->isChecked() ? 16 : 2));
	results->clear();
	results->append("Hex : 0x" + QString::number(header.GetHeader(), 16));
	results->append("Dec : " + QString::number(header.GetHeader()));
	results->append("Bin : " + QString::number(header.GetHeader(), 2));
	results->append("Valid : " + fromBool(header.IsValid()));
	results->append("Bitrate : " + QString::number(header.GetBitRate()));
	results->append("Sample Frequency : " + QString::number(header.GetSampleFrequency()));
	switch(header.GetMpegVersion())
	{
		case Mp3Header::MPEG_VERSION_1: results->append("MPEG Version : 1"); break;
		case Mp3Header::MPEG_VERSION_2: results->append("MPEG Version : 2"); break;
		case Mp3Header::MPEG_VERSION_2_5: results->append("MPEG Version : 2.5"); break;
	}

	switch(header.GetLayerVersion())
	{
		case Mp3Header::LAYER_VERSION_1: results->append("Layer : 1"); break;
		case Mp3Header::LAYER_VERSION_2: results->append("Layer : 2"); break;
		case Mp3Header::LAYER_VERSION_3: results->append("Layer : 3"); break;
	}

	switch(header.GetChannelMode())
	{
		case Mp3Header::CHAN_STEREO: results->append("ChannelMode : Stereo"); break;
		case Mp3Header::CHAN_JOINT_STEREO: results->append("ChannelMode : Joint Stereo"); break;
		case Mp3Header::CHAN_DUAL_CHANNEL: results->append("ChannelMode : Dual Channel"); break;
		case Mp3Header::CHAN_MONO: results->append("ChannelMode : Mono"); break;
	}

	switch(header.GetModeExtension())
	{
		case Mp3Header::BANDS4to31: results->append("ModeExtension : Bands 4 to 31"); break;
		case Mp3Header::BANDS8to31: results->append("ModeExtension : Bands 8 to 31"); break;
		case Mp3Header::BANDS12to31: results->append("ModeExtension : Bands 12 to 31"); break;
		case Mp3Header::BANDS16to3: results->append("ModeExtension : Bands 16 to 3"); break;
	}

	results->append("ModeExtIntensityStereo : " + fromBool(header.IsModeExtIntensityStereo()));
	results->append("ModeExtMsStereo : " + fromBool(header.IsModeExtMsStereo()));

	switch(header.GetEmphasis())
	{
		case Mp3Header::EMPHASIS_NONE: results->append("Emphasis : None"); break;
		case Mp3Header::EMPHASIS_50_15_MS: results->append("Emphasis : 50/15 MS"); break;
		case Mp3Header::EMPHASIS_RESERVED: results->append("Emphasis : Reserved"); break;
		case Mp3Header::EMPHASIS_CCIT_J_17: results->append("Emphasis : CCIT J.17"); break;
	}
	
	results->append("ProtectedByCrc : " + fromBool(header.IsProtectedByCrc()));
	results->append("Padded : " + fromBool(header.IsPadded()));
	results->append("Private : " + fromBool(header.IsPrivate()));
	results->append("CopyRighted : " + fromBool(header.IsCopyRighted()));
	results->append("Original : " + fromBool(header.IsOriginal()));
	results->append("FreeBitrate : " + fromBool(header.IsFreeBitrate()));
	results->append("Frame Size : " + QString::number(header.GetFrameSize()));

}
