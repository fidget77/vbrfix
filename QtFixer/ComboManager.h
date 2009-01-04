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

#ifndef COMBOMANAGER_H
#define COMBOMANAGER_H

#include <string>
#include <vector>
#include <algorithm>

template < typename _Data >
class ComboManager
{
	typedef std::vector < _Data > ListType;
	public:
		ComboManager(QComboBox* comboBox = NULL) : m_ComboBox(comboBox) {}
		virtual ~ComboManager() {}
		
		void SetComboBox(QComboBox* comboBox) {m_ComboBox = comboBox;}
		
		void AddItem(const _Data & d, const std::string & text)
		{
			m_ComboBox->addItem(text.c_str());
			m_DataList.push_back(d);
		}
		
		_Data GetSelection(_Data defaultValue) const
		{
			int iIndex = m_ComboBox->currentIndex();
			if(iIndex >= 0 && iIndex < static_cast<int>(m_DataList.size()))
			{
				return m_DataList[iIndex];
			}
			return defaultValue;
		}
		
		bool SetSelection(const _Data & d)
		{
			typename ListType::iterator index;
			index = std::find(m_DataList.begin(), m_DataList.end(), d);
			if(index != m_DataList.end())
			{
				m_ComboBox->setCurrentIndex(index - m_DataList.begin());
				return true;
			}
			return false;
		}
	private:
		ListType m_DataList;
		QComboBox * m_ComboBox;
};

#endif
