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

typedef void *HANDLE;

namespace DXF
{
	class CFileMapping
	{
	public:
		CFileMapping();
		CFileMapping(const char* FileName);
		~CFileMapping(void);
		void Open(const char* FileName);
		void Close();

		const unsigned char* GetStart() const{ return m_pStart;}
		const unsigned char* GetEnd() const{ return m_pEnd;}
	private:
		HANDLE m_hFile;//打开的文件句柄
		HANDLE m_hMapFile;//文件映射句柄
		const unsigned char* m_pStart;//文件的起点映射至虚拟内存的位置
		const unsigned char* m_pEnd;//文件的终点映射至虚拟内存的位置
	};

}
