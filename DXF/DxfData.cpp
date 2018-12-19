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
#include "DxfData.h"
#include "atlbase.h"
#include <float.h>
#include <eh.h>

#include <map>
#include <utility>
#include <AtlConv.h>
#include "ifstream.h"
#include "DxfReader.h"
#include "DxfWriter.h"
#include <iomanip>

#pragma warning(disable:4996)

namespace DXF {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
DxfData::DxfData()
	: m_LinetypeScale(1.0)
	, m_TmpBlockNo(0)
{
}

DxfData::~DxfData()
{
}

std::shared_ptr<EntAttribute> DxfData::FindEntity(int handle)
{
	auto it = m_Objects.FindEntity(handle);
	if (it != m_Objects.end())
		return *it;

	for (const auto& bd : m_RealBlockDefs)
	{
		it = bd.second->m_Objects.FindEntity(handle);
		if (it != bd.second->m_Objects.end())
			return *it;
	}

	for (const auto& ld : m_Layouts)
	{
		auto it = ld.second->m_Objects.FindEntity(handle);
		if (it != ld.second->m_Objects.end())
			return *it;
	}

	return nullptr;
}

void DxfData::Reset()
{
	m_Linetypes.clear();
	m_Layers.clear();
	m_TextStyles.clear();
	m_DimStyles.clear();
	m_MLeaderStyles.clear();
	m_TableStyles.clear();

	m_Objects.clear();
	m_Layouts.clear();
	m_RealBlockDefs.clear();
	m_TmpBlockNo = 0;
}

int DxfData::PrepareBeforeWrite()
{
	// Remove must-not elements
	m_Linetypes.erase("ByBlock");
	m_Linetypes.erase("ByLayer");
	m_Linetypes.erase("Continuous");

	// Add must-have elements
	if (m_Layers.find("0") == m_Layers.end())
	{
		m_Layers["0"] = LayerData();
	}

	if (m_TextStyles.find("Standard") == m_TextStyles.end())
	{
		m_TextStyles["Standard"] = TextStyleData();
	}

	if (m_DimStyles.find("ISO-25") == m_DimStyles.end())
	{
		m_DimStyles["ISO-25"] = DimStyleData(0);
	}

	if (m_MLeaderStyles.find("Standard") == m_MLeaderStyles.end())
	{
		m_MLeaderStyles["Standard"] = MLeaderStyle();
	}

	if (m_MLeaderStyles.find("Annotative") == m_MLeaderStyles.end())
	{
		m_MLeaderStyles["Annotative"] = MLeaderStyle();
		m_MLeaderStyles["Annotative"].m_IsAnnotative = true;
	}

	if (m_TableStyles.find("Standard") == m_TableStyles.end())
	{
		m_TableStyles["Standard"] = TableStyle();
	}

	// Generate blocks for dimensions
	char blockName[24];
	auto GenerateBlockName = [&blockName](int blockNo) {
		_snprintf_s(blockName, 24, _TRUNCATE, (const char *)"*D%d", blockNo);
	};
	for (const auto& ent : m_Objects)
	{
		AcadDim* pDim = dynamic_cast<AcadDim*>(ent.get());
		if (pDim && pDim->m_BlockName.empty())
		{
			GenerateBlockName(++m_TmpBlockNo);
			pDim->GenerateBlock(*this, blockName);
		}
	}

	for (const auto& bd : m_RealBlockDefs)
	{
		for (const auto& ent : bd.second->m_Objects)
		{
			AcadDim* pDim = dynamic_cast<AcadDim*>(ent.get());
			if (pDim && pDim->m_BlockName.empty())
			{
				GenerateBlockName(++m_TmpBlockNo);
				pDim->GenerateBlock(*this, blockName);
			}
		}
	}

	for (const auto& ld : m_Layouts)
	{
		for (const auto& ent : ld.second->m_Objects)
		{
			AcadDim* pDim = dynamic_cast<AcadDim*>(ent.get());
			if (pDim && pDim->m_BlockName.empty())
			{
				GenerateBlockName(++m_TmpBlockNo);
				pDim->GenerateBlock(*this, blockName);
			}
		}
	}

	// Assign handle to entities. Start from 10000, it should be enough to hold other handles.
	int handle = 10000;

	// It is harmless to assign handle to entities even if it is not exported to DXF.
	for (const auto& pb : DxfWriter::s_PredefinedBlocks)
	{
		for (const auto& ent : *pb)
		{
			ent->AssignHandle(handle);
		}
	}

	for (const auto& ent : m_Objects)
	{
		ent->AssignHandle(handle);
	}

	for (const auto& bd : m_RealBlockDefs)
	{
		for (const auto& ent : bd.second->m_Objects)
		{
			ent->AssignHandle(handle);
		}
	}

	// Add must-have layout
	if (m_Layouts.empty())
	{
		auto pLayout = std::make_shared<LayoutData>();
		m_Layouts["Layout1"] = pLayout;
	}

	int i = -1;
	for (const auto& ld : m_Layouts)
	{
		// Assign block name to layouts
		if (i == -1)
		{
			ld.second->m_BlockName = "*Paper_Space";
		}
		else
		{
			char blockName[16];
			_snprintf(blockName, sizeof(blockName), "*Paper_Space%d", i);
			ld.second->m_BlockName = blockName;
		}
		ld.second->m_LayoutOrder = i + 2;
		++i;
		for (const auto& ent : ld.second->m_Objects)
		{
			ent->AssignHandle(handle);
		}
	}

	return handle;
}

}
