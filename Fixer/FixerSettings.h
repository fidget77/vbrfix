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

#ifndef FIXERSETTINGS_H
#define FIXERSETTINGS_H

#include <string>
#include <set>
#include "Mp3FileObject.h"

// TODO Make it so we have a Setting Class and Settings returns a list of settings
// with descriptions of there ranges so the gui can generate from the options.

class FixerSettings
{
	public:
		FixerSettings();
		
		bool RemoveType(Mp3ObjectType eType) const;
		void SetRemoveType(Mp3ObjectType eType, bool bRemove);
		const Mp3ObjectType::Set & GetRemovingDataTypes() const;

		bool AlwaysSkip() const;
		void SetAlwaysSkip(bool bAlwaysSkip);

		bool KeepLameInfo() const;
		void SetKeepLameInfo(bool bKeepLameInfo);

		int MinimumPercentUnderStood() const;
		void SetMinimumPercentUnderStood(int iMinPercentUnderStood);

		bool loggingDetail() const {return m_bLogDetail;}
		void setLoggingDetail(bool value) {m_bLogDetail = value;}

		void Defaults();
	protected:
		Mp3ObjectType::Set m_RemoveTypes;
		bool m_AlwaysSkip;
		bool m_KeepLameInfo;
		int m_MinPercentUnderstood;
		bool m_bLogDetail;
};

#endif
