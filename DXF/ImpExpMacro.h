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

#ifdef DXF_LIB
	#define DXF_API __declspec(dllexport)
#else
	#define DXF_API __declspec(dllimport)
	#define EXPIMP_TEMPLATE extern
#endif
//disable warnings : class 'type' needs to have dll-interface to be used by clients of class 'type2' 
#pragma warning (disable : 4251)