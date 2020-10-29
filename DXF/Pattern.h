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
#include <string>
#include <vector>
#include <memory>

namespace DXF
{
	class ifstream;

	struct Pattern_Line
	{
		double angle; // in degrees
		double x_origin;
		double y_origin;
		double delta_x;
		double delta_y;
		std::vector<double> dashes;
	};

	class PatternManager;
	class Pattern
	{
	public:
		//返回true表示在图案文件patFile中找到patName图案，否则没找到。
		bool LoadPattern(PatternManager &manager, const char *patName);

	public:
		std::string name;
		std::string description;
		std::vector<std::unique_ptr<Pattern_Line>> dashlines;
	};

	class PatternManager
	{
	public:
		PatternManager();
		~PatternManager();
		PatternManager(const PatternManager &) = delete;
		void SetPatFile(const char *pPatFile);
		bool IsValid() const;
		Pattern *FindPattern(const char *ptname);

	private:
		friend class Pattern;
		std::unique_ptr<DXF::ifstream> m_PatFile;
		std::vector<std::unique_ptr<Pattern>> m_Patterns;
	};

} // namespace DXF
