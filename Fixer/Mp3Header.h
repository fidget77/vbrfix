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

#ifndef MP3HEADER_H
#define MP3HEADER_H

class Mp3Header
{
	typedef unsigned long _ul32;
	
	public:
		enum MpegVersion { MPEG_VERSION_RESERVED, MPEG_VERSION_1, MPEG_VERSION_2, MPEG_VERSION_2_5};
		enum MpegLayerVersion { LAYER_VERSION_RESERVED, LAYER_VERSION_1, LAYER_VERSION_2, LAYER_VERSION_3};
		enum Emphasis {EMPHASIS_NONE, EMPHASIS_50_15_MS, EMPHASIS_RESERVED, EMPHASIS_CCIT_J_17};
		enum ChannelMode {CHAN_STEREO, CHAN_JOINT_STEREO, CHAN_DUAL_CHANNEL, CHAN_MONO};
		enum ModeExtension { BANDS4to31, BANDS8to31, BANDS12to31, BANDS16to3 };
		
		enum {
			BITRATE_FREE = 0,
			BITRATE_ERRROR = -1,
			SAMPLE_FREQ_ERROR = -2,
			FRAME_SIZE_ERROR = -3,
			HEADER_SIZE_IN_BYTES = 4
		};
		
		Mp3Header(_ul32 header);
		Mp3Header(const Mp3Header& rOther);
		bool IsValid() const;
		MpegVersion GetMpegVersion() const;
		MpegLayerVersion GetLayerVersion() const;
		int GetBitRate() const;
		int GetKBitRate() const;
		int GetSampleFrequency() const;
		int GetFrameSize() const;

		bool IsProtectedByCrc() const;
		bool IsPadded() const;
		bool IsPrivate() const;
		bool IsCopyRighted() const;
		bool IsOriginal() const;
		bool IsFreeBitrate() const;

		ChannelMode GetChannelMode() const;
		ModeExtension GetModeExtension() const; // layer I & II only
		bool IsModeExtIntensityStereo() const; // layer III only
		bool IsModeExtMsStereo() const; // layer III only

		Emphasis GetEmphasis() const;

		_ul32 GetHeader() const {return m_Header;}

		bool IncreaseBitrate();
		
		void RemoveCrcProtection();

		bool operator < (const Mp3Header& rOther) const
		{
			return m_Header < rOther.m_Header;
		}

	private:
		enum {HEADER_BITS = 32};
		_ul32 m_Header;

		int GetSizeCoef2() const;
		int GetSizeCoef1() const;
};

#endif
