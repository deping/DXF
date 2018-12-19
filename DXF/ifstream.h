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

#include "FileMapping.h"
#ifndef EOF
#define EOF -1
#endif

//文件输入流ifstream是个很好的读固定格式文件的类，但很不可靠，
//operator>>经常莫名其妙的fail，或read函数只读出一部分数据。
//所以写了以下模仿类，要特别注意setdelimiter方法，这是我为了简化
//operator>>而特设的，用户应该在调用operator>>之前调用。
namespace DXF
{
	const int goodbit = 0x0;
	const int eofbit = 0x1;
	const int failbit = 0x2;
	const int statmask = 0x3;
	typedef unsigned char seekdir; 
	const seekdir beg=0, cur=1, end=2;

	//所有影响流指针的操作，除了seekg，如果流状态失败(fail()==true)，都会立即返回。

	class ifstream
	{
	public:
		ifstream(void);
		ifstream(const char* filename);
		ifstream(const wchar_t* filename);
		void reset()
		{
			clear();
			seekg(0);
		}
		//call close()
		~ifstream(void);
		//open a file
		void open(const char* filename);
		void open(const wchar_t* filename);
		//close a file
		void close();
		//status
		//Determines if a file is open.
		bool is_open();
		// test if eofbit is set in stream state
		bool eof();
		// test if badbit or failbit is set in stream state
		bool fail();
		// test if no state bits are set
		bool good();
		// test if any stream operation has failed
		operator void*();
		// test if no stream operation has failed
		bool operator!();
		//设置流状态
		void clear(int State=goodbit);
		//读取流状态
		int rdstate();
		//在流的原状态之上添加新状态
		void setstate(int state);
		//设置除了空白之外的field分隔符，将影响所有的operator>>，
		//如果传入0，表示只接受空白作为分隔符。
		void setdelimiter(const char* delimiter);
		//读布尔值
		ifstream& operator>>(bool& Val);
		//读字符串
		ifstream& operator>>(char* Str);
		//读取下一个字符，不管它是不是分隔符。
		ifstream& operator>>(char& Val);
		//读取浮点数
		ifstream& operator>>(float& Val);
		ifstream& operator>>(double& Val);
		//读取整数
		ifstream& operator>>(short& Val);
		ifstream& operator>>(unsigned short& Val);
		ifstream& operator>>(int& Val);
		ifstream& operator>>(unsigned int& Val);
		ifstream& operator>>(long& Val);
		ifstream& operator>>(unsigned long& Val);
		//取得流的当前指针
		unsigned long tellg();
		//设置流的当前指针
		ifstream& seekg(unsigned long pos);
		ifstream& seekg(long off, seekdir origin);
		//取得文件大小
		unsigned long getsize();
		//getline & read
		ifstream& getline(char *Str, unsigned int count);
		ifstream& read(char *Str, unsigned int count);
		//当count等于numeric_limits<unsigned int>::max()时表示只考虑stop,不考虑count。
		//流的当前指针前进count个字节。如果在这之前找到stop字符，那么流的当前指针停在stop
		//字符之后。
		ifstream& ignore(unsigned int count, char stop=EOF);
		//同上。不同的是可以传入多个stop字符。
		ifstream& ignore(unsigned int count, const char* stop);
		//流的当前指针回退count个字节。如果退到文件开始之前，failbit将被设置，
		//同时，流的当前指针指向文件开始处。
		ifstream& backward(unsigned int count);
		//跳过空白
		ifstream& skipws();
	private:
		void init();
		void getfld(char *buffer);
		void do_get(__int64& Val);
		void do_get(double& Val);
		bool isdelim(char code);
		CFileMapping m_FileMapping;
		const char* m_pStart;//文件的起点映射至虚拟内存的位置
		const char* m_pEnd;//文件的终点映射至虚拟内存的位置
		const char* m_pCur;//当前文件指针位置
		int m_state;
		char* m_delimiter;
	};
}
