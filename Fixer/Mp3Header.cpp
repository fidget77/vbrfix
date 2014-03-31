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

#include "Mp3Header.h"
#include <assert.h>

#include "BitReader.h"

// TODO Allowed Modes

namespace
{
	const IndexMask SYNC_MASK(21, 11);
	const IndexMask MPEG_MASK(19, 2);
	const IndexMask LAYER_MASK(17, 2);
	const IndexMask NOT_CRC_PROTECTED_MASK(16);
	const IndexMask BITRATE_MASK(12, 4);
	const IndexMask SAMPFREQ_MASK(10, 2);
	const IndexMask PADDING_MASK(9);
	const IndexMask PRIVATE_MASK(8);
	const IndexMask CHANNEL_MASK(6, 2);
	const IndexMask MODE_EXT_MASK(4, 2);
	const IndexMask MODE_EXT_JOIN_STEREO(5);
	const IndexMask MODE_EXT_MS_STEREO(4);
	const IndexMask COPYRIGHT_MASK(3);
	const IndexMask ORIGINAL_MASK(2);
	const IndexMask EMPHASIS_MASK(0, 2);
}

Mp3Header::Mp3Header(_ul32 header)
	: m_Header(header)
{
}

bool Mp3Header::IsValid( ) const
{
	bool bValid = true;
	bValid &= SYNC_MASK.areAllOn(m_Header);
	bValid &= (GetMpegVersion() != MPEG_VERSION_RESERVED);
	bValid &= (GetLayerVersion() != LAYER_VERSION_RESERVED);
	bValid &= (GetBitRate() != BITRATE_ERRROR);
	bValid &= (GetSampleFrequency() != SAMPLE_FREQ_ERROR);
	bValid &= (GetEmphasis() != EMPHASIS_RESERVED);
	return bValid;
}

Mp3Header::MpegVersion Mp3Header::GetMpegVersion( ) const
{
	const _ul32 iIndex = MPEG_MASK.GetIndex(m_Header);
	static const MpegVersion mpegVer[] = {MPEG_VERSION_2_5, MPEG_VERSION_RESERVED, MPEG_VERSION_2, MPEG_VERSION_1};
	return mpegVer[iIndex];
}

Mp3Header::MpegLayerVersion Mp3Header::GetLayerVersion( ) const
{
	const _ul32 iIndex = LAYER_MASK.GetIndex(m_Header);
	static const MpegLayerVersion layerVer[] = {LAYER_VERSION_RESERVED, LAYER_VERSION_3, LAYER_VERSION_2, LAYER_VERSION_1};
	return layerVer[iIndex];
}

bool Mp3Header::IsProtectedByCrc( ) const
{
	return !NOT_CRC_PROTECTED_MASK.IsOn(m_Header);
}

bool Mp3Header::IsPadded( ) const
{
	return PADDING_MASK.IsOn(m_Header);
}

bool Mp3Header::IsPrivate( ) const
{
	return PRIVATE_MASK.IsOn(m_Header);
}

bool Mp3Header::IsCopyRighted() const
{
	return COPYRIGHT_MASK.IsOn(m_Header);
}

bool Mp3Header::IsOriginal() const
{
	return ORIGINAL_MASK.IsOn(m_Header);
}

int Mp3Header::GetSampleFrequency( ) const
{
	static const int sampFreqs[][4] =
	{
		{44100, 48000, 32000, SAMPLE_FREQ_ERROR}, //MPEG 1
		{22050, 24000, 16000, SAMPLE_FREQ_ERROR}, //MPEG 2
		{11025, 12000,  8000, SAMPLE_FREQ_ERROR}  //MPEG 2.5
	};
	const _ul32 iIndex = SAMPFREQ_MASK.GetIndex(m_Header);
	switch(GetMpegVersion())
	{
		case MPEG_VERSION_1:
			return sampFreqs[0][iIndex];
		case MPEG_VERSION_2:
			return sampFreqs[1][iIndex];
		case MPEG_VERSION_2_5:
			return sampFreqs[2][iIndex];
		default:
			return SAMPLE_FREQ_ERROR;
	}
}

Mp3Header::Emphasis Mp3Header::GetEmphasis( ) const
{
	const _ul32 iIndex = EMPHASIS_MASK.GetIndex(m_Header);
	static Emphasis emps[] = {EMPHASIS_NONE, EMPHASIS_50_15_MS, EMPHASIS_RESERVED, EMPHASIS_CCIT_J_17};
	return emps[iIndex];
}

int Mp3Header::GetKBitRate() const
{
	const _ul32 iIndex = BITRATE_MASK.GetIndex(m_Header);

	static const int mpeg1layer1[]   = {BITRATE_FREE, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, BITRATE_ERRROR};
	static const int mpeg1layer2[]   = {BITRATE_FREE, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, BITRATE_ERRROR};
	static const int mpeg1layer3[]   = {BITRATE_FREE, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, BITRATE_ERRROR};
	static const int mpeg2layer1[]   = {BITRATE_FREE, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, BITRATE_ERRROR};
	static const int mpeg2layer2n3[] = {BITRATE_FREE,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, BITRATE_ERRROR};

	// decide witch table to use
	const int* pBitRateTable = 0;
	switch(GetMpegVersion())
	{
		case MPEG_VERSION_1:
			switch(GetLayerVersion())
			{
				case LAYER_VERSION_1:
					pBitRateTable = reinterpret_cast<const int*>(&mpeg1layer1); break;
				case LAYER_VERSION_2:
					pBitRateTable = reinterpret_cast<const int*>(&mpeg1layer2); break;
				case LAYER_VERSION_3:
					pBitRateTable = reinterpret_cast<const int*>(&mpeg1layer3); break;
				default: break;
			}
			break;
		case MPEG_VERSION_2:
		case MPEG_VERSION_2_5:
			switch(GetLayerVersion())
			{
				case LAYER_VERSION_1:
					pBitRateTable = reinterpret_cast<const int*>(&mpeg2layer1); break;
				case LAYER_VERSION_2:
				case LAYER_VERSION_3:
					pBitRateTable = reinterpret_cast<const int*>(&mpeg2layer2n3); break;
				default: break;
			}
			break;
		default: break;
	}
	if(pBitRateTable)
	{
		return pBitRateTable[iIndex];
	}
	else
	{
		return BITRATE_ERRROR;
	}

}

int Mp3Header::GetSizeCoef1() const
{
	switch(GetLayerVersion())
	{
		case LAYER_VERSION_1: return 12; break;
		case LAYER_VERSION_2: return 144; break;
		case LAYER_VERSION_3:
		{
			switch(GetMpegVersion())
			{
				case MPEG_VERSION_1:
					return 144; break;
				case MPEG_VERSION_2:
				case MPEG_VERSION_2_5:
					return 144 / 2; break;
				case MPEG_VERSION_RESERVED: break;
			}
			break;
		}
		case LAYER_VERSION_RESERVED: break;
	}
	assert(0);
	throw("Error calculating frame size");
	return 1; 
}

int Mp3Header::GetSizeCoef2() const
{
	switch(GetLayerVersion())
	{
		case LAYER_VERSION_1:
			return 4; break;
		case LAYER_VERSION_2:
		case LAYER_VERSION_3:
			return 1; break;
		case LAYER_VERSION_RESERVED: break;
	}
	assert(0);
	throw ("Error Calculating Frame size");
	return 1;
}

int Mp3Header::GetFrameSize( ) const
{
	const int iBitRate = GetBitRate();
	const int iSampFreq = GetSampleFrequency();
	if((iBitRate == BITRATE_ERRROR) || (iSampFreq == SAMPLE_FREQ_ERROR))
	{
		return FRAME_SIZE_ERROR;
	}
	else if(iBitRate == BITRATE_FREE)
	{
		// TODO do something better with free bitrate
		// for now check with IsFreeBitrate() before asking for the frame size
		// and inform the user it is not supported
		assert(0);
		return FRAME_SIZE_ERROR;
	}
	else
	{
		const int coef1 = GetSizeCoef1();
		const int coef2 = GetSizeCoef2();
		const int paddingSize = (IsPadded() ? 1 : 0);
		const int size = (coef1 * iBitRate / iSampFreq + paddingSize) * coef2;
		return size;
	}
}

int Mp3Header::GetBitRate( ) const
{
	const int iKBitRate = GetKBitRate();
	if(iKBitRate != BITRATE_FREE && iKBitRate != BITRATE_ERRROR)
	{
		return iKBitRate * 1000;
	}
	return iKBitRate;
}

Mp3Header::ChannelMode Mp3Header::GetChannelMode( ) const
{
	const _ul32 iIndex = CHANNEL_MASK.GetIndex(m_Header);
	static ChannelMode chans[] = {CHAN_STEREO, CHAN_JOINT_STEREO, CHAN_DUAL_CHANNEL, CHAN_MONO};
	return chans[iIndex];
}

Mp3Header::Mp3Header( const Mp3Header & rOther )
	: m_Header(rOther.GetHeader())
{
	
}

bool Mp3Header::IsFreeBitrate( ) const
{
	if((GetBitRate() == BITRATE_FREE))
	{
		return true;
	}
	return false;
}

bool Mp3Header::IncreaseBitrate( )
{
	int iIndex = BITRATE_MASK.GetIndex(m_Header);
	int iBitRate = GetBitRate();
	if(iBitRate != BITRATE_ERRROR && iBitRate != BITRATE_FREE)
	{
		BITRATE_MASK.SetIndex(m_Header, iIndex + 1);
		iBitRate = GetBitRate();
		if(iBitRate == BITRATE_ERRROR || iBitRate == BITRATE_FREE)
		{
			// set it back
			BITRATE_MASK.SetIndex(m_Header, iIndex);
			return false;
		}
		return true;
	}
	return false;
}

void Mp3Header::RemoveCrcProtection( )
{
	NOT_CRC_PROTECTED_MASK.SetOn(m_Header, true);
}

Mp3Header::ModeExtension Mp3Header::GetModeExtension() const
{
	const _ul32 iIndex = MODE_EXT_MASK.GetIndex(m_Header);
	static ModeExtension chans[] = {BANDS4to31, BANDS8to31, BANDS12to31, BANDS16to3};
	return chans[iIndex];
}

bool Mp3Header::IsModeExtIntensityStereo() const
{
	return MODE_EXT_JOIN_STEREO.IsOn(m_Header);
}

bool Mp3Header::IsModeExtMsStereo() const
{
	return MODE_EXT_MS_STEREO.IsOn(m_Header);
}


