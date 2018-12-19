/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace DXF
{

class ifstream;

struct Linetype_Descriptor
{
	Linetype_Descriptor()
	{
		type = 0;
		scale_length = 1.0;
		bRelative = true;
		rotate = 0;
		xoffset = 0;
		yoffset = 0;

	}
	int type;//0 -- simple, 1--shape, 2--text
	struct
	{
		std::string shapename_text;
		std::string shxfilename_style;
		double scale_length;
		bool bRelative;
		double rotate;//in radians
		double xoffset;
		double yoffset;
	};
};

class LinetypeManager;
class Linetype
{
public:
	//返回true表示在线型文件linFileName中找到lineTypeName线型，否则没找到。
	bool LoadLinetype(LinetypeManager& manager, const char* lineTypeName);

public:
	std::string line_name;
	std::string line_descrip;
	std::vector<Linetype_Descriptor> descriptors;
};

class LinetypeManager
{
public:
	LinetypeManager();
	~LinetypeManager();
	LinetypeManager(const LinetypeManager&) = delete;
	void SetLinFile(const char* pLinFile);
	bool IsValid() const;
	Linetype* FindLinetype(const char* lineTypeName);

private:
	friend class Linetype;
	std::unique_ptr<DXF::ifstream> m_LinFile;
	std::vector<std::unique_ptr<Linetype>> m_Linetypes;
};

}
