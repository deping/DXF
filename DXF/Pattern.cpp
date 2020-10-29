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

#include "Pattern.h"
#include "ifstream.h"
#include "utility.h"

namespace DXF {

	PatternManager::PatternManager() = default;

	PatternManager::~PatternManager() = default;

	void PatternManager::SetPatFile(const char* pLinFile)
	{
		if (!m_PatFile)
			m_PatFile = std::make_unique<DXF::ifstream>();
		if (m_PatFile->is_open())
		{
			m_PatFile->close();
			m_Patterns.clear();
		}
		m_PatFile->open(pLinFile);
	}

	bool PatternManager::IsValid() const
	{
		return m_PatFile && m_PatFile->is_open();
	}

	Pattern* PatternManager::FindPattern(const char* patName)
	{
		for (auto& pPattern : m_Patterns)
		{
			if (_stricmp(patName, pPattern->name.c_str()) == 0)
				return pPattern.get();
		}
		if (m_PatFile->is_open())
		{
			auto pPat = std::make_unique<Pattern>();
			if (pPat->LoadPattern(*this, patName))
			{
				auto* ret = pPat.get();
				m_Patterns.push_back(std::move(pPat));
				return ret;
			}
		}
		return nullptr;
	}

	bool Pattern::LoadPattern(PatternManager& manager, const char* patName)
	{
		bool bFound = false;
		ASSERT_DEBUG_INFO(manager.m_PatFile);
		DXF::ifstream& s_PatFile = *manager.m_PatFile;
		ASSERT_DEBUG_INFO(s_PatFile.is_open());
		s_PatFile.reset();
		s_PatFile.setdelimiter(",");
		char buffer[512];
		while (s_PatFile)
		{
			s_PatFile >> buffer;
			if (buffer[0] == ';')
			{
				if (bFound)
					return true;
				s_PatFile.ignore(INT_MAX, '\n');
			}
			else if (buffer[0] == '*')
			{
				if (bFound)
					return true;
				name = &buffer[1];
				if (_stricmp(name.c_str(), patName) == 0)
				{
					bFound = true;
				}
				s_PatFile.ignore(INT_MAX, ',');
				s_PatFile.getline(buffer, sizeof(buffer));
				if (bFound)
					description = buffer;
			}
			else
			{
				if (!bFound)
				{
					s_PatFile.ignore(INT_MAX, '\n');
					continue;
				}

				auto dashline = std::make_unique<Pattern_Line>();
				dashline->angle = atof(buffer);
				s_PatFile.ignore(INT_MAX, ',');
				s_PatFile >> dashline->x_origin;
				s_PatFile.ignore(INT_MAX, ',');
				s_PatFile >> dashline->y_origin;
				s_PatFile.ignore(INT_MAX, ',');
				s_PatFile >> dashline->delta_x;
				s_PatFile.ignore(INT_MAX, ',');
				s_PatFile >> dashline->delta_y;

				s_PatFile.getline(buffer, sizeof(buffer));
				//将文件中的pattern数据变成double形式
				char* part = strchr(buffer, ',');
				while (part != NULL)
				{
					++part;
					double dash = strtod(part, NULL);
					dashline->dashes.push_back(dash);
					part = strchr(part, ',');
				}
				dashlines.push_back(std::move(dashline));
			}
		}

		return false;
	}

}
