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
#include <stdexcept>
#include <string>
#include "ImpExpMacro.h"

namespace DXF {

DXF_API void SetThreadDebugInfo(const char* fileName);
DXF_API std::string GetThreadDebugInfoFileName();
DXF_API void EnableDebugInfo(bool value);

// The following functions/classes are just for internal use, so they are not exported.
void __cdecl PrintDebugInfo(const char* szMsg, ...);
void __cdecl PrintDebugInfo(const char* fileName, int lineNo, const char* funcName, const char* szMsg, ...);
#define PRINT_DEBUG_INFO(msg, ...) \
	PrintDebugInfo(__FILE__, __LINE__, __FUNCTION__, msg, __VA_ARGS__)
#define ASSERT_DEBUG_INFO(boolExp)														\
	if (!(boolExp)) {																	\
		PrintDebugInfo(__FILE__, __LINE__, __FUNCTION__, "Assertion failed:" #boolExp);	\
		throw std::logic_error("Program error!");																	\
	}
#define THROW_INVALID_DXF()													\
	PrintDebugInfo(__FILE__, __LINE__, __FUNCTION__, "Invalid DXF file.");	\
	throw std::runtime_error("Invalid DXF file.")																\


inline int hextoi(const char* src)
{
	return std::stoi(src, nullptr, 16);
}
const char* right(const std::string& src, size_t num);
bool RightCompareNoCase(const std::string& str1, size_t num, const char* str2);
std::string TrimLeftSpace(const std::string& str);
std::string TrimRightSpace(const std::string& str);
std::string TrimSpace(const std::string& str);
typedef const char* cstr;
DXF_API cstr AdvanceUtf8(const char* pStart, const char* pEnd, int bytes);

class CDblPoint
{
public:
	CDblPoint()
	{
		x = 0.0;
		y = 0.0;
	}
	CDblPoint(double X, double Y)
	{
		x = X;
		y = Y;
	}
	CDblPoint(const CDblPoint& Point)
	{
		x = Point.x;
		y = Point.y;
	}
	void SetPoint(double u, double v)
	{
		x = u;
		y = v;
	}
	//绕this点把point点旋转angle弧度到新的位置
	void Rotate(/*in, out*/CDblPoint& point, double angle/*in radians*/) const;

	double x;
	double y;
};

class TraceFunction
{
public:
	TraceFunction(const char* fileName, const char* funcName);
	~TraceFunction();

private:
	const char* m_FileName;
	const char* m_FuncName;
};

#define TRACE_FUNCTION(arguments, ...)										\
PrintDebugInfo(__FILE__, __LINE__, __FUNCTION__, arguments, __VA_ARGS__);	\
TraceFunction func##__LINE__(__FILE__, __FUNCTION__);

}
