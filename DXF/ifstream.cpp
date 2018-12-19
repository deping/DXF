/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include "ifstream.h"

#include <string>

#include <atlconv.h>
#include <atlexcept.h>

#include "utility.h"

#pragma warning(disable:4996)
#pragma warning(disable:4800)

#define MAX_EXP_DIG	8	/* for parsing numerics */
#define MAX_INT_DIG	32
#define MAX_SIG_DIG	36
#define MAX_BOOL_DIG 6

namespace DXF
{
	void ifstream::init()
	{
		m_pStart = nullptr;
		m_pEnd = nullptr;
		m_pCur = nullptr;
		m_state = statmask;
		m_delimiter = 0;
	}

	ifstream::ifstream(void)
	{
		init();
	}

	ifstream::ifstream(const char* filename)
	{
		init();
		open(filename);
	}

	ifstream::ifstream(const wchar_t* filename)
	{
		init();
		open(filename);
	}

	ifstream::~ifstream(void)
	{
		close();
	}

	void ifstream::open(const char* filename)
	{
		m_FileMapping.Open(filename);
		m_pStart = (const char*)m_FileMapping.GetStart();
		m_pEnd = (const char*)m_FileMapping.GetEnd();
		m_pCur = m_pStart;
		m_state = goodbit;
		if(nullptr == m_pStart)
			m_state |= failbit;
	}

	void ifstream::open(const wchar_t* filename)
	{
		m_FileMapping.Open(ATL::CW2A(filename));
		m_pStart = (const char*)m_FileMapping.GetStart();
		m_pEnd = (const char*)m_FileMapping.GetEnd();
		m_pCur = m_pStart;
		m_state = goodbit;
		if(nullptr == m_pStart)
			m_state |= failbit;
	}

	void ifstream::close()
	{
		m_FileMapping.Close();
		m_pStart = nullptr;
		m_pEnd = nullptr;
		m_pCur = nullptr;
		m_state = statmask;

		if(m_delimiter)
		{
			free(m_delimiter);
			m_delimiter = nullptr;
		}
	}

	bool ifstream::is_open()
	{
		return nullptr != m_pStart;
	}

	bool ifstream::good()
	{
		return (rdstate() == goodbit);
	}

	bool ifstream::eof()
	{
		return (rdstate() & eofbit);
	}

	bool ifstream::fail()
	{
		return (rdstate() & (failbit|eofbit)) != 0;
	}

	bool ifstream::operator!()
	{
		return fail();
	}

	ifstream::operator void *()
	{
		return (fail() ? 0 : (void *)this);
	}

	void ifstream::clear(int state)
	{
		m_state = state & statmask;
	}

	int ifstream::rdstate()
	{
		return m_state;
	}

	void ifstream::setstate(int state)
	{
		if (state != goodbit)
			clear(rdstate() | state);
	}

	ifstream& ifstream::skipws()
	{
		while(!fail())
		{
			if(iswspace(*m_pCur))//不可使用isspace，否则文本文件有中文字符时，导致调试时断言失败
			{
				++m_pCur;
				if(m_pCur == m_pEnd)
					setstate(eofbit);
			}
			else
				break;
		}
		return *this;
	}

	//helper functions
	bool ifstream::isdelim(char code)
	{
		if(iswspace(code))//不可使用isspace，否则文本文件有中文字符时，导致调试时断言失败
			return true;
		if(m_delimiter != 0)
		{
			size_t count = strlen(m_delimiter);
			for(size_t i=0; i<count; ++i)
			{
				if(code == m_delimiter[i])
					return true;
			}
		}
		return false;
	}

	// get field from file at current read position into buffer
	void ifstream::getfld(char *buffer)
	{
		skipws();
		while(!fail())
		{
			if(isdelim(*m_pCur))
				break;
			*buffer++ = *m_pCur++;
			if(m_pCur == m_pEnd)
				setstate(eofbit);
		}
		*buffer = 0;
	}

	void ifstream::do_get(__int64& Val)
	{
		char buffer[MAX_INT_DIG], *Ep;
		getfld(buffer);
		__int64 Ans = _strtoi64(buffer, &Ep, 10);

		if (*Ep != 0 || buffer == Ep)
			setstate(failbit);
		else
			Val = Ans;	// deliver value
	}

	void ifstream::do_get(double& Val)
	{
		char buffer[MAX_EXP_DIG + MAX_SIG_DIG + 16], *Ep;
		getfld(buffer);
		double Ans = strtod(buffer, &Ep);

		if (*Ep != 0 || buffer == Ep)
			setstate(failbit);
		else
			Val = Ans;	// deliver value
	}

	ifstream& ifstream::operator>>(char* Str)
	{
		getfld(Str);
		return (*this);
	}

	ifstream& ifstream::operator>>(char& Val)
	{
		//__int64 temp;
		//do_get(temp);
		//Val = (char)temp;
		if(!fail())
			Val = *m_pCur++;
		else
			Val = 0;
		if(m_pCur == m_pEnd)
			setstate(eofbit);
		return (*this);
	}

	ifstream& ifstream::operator>>(short& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (short)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(unsigned short& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (unsigned short)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(int& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (int)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(unsigned int& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (unsigned int)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(long& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (long)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(unsigned long& Val)
	{
		__int64 temp;
		do_get(temp);
		Val = (unsigned long)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(float& Val)
	{
		double temp;
		do_get(temp);
		Val = (float)temp;
		return (*this);
	}

	ifstream& ifstream::operator>>(double& Val)
	{
		do_get(Val);
		return (*this);
	}

	ifstream& ifstream::operator>>(bool& Val)
	{
		static const char* const boolname[2] = {"false", "true"};
		char buffer[MAX_BOOL_DIG], *Ep;
		getfld(buffer);
		for(int i=0; i<2; ++i)
		{
			if(stricmp(buffer, boolname[i]) == 0)
			{
				Val = (bool)i;
				return (*this);
			}
		}
		__int64 temp = _strtoi64(buffer, &Ep, 10);
		if((temp != 0 && temp != 1) || (*Ep != 0))
		{
			setstate(failbit);
			return (*this);
		}
		Val = (bool)temp;
		return (*this);
	}

	void ifstream::setdelimiter(const char* delimiter)
	{
		if(m_delimiter)
		{
			free(m_delimiter);
			m_delimiter = nullptr;
		}
		if(delimiter == nullptr)
		{
			m_delimiter = nullptr;
			return;
		}
		size_t count = strlen(delimiter);
		m_delimiter = (char*)malloc(count+1);
		strcpy(m_delimiter, delimiter);
	}

	unsigned long ifstream::tellg()
	{
		return (unsigned long)(m_pCur - m_pStart);
	}

	unsigned long ifstream::getsize()
	{
		return (unsigned long)(m_pEnd - m_pStart);
	}

	ifstream& ifstream::seekg(unsigned long pos)
	{
		m_pCur = m_pStart + pos;
		if((m_pCur < m_pStart) || (m_pCur == m_pEnd))
			setstate(failbit|eofbit);
		return *this;
	}

	ifstream& ifstream::seekg(long off, seekdir origin)
	{
		switch(origin)
		{
		case beg:
			m_pCur = m_pStart + off;
			break;
		case cur:
			m_pCur = m_pCur + off;
			break;
		case end:
			m_pCur = m_pEnd + off;
			break;
		default:
			ASSERT_DEBUG_INFO(false);
			break;
		};
		if((m_pCur < m_pStart) || (m_pCur >= m_pEnd))
			setstate(failbit|eofbit);
		return *this;
	}

	ifstream& ifstream::getline(char *Str, unsigned int count)
	{
		for(unsigned int i=0; (i<count-1) && !fail(); ++i)
		{
			if((*m_pCur) != '\n')
			{
				*Str++ = *m_pCur++;
				if(m_pCur == m_pEnd)
					setstate(eofbit);
			}
			else
			{
				++m_pCur;
				if(m_pCur == m_pEnd)
					setstate(eofbit);
				if(*(Str-1) == '\r')
					*(Str-1) = 0;
				break;
			}
		}
		*Str = 0;
		return *this;
	}

	ifstream& ifstream::read(char *Str, unsigned int count)
	{
		if(!fail())
		{
			int count2 = min(count-1, (unsigned int)(m_pEnd-m_pCur));
			strncpy(Str, m_pCur, count2);
			*(Str+count2) = '\0';
			m_pCur += count2;
			if(m_pCur == m_pEnd)
				setstate(eofbit);
		}
		return *this;
	}

	ifstream& ifstream::ignore(unsigned int count, char stop)
	{
		for(unsigned int i=1; (i<=count) && !fail(); ++i)
		{
			char code = (*m_pCur);
			++m_pCur;
			if(m_pCur == m_pEnd)
				setstate(eofbit);
			if(code == stop)
				break;
		}
		return *this;
	}

	ifstream& ifstream::ignore(unsigned int count, const char* stop)
	{
		ASSERT_DEBUG_INFO(stop);
		size_t len = strlen(stop);
		for(unsigned int i=1; (i<=count) && !fail(); ++i)
		{
			char code = (*m_pCur);
			++m_pCur;
			if(m_pCur == m_pEnd)
				setstate(eofbit);
			for(size_t j=0; j<len; ++j)
			{
				if(code == stop[j])
					goto exit;
			}
		}
	exit:
		return *this;
	}

	ifstream& ifstream::backward(unsigned int count)
	{
		if(!fail())
		{
			m_pCur -= count;
			if(m_pCur < m_pStart)
			{
				m_pCur = m_pStart;
				setstate(failbit);
			}
		}
		return *this;
	}
}
