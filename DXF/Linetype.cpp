/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include "stdafx.h"

#include <AtlConv.h>
#include <string>
#include "ifstream.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "linetype.h"
#include "utility.h"

using namespace std;
#pragma warning(disable:4996)

namespace DXF {

LinetypeManager::LinetypeManager() = default;

LinetypeManager::~LinetypeManager() = default;

void LinetypeManager::SetLinFile(const char* pLinFile)
{
	if (!m_LinFile)
		m_LinFile = std::make_unique<DXF::ifstream>();
	if (m_LinFile->is_open())
	{
		m_LinFile->close();
		m_Linetypes.clear();
	}
	m_LinFile->open(pLinFile);
}

bool LinetypeManager::IsValid() const
{
	return m_LinFile && m_LinFile->is_open();
}

Linetype* LinetypeManager::FindLinetype(const char* lineTypeName)
{
	for (auto& pLt : m_Linetypes)
	{
		if (_stricmp(lineTypeName, pLt->line_name.c_str()) == 0)
			return pLt.get();
	}
	if (m_LinFile->is_open())
	{
		auto plt = std::make_unique<Linetype>();
		if (plt->LoadLinetype(*this, lineTypeName))
		{
			auto* ret = plt.get();
			m_Linetypes.push_back(std::move(plt));
			return ret;
		}
	}
	return nullptr;
}

bool Linetype::LoadLinetype(LinetypeManager& manager, const char* lineTypeName)
{
	bool bFound = false;
	ASSERT_DEBUG_INFO(manager.m_LinFile);
	DXF::ifstream& s_LinFile = *manager.m_LinFile;
	ASSERT_DEBUG_INFO(s_LinFile.is_open());
	s_LinFile.reset();
	s_LinFile.setdelimiter(",");
	char buffer[128];
	while (s_LinFile)
	{
		s_LinFile >> buffer;
		if (buffer[0] == ';')
		{
			s_LinFile.ignore(INT_MAX, '\n');
		}
		else if (buffer[0] == '*')
		{
			line_name = &buffer[1];
			if (stricmp(line_name.c_str(), lineTypeName) == 0)
			{
				bFound = true;
			}
			s_LinFile.ignore(INT_MAX, ',');
			s_LinFile.getline(buffer, sizeof(buffer));
			if (bFound)
				line_descrip = buffer;
		}
		else
		{
			if (!bFound)
			{
				s_LinFile.ignore(INT_MAX, '\n');
				continue;
			}

			char onechar;
			s_LinFile >> onechar;
			while (onechar == ',')
			{
				Linetype_Descriptor lts;
				s_LinFile >> buffer;
				if (buffer[0] == '[')
				{
					char anotherchar;
					lts.shapename_text = &buffer[1];
					s_LinFile.skipws();
					s_LinFile >> anotherchar;
					s_LinFile >> buffer;
					s_LinFile.skipws();
					s_LinFile >> onechar;
					lts.shxfilename_style = buffer;
					if (lts.shapename_text.find(".shx") != std::string::npos)
						lts.type = 1;//shape linetype
					else
						lts.type = 2;//text linetype

					do
					{
						unsigned long streampos1 = s_LinFile.tellg();
						s_LinFile.ignore(INT_MAX, ",]");
						unsigned long streampos2 = s_LinFile.tellg();
						s_LinFile.seekg(streampos1);
						s_LinFile.read(buffer, streampos2 - streampos1);
						s_LinFile >> anotherchar;
						std::string transformtype = strtok(buffer, "=");
						transformtype = TrimSpace(transformtype);
						char code = transformtype[0];
						char* pValue = strtok(NULL, "=");
						char* pEnd;
						double value = strtod(pValue, &pEnd);
						switch (code)
						{
						case 's':
						case 'S':
							lts.scale_length = value;
							break;
						case 'a':
						case 'A':
							lts.bRelative = false;
							//break;
						case 'r':
						case 'R':
							{
								char EndChar = *pEnd;
								switch (EndChar)
								{
								case 0:
								case 'd':
								case 'D':
									value *= M_PI / 180.0;
									break;
								case 'r':
								case 'R':
									break;
								case 'g':
								case 'G':
									value = atan(value / 100);
									break;
								default:
									ASSERT_DEBUG_INFO(false);
									break;
								};
								lts.rotate = value;
							}
							break;
						case 'x':
						case 'X':
							lts.xoffset = value;
							break;
						case 'y':
						case 'Y':
							lts.yoffset = value;
							break;
						default:
							ASSERT_DEBUG_INFO(false);
							break;
						};
					} while (anotherchar == ',');
				}
				else
				{
					double temp = strtod(buffer, NULL);
					lts.scale_length = temp;
				}
				descriptors.push_back(lts);
				s_LinFile.skipws();
				s_LinFile >> onechar;
			}
			return true;
		}
	}

	return false;
}

}
