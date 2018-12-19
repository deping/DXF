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
#include "FileMapping.h"
#include "utility.h"

namespace DXF
{
	CFileMapping::CFileMapping()
	{
		m_hFile = INVALID_HANDLE_VALUE;
		m_hMapFile = INVALID_HANDLE_VALUE;
		m_pStart = nullptr;
	}

	CFileMapping::CFileMapping(const char* FileName)
	{
		m_hFile = INVALID_HANDLE_VALUE;
		m_hMapFile = INVALID_HANDLE_VALUE;
		m_pStart = nullptr;
		Open(FileName);
	}

	void CFileMapping::Open(const char* FileName)
	{
		ASSERT_DEBUG_INFO(m_hFile == INVALID_HANDLE_VALUE);
		m_hFile = CreateFileA(FileName, // file to open
			GENERIC_READ,          // open for reading
			FILE_SHARE_READ,       // share for reading
			nullptr,                  // default security
			OPEN_EXISTING,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			nullptr);                 // no attr. template
		if (m_hFile == INVALID_HANDLE_VALUE) 
		{
			// Get file name without path
			const char* pureFileName = strrchr(FileName, '\\');
			if (pureFileName == NULL)
				pureFileName = strrchr(FileName, '/');
			if (pureFileName == NULL)
				pureFileName = FileName;
			else
				pureFileName += 1;

			PRINT_DEBUG_INFO("Could not open %s(LastError=%d).\n", pureFileName, GetLastError());
			return;
		}

		m_hMapFile = CreateFileMapping(
			m_hFile,				// use paging file
			nullptr,                   // default security 
			PAGE_READONLY,         // read access
			0,    //the maximum size of the file-mapping object is equal to the current size of the file
			0,
			nullptr);                // name of mapping object

		if (m_hMapFile == INVALID_HANDLE_VALUE) 
		{ 
			PRINT_DEBUG_INFO("Could not create file mapping object (%d).\n", GetLastError());
			return;
		}

		DWORD FileLen = GetFileSize(m_hFile, nullptr);
		m_pStart = (const unsigned char*)MapViewOfFile(m_hMapFile,// handle to map object
			FILE_MAP_READ, // read permission
			0,                   
			0,                   
			FileLen);
		m_pEnd = m_pStart + FileLen;

		if (m_pStart == nullptr) 
		{
			PRINT_DEBUG_INFO("Could not map view of file (%d).\n", GetLastError());
		}
	}

	void CFileMapping::Close()
	{
		if (m_pStart)
		{
			UnmapViewOfFile(m_pStart);
			m_pStart = nullptr;
		}

		if (m_hMapFile)
		{
			CloseHandle(m_hMapFile);
			m_hMapFile = INVALID_HANDLE_VALUE;
		}

		if (m_hFile)
		{
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}
	}

	CFileMapping::~CFileMapping(void)
	{
		Close();
	}

}
