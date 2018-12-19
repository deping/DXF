/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include "utility.h"
#include <stdarg.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <thread>
#include <sstream>
#include "utf8.h"

namespace DXF {

struct ThreadDebugInfo
{
	// ofstream can not be copied.
	std::ofstream m_DebugInfo;
	std::string m_FileName;
	bool m_Enabled;
};

std::map<std::thread::id, std::unique_ptr<ThreadDebugInfo>> g_ThreadDebugInfos;

void SetThreadDebugInfo(const char* fileName)
{
	auto it = g_ThreadDebugInfos.find(std::this_thread::get_id());
	if (it == g_ThreadDebugInfos.end())
	{
		g_ThreadDebugInfos[std::this_thread::get_id()] = std::make_unique<ThreadDebugInfo>();
		it = g_ThreadDebugInfos.find(std::this_thread::get_id());
		it->second->m_Enabled = true;
	}
	if (it != g_ThreadDebugInfos.end())
	{
		if (it->second->m_DebugInfo.is_open())
			it->second->m_DebugInfo.close();
		it->second->m_FileName = fileName;
	}
}

std::string GetThreadDebugInfoFileName()
{
	std::thread::id tid = std::this_thread::get_id();
	auto it = g_ThreadDebugInfos.find(tid);
	if (it != g_ThreadDebugInfos.end())
	{
		return it->second->m_FileName;
	}
	std::stringstream ss;
	ss << "DxfDebugInfo-";
	tid._To_text(ss);
	ss << ".log";
	return ss.str();
}

void EnableDebugInfo(bool value)
{
	std::thread::id tid = std::this_thread::get_id();
	auto it = g_ThreadDebugInfos.find(tid);
	if (it == g_ThreadDebugInfos.end())
	{
		SetThreadDebugInfo(GetThreadDebugInfoFileName().c_str());
		it = g_ThreadDebugInfos.find(tid);
	}
	if (it != g_ThreadDebugInfos.end())
	{
		it->second->m_Enabled = value;
	}
}

std::ofstream* GetThreadDebugInfoStream()
{
	std::thread::id tid = std::this_thread::get_id();
	auto it = g_ThreadDebugInfos.find(tid);
	if (it == g_ThreadDebugInfos.end())
	{
		SetThreadDebugInfo(GetThreadDebugInfoFileName().c_str());
		it = g_ThreadDebugInfos.find(tid);
	}
	if (it != g_ThreadDebugInfos.end())
	{
		if (it->second->m_Enabled)
		{
			if (!it->second->m_DebugInfo.is_open())
				it->second->m_DebugInfo.open(it->second->m_FileName);

			return &it->second->m_DebugInfo;
		}
	}
	return nullptr;
}

void __cdecl PrintDebugInfo(const char* szMsg, ...)
{
	auto debugInfo = GetThreadDebugInfoStream();
	if (!debugInfo || !debugInfo->is_open())
		return;
	char buffer[1024];
	va_list args;
	va_start(args, szMsg);
	int len = vsnprintf(buffer, sizeof(buffer) - 1, szMsg, args);
	va_end(args);
	buffer[sizeof(buffer) - 1] = '\0';
	if (len != -1)
	{
		buffer[len] = '\0';
	}
	*debugInfo << buffer << std::endl;
}

void __cdecl PrintDebugInfo(const char* fileName, int lineNo, const char* funcName, const char* szMsg, ...)
{
	auto debugInfo = GetThreadDebugInfoStream();
	if (!debugInfo || !debugInfo->is_open())
		return;
	char buffer[1024];
	va_list args;
	va_start(args, szMsg);
	int len = vsnprintf(buffer, sizeof(buffer) - 1, szMsg, args);
	va_end(args);
	buffer[sizeof(buffer) - 1] = '\0';
	if (len != -1)
	{
		buffer[len] = '\0';
	}
	const char* sep = strrchr(fileName, '\\');
	if (sep)
		fileName = sep + 1;
	*debugInfo << fileName << ':' << lineNo << '@' << funcName << ' ' << buffer << std::endl;
}

const char* right(const std::string& src, size_t num)
{
	if (src.length() <= num)
	{
		return src.c_str();
	}
	return src.c_str() + src.length() - num;
}

bool RightCompareNoCase(const std::string& str1, size_t num, const char* str2)
{
	return _stricmp(right(str1, num), str2) == 0;
}

std::string TrimLeftSpace(const std::string& str)
{
	size_t pos = str.find_first_not_of(" \t\r\n");
	if (std::string::npos == pos)
	{
		return std::string();
	}
	return str.substr(pos);
}

std::string TrimRightSpace(const std::string& str)
{
	size_t pos = str.find_last_not_of(" \t\r\n");
	return str.substr(0, pos + 1);
}

std::string TrimSpace(const std::string& str)
{
	size_t pos1 = str.find_first_not_of(" \t\r\n");
	if (std::string::npos == pos1)
	{
		return std::string();
	}
	size_t pos2 = str.find_last_not_of(" \t\r\n");
	return str.substr(pos1, pos2 + 1);
}

const char * AdvanceUtf8(const char * pStart, const char* pEnd, int bytes)
{
	//ASSERT_DEBUG_INFO(pStart && pEnd >= pStart && bytes > 0);
	// Quick check and return.
	if (pEnd - pStart <= bytes)
		return pEnd;
	const char* prev = pStart;
	const char* cur;
	while (true)
	{
		try
		{
			cur = prev;
			utf8::next(cur, pEnd);
		}
		catch (utf8::not_enough_room&)
		{
			// go to pEnd;
			if (cur - pStart <= bytes)
			{
				return cur;
			}
		}
		if (cur - pStart > bytes)
		{
			return prev;
		}
		prev = cur;
	}
	return nullptr;
}

void CDblPoint::Rotate(CDblPoint& point, double angle) const
{
	double detX = point.x - x;
	double detY = point.y - y;
	double cosAngle = cos(angle);
	double sinAngle = sin(angle);
	double detX2 = detX*cosAngle - detY*sinAngle;
	double detY2 = detX*sinAngle + detY*cosAngle;
	point.x = x + detX2;
	point.y = y + detY2;
}

TraceFunction::TraceFunction(const char* fileName, const char* funcName)
	: m_FileName(fileName)
	, m_FuncName(funcName)
{
}

TraceFunction::~TraceFunction()
{
	PrintDebugInfo(m_FileName, 0, m_FuncName, "Exit");
}

}
