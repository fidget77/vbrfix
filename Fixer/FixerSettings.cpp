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

#include "FixerSettings.h"

FixerSettings::FixerSettings( )
	: ReadSettings()
	, m_AlwaysSkip(false)
	, m_MinPercentUnderstood(99)
	, m_bLogDetail(true)
	, m_LameInfoOption(LAME_KEEP)
	, m_bSkipNonVbr(true)
	, m_XingFrameCrcOption(CRC_REMOVE)
	, m_bSkipIfXingTagLooksGood(true)
	, m_bRemoveInconsistentFrames(false)
{
	m_RemoveTypes.insert(Mp3ObjectType::XING_FRAME);
	m_RemoveTypes.insert(Mp3ObjectType::VBRI_FRAME);
}

void FixerSettings::Defaults( )
{
	*this = FixerSettings();
}

bool FixerSettings::RemoveType(Mp3ObjectType eType ) const
{
	return (m_RemoveTypes.find(eType) != m_RemoveTypes.end());
}

void FixerSettings::SetRemoveType( Mp3ObjectType eType, bool bMarkForRemoval )
{
	if(bMarkForRemoval)
	{
		m_RemoveTypes.insert(eType);
	}
	else
	{
		m_RemoveTypes.erase(eType);
	}
}

const Mp3ObjectType::Set & FixerSettings::GetRemovingDataTypes( ) const
{
	return m_RemoveTypes;
}

bool FixerSettings::AlwaysSkip( ) const
{
	return m_AlwaysSkip;
}

void FixerSettings::SetAlwaysSkip( bool bAlwaysSkip )
{
	m_AlwaysSkip = bAlwaysSkip;
}

int FixerSettings::MinimumPercentUnderStood( ) const
{
	return m_MinPercentUnderstood;
}

void FixerSettings::SetMinimumPercentUnderStood( int iMinPercentUnderStood )
{
	m_MinPercentUnderstood = iMinPercentUnderStood;
}
