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

#ifndef MP3FILEOBJECT_H
#define MP3FILEOBJECT_H

#include <iosfwd>
#include <set>

class FileBuffer;

class Mp3ObjectType
{
	public:
		enum ObjectId
		{
			UNKNOWN_DATA,
			FRAME,
			XING_FRAME,
			ID3V1_TAG,
			ID3V2_TAG,
			LYRICS_TAG,
			APE_TAG
		};
		
		typedef std::set<ObjectId> ObjectIdSet;
		typedef std::set<Mp3ObjectType> Set;

		Mp3ObjectType(ObjectId type);
		//Mp3ObjectType(const Mp3ObjectType &rOther);
	
		bool IsTypeOfFrame() const;
		bool IsTypeOfTag() const;
		bool IsUnknown() const;

		ObjectId GetObjectId() const {return m_Type;}
	
		static const Set& GetTypes();
		static const Set& GetFrameTypes();
		static const Set& GetTagTypes();

		bool operator < (const Mp3ObjectType& rOther) const;
		Mp3ObjectType& operator = (const Mp3ObjectType& rOther);
		
	private:
		ObjectId m_Type;
};

class Mp3Object
{
	public:
		typedef Mp3ObjectType::Set Mp3ObjectTypeSet;

		Mp3Object(); // not from a file
		Mp3Object(unsigned long iFromFilePosition);

		virtual ~Mp3Object();
		
		virtual Mp3ObjectType GetObjectType() const = 0;
		virtual unsigned long size() const = 0;

		// original file position (if from original file)
		virtual bool IsFromFile() const;
		virtual unsigned long getOldFilePosition() const;
		virtual unsigned long getOldEndOfObjectFilePosition() const;

		virtual void writeToFile(FileBuffer & originalFile, std::ofstream & rOutFile) const;
	private:
		bool m_IsFromFile;
		unsigned long m_OldFilePosition; // if from file
};

#endif
