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

#include <atlconv.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <time.h>
#include <algorithm>

#include "DxfReader.h"
#include "DxfData.h"
#include "ifstream.h"

using namespace std;
#pragma warning(disable:4996)

namespace DXF
{

	const char g_PaperSpace[] = "*Paper_Space";
	const size_t g_LenOfPaperSpace = strlen(g_PaperSpace);

	DxfReader::DxfReader(DxfData& graph)
		: m_Graph(graph)
		, m_DxfFile(*new DXF::ifstream)
	{
	}

	DxfReader::~DxfReader(void)
	{
		delete &m_DxfFile;
	}

	bool DxfReader::ReadDxf(const char* dxfFileName, bool OnlyReadModelSpace)
	{
		TRACE_FUNCTION("(dxfFileName = %s, OnlyReadModelSpace = %s)", dxfFileName, OnlyReadModelSpace ? "true" : "false");

		srand((unsigned)time(nullptr));
		m_Graph.Reset();
		Reset();

		m_OnlyReadModelSpace = OnlyReadModelSpace;
		m_DxfFile.open(dxfFileName);
		bool bSuccess = true;
		try
		{
			bSuccess = m_DxfFile.is_open() && m_DxfFile.good();
			bSuccess = bSuccess && ReadHeader();

			bSuccess = bSuccess && ReadTables();
			bSuccess = bSuccess && ReadBlocks();
			bSuccess = bSuccess && ReadEntities();
			bSuccess = bSuccess && ReadObjects();
			if (m_DxfFile.is_open())
				m_DxfFile.close();
			ConvertHandle();
		}
		catch (const std::logic_error&)
		{
			bSuccess = false;
		}
		catch (const std::runtime_error&)
		{
			bSuccess = false;
		}
		if (!bSuccess)
		{
			m_Graph.Reset();
		}
		return bSuccess;
	}

	void DxfReader::ReadPair(int& groupCode, char* value, int bufsize)
	{
		// Don't use TRACE_FUNCTION, it is a high frequent function.

		m_DxfFile >> groupCode;
		m_DxfFile.ignore(INT_MAX, '\n');
		int curPos = m_DxfFile.tellg();
		m_DxfFile.ignore(INT_MAX, '\n');
		int nextPos = m_DxfFile.tellg();
		m_DxfFile.seekg(curPos);
		m_DxfFile.getline(value, bufsize);
		// Trim whitespace from both ends of value
		size_t len = strlen(value);
		char* pStart = value;
		char* pEnd = value + len;
		while (pStart < pEnd)
		{
			if (iswspace(*pStart) == 0)
				break;
			++pStart;
		}
		while (pEnd > pStart)
		{
			char* pTemp = pEnd - 1;
			if (iswspace(*pTemp))
			{
				pEnd = pTemp;
			}
			else
			{
				break;
			}
		}
		*pEnd = 0;
		if (value != pStart)
		{
			size_t len = size_t(pEnd - pStart);
			memmove(value, pStart, len);
			value[len] = 0;
		}
		m_DxfFile.seekg(nextPos);
	}

	bool DxfReader::AdvanceToSection(const char* sectionName)
	{
		char buffer[BUFSIZE];
		int num;
		int phase = 0;
		while (phase < 2 && !m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (0 == phase)
			{
				if (num != 0 || (stricmp("SECTION", buffer) != 0))
					continue;
			}
			else if (1 == phase)
			{
				if (num != 2 || (stricmp(buffer, sectionName) != 0))
				{
					phase = 0;
					continue;
				}
			}
			++phase;
		}
		return 2 == phase;
	}

	bool DxfReader::AdvanceToObject(const char * objectType)
	{
		char buffer[BUFSIZE];
		int num;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0 && (stricmp(objectType, buffer) == 0))
			{
				m_DxfFile.seekg(pos);
				return true;
			}
		}
		return false;
	}

	bool DxfReader::AdvanceToObjectStopAtLayout(const char * objectType)
	{
		char buffer[BUFSIZE];
		int num;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				if (stricmp(objectType, buffer) == 0)
				{
					m_DxfFile.seekg(pos);
					return true;
				}
				else if (stricmp(objectType, "LAYOUT") == 0)
				{
					m_DxfFile.seekg(pos);
					return true;
				}
			}
		}
		return false;
	}

	bool DxfReader::NextVariable(char* varName, char* varVal)
	{
		int num;
		ReadPair(num, varName);
		if (num != 9)
			return false;
		ReadPair(num, varVal);
		return true;
	}

	bool DxfReader::ReadHeader()
	{
		bool bSuccess = AdvanceToSection("HEADER");
		if (!bSuccess)
			return bSuccess;
		char varName[BUFSIZE], varVal[BUFSIZE];
		while (NextVariable(varName, varVal) && !m_DxfFile.fail())
		{
			if (stricmp(varName, "$LTSCALE") == 0)
				m_Graph.m_LinetypeScale = atof(varVal);
		}
		return bSuccess;
	}

	bool DxfReader::NextTable(char* tableName)
	{
		char buffer[BUFSIZE];
		int num;
		int phase = 0;
		while (phase < 2 && !m_DxfFile.fail())
		{
			if (0 == phase)
			{
				ReadPair(num, buffer);
				if (num == 0)
				{
					if (stricmp("ENDSEC", buffer) == 0)
					{
						return false;//遇到段结尾退出，防止不停地找下去
					}
					else if (stricmp("TABLE", buffer) != 0)
					{
						continue;
					}
				}
				else
				{
					continue;
				}
			}
			else if (1 == phase)
			{
				ReadPair(num, tableName);
				if (num != 2)
				{
					phase = 0;
					continue;
				}
			}
			++phase;
		}
		return 2 == phase;
	}

	bool DxfReader::NextTableEntry(const char* entryName)
	{
		char buffer[BUFSIZE];
		int num;
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 0)
			{
				if (stricmp("ENDTAB", buffer) == 0)
				{
					return false;//遇到表结尾退出，防止不停地找下去
				}
				else if (stricmp(entryName, buffer) == 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	void DxfReader::ReadTableEntry(std::string& name, void* data)
	{
		int groupCode;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(groupCode, buffer);
			if (groupCode != 0)
			{
				(this->*HandleTableEntryCode)(name, data, groupCode, buffer);
			}
			else
			{
				m_DxfFile.seekg(pos);
				break;
			}
		}
	}

	void DxfReader::HandleBlockRecordEntryCode(std::string& name, void* data, int groupCode, const char* value)
	{
		int* handle = (int*)data;
		switch (groupCode)
		{
		case 2:
			name = value;
			break;
		case 5:
			*handle = hextoi(value);
			break;
		default:
			break;
		}
	}

	void DxfReader::HandleLayerEntryCode(std::string& name, void* data, int groupCode, const char* value)
	{
		LayerData* ld = (LayerData*)data;
		switch (groupCode)
		{
		case 2:
			name = value;
			break;
		case 62:
			ld->m_Color = atol(value);
			break;
		case 6:
			ld->m_Linetype = value;
			break;
		case 290:
			ld->m_Plottable = atoi(value) != 0;
			break;
		case 370:
			ld->m_LineWeight = atol(value);
			break;
		default:
			break;
		}
	}

	void DxfReader::HandleLTypeEntryCode(std::string& name, void* data, int groupCode, const char* value)
	{
		int* handle = (int*)data;
		switch (groupCode)
		{
		case 2:
			name = value;
			break;
		case 5:
			*handle = hextoi(value);
			break;
		default:
			break;
		}
	}

	struct TextStyleDataExt
	{
		TextStyleData tsd;
		int handle;
	};

	const char* FileNameFromCadFace(const char* cadFace);
	void DxfReader::HandleStyleEntryCode(std::string& name, void* data, int groupCode, const char* value)
	{
		TextStyleDataExt* tsd = (TextStyleDataExt*)data;
		switch (groupCode)
		{
		case 2:
			name = value;
			break;
		case 3:
			tsd->tsd.m_PrimaryFontFile = value;
			break;
		case 1001:
			if (strcmp("ACAD", value) == 0)
				tsd->tsd.m_TrueType = "true";
			break;
		case 1000:
			if (tsd->tsd.m_TrueType == "true")
				tsd->tsd.m_TrueType = value;
			break;
		case 4:
			tsd->tsd.m_BigFontFile = value;
			break;
		case 5:
			tsd->handle = hextoi(value);
			break;
		case 40:
			tsd->tsd.m_Height = atof(value);
			break;
		case 41:
			tsd->tsd.m_WidthFactor = atof(value);
			break;
		case 50:
			tsd->tsd.m_ObliqueAngle = atof(value);
			break;
		default:
			break;
		}
	}

	void DxfReader::HandleDimStyleEntryCode(std::string& name, void* data, int groupCode, const char* value)
	{
		DimStyleData* dsd = (DimStyleData*)data;
		switch (groupCode)
		{
		case 2:
			name = value;
			break;
		case 3:
			//case 4:
			dsd->Text = value;
			break;
		case 41:
			dsd->ArrowHeadSize = atof(value);
			break;
		case 42:
			dsd->ExtensionLineOffset = atof(value);
			break;
		case 44:
			dsd->ExtensionLineExtend = atof(value);
			break;
		case 140:
			dsd->TextHeight = atof(value);
			break;
		case 141:
			dsd->CenterMarkSize = atof(value);
			{
				if (dsd->CenterMarkSize > 0)
					dsd->CenterType = acCenterMark;
				else if (dsd->CenterMarkSize == 0)
					dsd->CenterType = acCenterNone;
				else
				{
					dsd->CenterType = acCenterLine;
					dsd->CenterMarkSize = -dsd->CenterMarkSize;
				}
			}
			break;
		case 144:
			dsd->LinearScaleFactor = atof(value);
			break;
		case 77:
		{
			long temp = atol(value);
			if (1L == temp)
				dsd->VerticalTextPosition = acAbove;
		}
		break;
		case 145:
			dsd->VerticalTextPosition = atol(value);
			break;
		case 147:
			dsd->TextGap = atof(value);
			break;
		case 73:
			//case 74:
			dsd->TextAlign = atoi(value) == 1;
			break;
		case 75:
			dsd->ExtLine1Suppress = atoi(value) == 1;
			break;
		case 76:
			dsd->ExtLine2Suppress = atoi(value) == 1;
			break;
		case 78:
		{
			int flag = atoi(value);
			dsd->SuppressLeadingZeros = (flag & 4) != 0;
			dsd->SuppressTrailingZeros = (flag & 8) != 0;
		}
		break;
		case 79:
		{
			int flag = atoi(value);
			dsd->AngleSuppressLeadingZeros = (flag & 1) != 0;
			dsd->AngleSuppressTrailingZeros = (flag & 2) != 0;
		}
		break;
		case 171:
			dsd->UnitsPrecision = atol(value);
			break;
		case 172:
			dsd->ForceLineInside = atoi(value) == 1;
			break;
		case 174:
			dsd->TextInside = atoi(value) == 1;
			break;
		case 176:
			dsd->DimensionLineColor = atol(value);
			break;
		case 177:
			dsd->ExtensionLineColor = atol(value);
			break;
		case 178:
			dsd->TextColor = atol(value);
			break;
		case 179:
			dsd->AnglePrecision = atol(value);
			break;
		case 271:
			dsd->UnitsPrecision = atol(value);
			break;
			//case 273:
		case 277:
			dsd->UnitsFormat = atol(value);
			break;
		case 275:
			dsd->AngleFormat = atol(value);
			break;
		case 279:
			dsd->TextMovement = atol(value);
			break;
		case 280:
			dsd->HorizontalTextPosition = atol(value);
			break;
		case 281:
			dsd->DimLine1Suppress = atoi(value) == 1;
			break;
		case 282:
			dsd->DimLine2Suppress = atoi(value) == 1;
			break;
		case 289:
			dsd->Fit = atol(value);
			break;
		case 340:
			// 此时调用LookupTextStyleName(handle)一般不会成功。
			dsd->TextStyle = value;
			break;
		case 342:
			// 此时调用LookupBlockEntryName(handle)一般不会成功。
			dsd->ArrowHead1Block = value;
			dsd->ArrowHead2Block = value;
			break;
		case 343:
			dsd->ArrowHead1Block = value;
			break;
		case 344:
			dsd->ArrowHead2Block = value;
			break;
		case 371:
			dsd->DimensionLineWeight = atol(value);
			break;
		case 372:
			dsd->ExtensionLineWeight = atol(value);
			break;
		default:
			break;
		}
	}

	bool DxfReader::ReadTables()
	{
		TRACE_FUNCTION("()");

		bool bSuccess = AdvanceToSection("TABLES");
		if (!bSuccess)
			return bSuccess;
		char tableName[BUFSIZE];
		while (NextTable(tableName))
		{
			PrintDebugInfo("Read Table %s", tableName);
			while (NextTableEntry(tableName))
			{
				void* data = nullptr;
				if (stricmp(tableName, "BLOCK_RECORD") == 0)
				{
					HandleTableEntryCode = &DxfReader::HandleBlockRecordEntryCode;
					data = new int;
				}
				else if (stricmp(tableName, "LAYER") == 0)
				{
					HandleTableEntryCode = &DxfReader::HandleLayerEntryCode;
					LayerData* ld = new LayerData;
					ld->m_Plottable = true;
					ld->m_LineWeight = acLnWtByLayer;
					data = ld;
				}
				else if (stricmp(tableName, "LTYPE") == 0)
				{
					HandleTableEntryCode = &DxfReader::HandleLTypeEntryCode;
					data = new int;
				}
				else if (stricmp(tableName, "STYLE") == 0)
				{
					HandleTableEntryCode = &DxfReader::HandleStyleEntryCode;
					TextStyleDataExt* tsd = new TextStyleDataExt;
					tsd->tsd.m_WidthFactor = 1.0;
					tsd->tsd.m_ObliqueAngle = 0.0;
					data = tsd;
				}
				else if (stricmp(tableName, "DIMSTYLE") == 0)
				{
					HandleTableEntryCode = &DxfReader::HandleDimStyleEntryCode;
					data = new DimStyleData;
				}
				else
				{
					HandleTableEntryCode = nullptr;
				}

				if (HandleTableEntryCode)
				{
					std::string name;
					ReadTableEntry(name, data);
					PrintDebugInfo("Read Entry %s", name.c_str());
					if (stricmp(tableName, "BLOCK_RECORD") == 0)
					{
						int* handle = (int*)data;
						if (!name.empty())
							m_BlockEntryHandleTable[*handle] = name;
						delete handle;
					}
					else if (stricmp(tableName, "LAYER") == 0)
					{
						LayerData* ld = (LayerData*)data;
						if (!name.empty())
							m_Graph.m_Layers[name] = *ld;
						delete ld;
					}
					else if (stricmp(tableName, "LTYPE") == 0)
					{
						// The 3 linetypes are not exist : acad.lin or acadiso.lin file
						if (name != "ByBlock" && name != "ByLayer" && name != "Continuous" && !name.empty())
						{
							m_Graph.m_Linetypes.insert(name);
						}
						int* handle = (int*)data;
						if (!name.empty())
							m_LinetypeHandleTable[*handle] = name;
						delete handle;
					}
					else if (stricmp(tableName, "STYLE") == 0)
					{
						TextStyleDataExt* tsd = (TextStyleDataExt*)data;
						if (!name.empty())
						{
							m_Graph.m_TextStyles[name] = tsd->tsd;
							m_TextStyleHandleTable[tsd->handle] = name;
						}
						delete tsd;
					}
					else if (stricmp(tableName, "DIMSTYLE") == 0)
					{
						DimStyleData* dsd = (DimStyleData*)data;
						if (!name.empty())
						{
							m_Graph.m_DimStyles[name] = *dsd;
							DimStyleData* dsd2 = &m_Graph.m_DimStyles[name];
							m_ToBeConvertedTextStyleName[name] = hextoi(dsd2->TextStyle.c_str());
							if (!dsd2->ArrowHead1Block.empty())
								m_ToBeConvertedArrow1Name[name] = hextoi(dsd2->ArrowHead1Block.c_str());
							if (!dsd2->ArrowHead2Block.empty())
								m_ToBeConvertedArrow2Name[name] = hextoi(dsd2->ArrowHead2Block.c_str());
						}
						delete dsd;
					}
				}

			}
		}
		return bSuccess;
	}

	bool DxfReader::NextBlock()
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 0 && stricmp(buffer, "BLOCK") == 0)
				return true;
			if (num == 0 && stricmp(buffer, "ENDSEC") == 0)
				return false;
		}
		return false;
	}

	bool DxfReader::NextObject(const char * objectType)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				if (stricmp(buffer, objectType) == 0)
				{
					ReadPair(num, buffer);
					if (num != 5)
					{
						THROW_INVALID_DXF();
						return false;
					}
					m_CurObjectHandle = hextoi(buffer);
					return true;
				}
				else
				{
					m_DxfFile.seekg(pos);
					return false;
				}
			}
		}
		return false;
	}

	void DxfReader::ReadDictionaryEntries(std::function<bool(const NameHandle&)> HandleDicEntry)
	{
		int num;
		char buffer[BUFSIZE];
		NameHandle curDicEntry;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				return;
			}
			switch (num)
			{
			case 3:
				curDicEntry.name = buffer;
				break;
			case 350:
				curDicEntry.handle = hextoi(buffer);
				if (HandleDicEntry(curDicEntry))
					return;
				break;
			}
		}
	}

	LayoutData* DxfReader::ReadBlockBegin(std::string& blockName, BlockDef* block)
	{
		int groupCode;
		char buffer[BUFSIZE];
		LayoutData* pLayout = nullptr;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(groupCode, buffer);
			if (groupCode != 0)
			{
				switch (groupCode)
				{
				case 2:
					blockName = buffer;
					if (buffer[0] == '*')
					{
						// Paperspace blocks
						if (strnicmp(buffer, g_PaperSpace, g_LenOfPaperSpace) == 0)
						{
							pLayout = new LayoutData();
						}
						// Temporary blocks
					}
					break;
				case 10:
					block->m_InsertPoint.x = atof(buffer);
					break;
				case 20:
					block->m_InsertPoint.y = atof(buffer);
					break;
				default:
					break;
				}
			}
			else
			{
				m_DxfFile.seekg(pos);
				break;
			}
		}
		return pLayout;
	}

	bool DxfReader::NextEntity(char* entityType)
	{
		int num;
		while (!m_DxfFile.fail())
		{
			ReadPair(num, entityType);
			if (num == 0)
			{
				if (stricmp(entityType, "ENDSEC") == 0 || stricmp(entityType, "ENDBLK") == 0)
					return false;
				else
					return true;

			}
		}
		return false;
	}

	void DxfReader::ReadAllEntities(EntityList* pObjList, bool bIn_ENTITIES_Section)
	{
		EntAttribute att;
		char buffer[BUFSIZE];
		EntityList* pContainerList = pObjList;
		EntityList* pModelSpaceList = nullptr;
		EntityList* pPaperSpaceList = nullptr;
		std::shared_ptr<LayoutData> pDefaultPaperSpace = nullptr;
		if (bIn_ENTITIES_Section)
		{
			pDefaultPaperSpace = LookupLayout(g_PaperSpace);
			if (pDefaultPaperSpace)
				pPaperSpaceList = &pDefaultPaperSpace->m_Objects;
			pModelSpaceList = &m_Graph.m_Objects;
		}
		while (NextEntity(buffer))
		{
			ReadAttribute();
			if (bIn_ENTITIES_Section)
			{
				if (m_Attribute.m_IsInPaperspace)
				{
					if (nullptr == pDefaultPaperSpace)
						continue;
					pContainerList = pPaperSpaceList;
				}
				else
				{
					pContainerList = pModelSpaceList;
				}
			}

			PrintDebugInfo("Read %s%s", buffer, m_Attribute.m_IsInPaperspace ? "(PS)" : "");

			//按出现概率大小排序
			if (stricmp(buffer, "LINE") == 0)
			{
				ReadLine(pContainerList);
			}
			else if (stricmp(buffer, "TEXT") == 0)
			{
				ReadText(pContainerList);
			}
			else if (stricmp(buffer, "LWPOLYLINE") == 0)
			{
				ReadLwpolyline(pContainerList);
			}
			else if (stricmp(buffer, "ARC") == 0)
			{
				ReadArc(pContainerList);
			}
			else if (stricmp(buffer, "DIMENSION") == 0)
			{
				ReadDimension(pContainerList);
			}
			else if (stricmp(buffer, "INSERT") == 0)
			{
				ReadInsert(pContainerList);
			}
			else if (stricmp(buffer, "CIRCLE") == 0)
			{
				ReadCircle(pContainerList);
			}
			else if (stricmp(buffer, "ELLIPSE") == 0)
			{
				ReadEllipse(pContainerList);
			}
			else if (stricmp(buffer, "HATCH") == 0)
			{
				ReadHatch(pContainerList);
			}
			else if (stricmp(buffer, "LEADER") == 0)
			{
				ReadLeader(pContainerList);
			}
			else if (stricmp(buffer, "MULTILEADER") == 0)
			{
				ReadMLeader(pContainerList);
			}
			else if (stricmp(buffer, "MTEXT") == 0)
			{
				ReadMtext(pContainerList);
			}
			else if (stricmp(buffer, "SPLINE") == 0)
			{
				ReadSpline(pContainerList);
			}
			else if (stricmp(buffer, "VIEWPORT") == 0)
			{
				ReadViewport(pContainerList);
			}
			else if (stricmp(buffer, "SOLID") == 0)
			{
				ReadSolid(pContainerList);
			}
			else if (stricmp(buffer, "POINT") == 0)
			{
				ReadPoint(pContainerList);
			}
			else if (stricmp(buffer, "ACAD_TABLE") == 0)
			{
				ReadTable(pContainerList);
			}
			else if (stricmp(buffer, "ATTDEF") == 0)
			{
				ReadAttDef(pContainerList);
			}
			else if (stricmp(buffer, "ATTRIB") == 0)
			{
				ReadAttrib(pContainerList);
			}
		}
	}

	bool DxfReader::ReadBlocks()
	{
		TRACE_FUNCTION("()");

		bool bSuccess = AdvanceToSection("BLOCKS");
		std::string blockName;
		while (NextBlock())
		{
			auto pBlockDef = std::make_shared<BlockDef>();
			std::shared_ptr<LayoutData> pLayout(ReadBlockBegin(blockName, pBlockDef.get()));
			PrintDebugInfo("Read Block %s", blockName.c_str());
			if (pLayout)
			{
				if (!m_OnlyReadModelSpace)
				{
					pLayout->m_BlockName = blockName;
					// The key should be layout name, but layout name is not available at this time.
					// So use unqiue block name temporarily.
					m_Graph.m_Layouts[blockName] = pLayout;
					ReadAllEntities(&pLayout->m_Objects, false);
				}
			}
			else
			{
				if (blockName.empty() || blockName == "*Model_Space")
					continue;
				// Block name prefix meanings:
				// '_' - predefined arrow head block
				// '*Paper_Space' - Paperspace blocks
				// '*' - temporary blocks
				m_Graph.m_RealBlockDefs[blockName] = pBlockDef;
				ReadAllEntities(&pBlockDef->m_Objects, false);
			}
		}
		return bSuccess;
	}

	bool DxfReader::ReadEntities()
	{
		TRACE_FUNCTION("()");

		bool bSuccess = AdvanceToSection("ENTITIES");
		ReadAllEntities(nullptr, true);
		return bSuccess;
	}

	void DxfReader::ReadLayout(std::string& layoutName, LayoutData& lo)
	{
		bool IsThis330 = false;
		int pos;
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 100 && stricmp(buffer, "AcDbLayout") == 0)
				break;
			switch (num)
			{
			case 2:
				lo.m_PlotSettings.m_PlotConfigFile = buffer;
				break;
			case 4:
				lo.m_PlotSettings.m_PaperName = buffer;
				break;
			case 40:
				lo.m_PlotSettings.m_LeftMargin = atof(buffer);
				break;
			case 41:
				lo.m_PlotSettings.m_BottomMargin = atof(buffer);
				break;
			case 42:
				lo.m_PlotSettings.m_RightMargin = atof(buffer);
				break;
			case 43:
				lo.m_PlotSettings.m_TopMargin = atof(buffer);
				break;
			case 44:
				lo.m_PlotSettings.m_Width = atof(buffer);
				break;
			case 45:
				lo.m_PlotSettings.m_Height = atof(buffer);
				break;
			case 46:
				lo.m_PlotSettings.m_PlotOrigin.x = atof(buffer);
				break;
			case 47:
				lo.m_PlotSettings.m_PlotOrigin.y = atof(buffer);
				break;
			case 73:
				lo.m_PlotSettings.m_PlotRotation = atoi(buffer);
				break;
			default:
				break;
			}
		}
		while (!m_DxfFile.fail())
		{
			pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 1:
				layoutName = buffer;
				break;
			case 71:
				lo.m_LayoutOrder = atoi(buffer);
				break;
			case 10:
				lo.m_MinLim.x = atof(buffer);
				break;
			case 20:
				lo.m_MinLim.y = atof(buffer);
				break;
			case 11:
				lo.m_MaxLim.x = atof(buffer);
				break;
			case 21:
				lo.m_MaxLim.y = atof(buffer);
				break;
			case 14:
				lo.m_MinExt.x = atof(buffer);
				break;
			case 24:
				lo.m_MinExt.y = atof(buffer);
				break;
			case 15:
				lo.m_MaxExt.x = atof(buffer);
				break;
			case 25:
				lo.m_MaxExt.y = atof(buffer);
				break;
			case 330:
			{
				int handle = hextoi(buffer);
				lo.m_BlockName = LookupBlockEntryName(handle);
			}
			break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadMLeaderStyle(MLeaderStyle & mls)
	{
		int num;
		char buffer[BUFSIZE];
		int pos = m_DxfFile.tellg();
		ContentType ct = MTEXT_TYPE;
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 170:
				if ((ct = ContentType(atoi(buffer))) == BLOCK_TYPE)
					mls.m_Content = make_shared<StyleBlockPart>();
				else
					mls.m_Content = make_shared<StyleMTextPart>();
				break;
			case 90:
				mls.m_MaxLeaderPoints = atoi(buffer);
				break;
			case 40:
				mls.m_FirstSegAngleConstraint = atof(buffer);
				break;
			case 41:
				mls.m_SecondSegAngleConstraint = atof(buffer);
				break;
			case 173:
				mls.m_LeaderType = AcMLeaderType(atoi(buffer));
				break;
			case 91:
				mls.m_LineColor = AcColor(atoi(buffer));
				break;
			case 340:
				mls.m_LineType = LookupLinetypeName(hextoi(buffer));
				break;
			case 92:
				mls.m_LineWeight = AcLineWeight(atoi(buffer));
				break;
			case 290:
				mls.m_EnableLanding = !!atoi(buffer);
				break;
			case 42:
				mls.m_LandingGap = atof(buffer);
				break;
			case 291:
				mls.m_EnableDogleg = !!atoi(buffer);
				break;
			case 43:
				mls.m_DoglegLength = atof(buffer);
				break;
			case 44:
				mls.m_ArrowSize = atof(buffer);
				break;
			case 296:
				mls.m_IsAnnotative = !!atoi(buffer);
				break;
			case 143:
				mls.m_BreakGapSize = atof(buffer);
				break;
			case 341:
				mls.m_ArrowHead = LookupBlockEntryName(hextoi(buffer));
				break;
			default:
				break;
			}
		}

		if (ct == BLOCK_TYPE)
			ReadStyleBlockPart(*static_cast<StyleBlockPart*>(mls.m_Content.get()));
		else
			ReadStyleMTextPart(*static_cast<StyleMTextPart*>(mls.m_Content.get()));
	}

	void DxfReader::ReadStyleBlockPart(StyleBlockPart & block)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 343:
				block.m_BlockName = LookupBlockEntryName(hextoi(buffer));
				break;
			case 177:
				block.m_BlockConnectionType = AcBlockConnectionType(atoi(buffer));
				break;
			case 94:
				block.m_BlockColor = AcColor(atoi(buffer));
				break;
			case 47:
				block.m_BlockScale.x = atof(buffer);
				break;
			case 49:
				block.m_BlockScale.y = atof(buffer);
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadStyleMTextPart(StyleMTextPart & mtext)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 342:
				mtext.m_TextStyle = LookupTextStyleName(hextoi(buffer));
				break;
			case 175:
				mtext.m_TextAngleType = AcTextAngleType(atoi(buffer));
				break;
			case 93:
				mtext.m_TextColor = AcColor(atoi(buffer));
				break;
			case 45:
				mtext.m_TextHeight = atof(buffer);
				break;
			case 46:
				mtext.m_AlignSpace = atof(buffer);
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadTable(EntityList * pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pTable = std::make_shared<AcadTable>();
		SetAttribute(pTable.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 2:
				pTable->m_TableBlockName = buffer;
				break;
			case 10:
				pTable->m_InsertionPoint.x = atof(buffer);
				break;
			case 20:
				pTable->m_InsertionPoint.y = atof(buffer);
				break;
			case 342:
				//此时转换TableStyle handle到name不会成功。
				m_ToBeConvertedTableStyleName[pTable->m_Handle] = hextoi(buffer);
				break;
			case 343:
				break;
			case 91:
				pTable->m_RowCount = atoi(buffer);
				pTable->m_RowHeights.reserve(pTable->m_RowCount);
				break;
			case 92:
				pTable->m_ColCount = atoi(buffer);
				pTable->m_ColWidths.reserve(pTable->m_ColCount);
				if (pTable->m_RowCount > 0 && pTable->m_ColCount > 0)
				{
					pTable->m_Cells.reserve(pTable->m_RowCount * pTable->m_ColCount);
				}
				break;
			case 141:
				pTable->m_RowHeights.push_back(atof(buffer));
				break;
			case 142:
				pTable->m_ColWidths.push_back(atof(buffer));
				break;
			case 171:
			{
				// Suppose cell begins with group code 171.
				pTable->m_Cells.push_back(Cell(CellType(atoi(buffer))));
				Cell& cell = pTable->m_Cells.back();
				ReadCell(cell);
			}
			break;
			default:
				break;
			}
		}
		pObjList->push_back(pTable);
	}

	void DxfReader::ReadCell(Cell & cell)
	{
		int num;
		char buffer[BUFSIZE];
		bool bFirst91 = true;
		bool bBefore301 = true;
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 304)
			{
				// buffer is "ACVALUE_END"
				break;
			}
			switch (num)
			{
			case 172:
				cell.m_Flag = atoi(buffer);
				break;
			case 173:
				cell.m_Merged = !!atoi(buffer);
				break;
			case 174:
				cell.m_AutoFit = !!atoi(buffer);
				break;
			case 175:
				cell.m_ColSpan = atoi(buffer);
				break;
			case 176:
				cell.m_RowSpan = atoi(buffer);
				break;
			case 93:
				cell.m_Flag93 = atoi(buffer);
				break;
			case 90:
				cell.m_Flag90 = atoi(buffer);
				break;
			case 91:
				if (bFirst91)
				{
					cell.m_OverrideFlag1 = atoi(buffer);
					bFirst91 = false;
				}
				else
				{
					cell.m_OverrideFlag2 = atoi(buffer);
				}
				break;
			case 178:
				cell.m_VirtualEdgeFlag = atoi(buffer);
				break;
			case 145:
				cell.m_Rotation = atof(buffer);
				break;
			case 170:
				cell.m_Alignment = AcCellAlignment(atoi(buffer));
				break;
			case 283:
				cell.m_BgColorEnabled = !!atoi(buffer);
				break;
			case 63:
				cell.m_BgColor = AcColor(atoi(buffer));
				break;
			case 64:
				cell.m_TextColor = AcColor(atoi(buffer));
				break;
			case 140:
				cell.m_TextHeight = atof(buffer);
				break;
			case 288:
				cell.m_LeftBorderVisible = !!atoi(buffer);
				break;
			case 285:
				cell.m_RightBorderVisible = !!atoi(buffer);
				break;
			case 289:
				cell.m_TopBorderVisible = !!atoi(buffer);
				break;
			case 286:
				cell.m_BottomBorderVisible = !!atoi(buffer);
				break;
			case 302:
			case 303:
				if (cell.m_Content->GetContentType() == TEXT_CELL)
				{
					CellText* pCell = static_cast<CellText*>(cell.m_Content.get());
					pCell->m_Text.append(buffer);
				}
				break;
			case 7:
				if (cell.m_Content->GetContentType() == TEXT_CELL)
				{
					CellText* pCell = static_cast<CellText*>(cell.m_Content.get());
					pCell->m_TextStyle = buffer;
				}
				break;
			case 340:
				if (cell.m_Content->GetContentType() == BLOCK_CELL)
				{
					CellBlock* pCell = static_cast<CellBlock*>(cell.m_Content.get());
					pCell->m_BlockName = LookupBlockEntryName(hextoi(buffer));
				}
				break;
			case 144:
				if (cell.m_Content->GetContentType() == BLOCK_CELL)
				{
					CellBlock* pCell = static_cast<CellBlock*>(cell.m_Content.get());
					pCell->m_BlockScale = atof(buffer);
				}
				break;
			case 331:
				if (cell.m_Content->GetContentType() == BLOCK_CELL)
				{
					CellBlock* pCell = static_cast<CellBlock*>(cell.m_Content.get());
					auto sp = m_Graph.FindEntity(hextoi(buffer));
					auto ad = dynamic_pointer_cast<AcadAttDef>(sp);
					if (ad)
						pCell->m_AttrDefs.push_back(ad);
					else
						ASSERT_DEBUG_INFO("Invalid ATTDEF handle.");
				}
				break;
			case 301:
				if (strcmp(buffer, "CELL_VALUE") == 0)
					bBefore301 = false;
				break;
			case 300:
				if (bBefore301 && cell.m_Content->GetContentType() == BLOCK_CELL)
				{
					CellBlock* pCell = static_cast<CellBlock*>(cell.m_Content.get());
					pCell->m_AttrValues.push_back(buffer);
				}
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadTableStyle(TableStyle & ts)
	{
		int num;
		char buffer[BUFSIZE];
		int pos = m_DxfFile.tellg();
		bool first280IsRead = false;
		std::map<int, int> cellAttribCount;
		int curCellIndex = -1;
		auto PushCellIfFirst = [&cellAttribCount, &curCellIndex, &ts](int num)
		{
			auto it = cellAttribCount.find(num);
			if (it == cellAttribCount.end())
			{
				cellAttribCount.insert(std::make_pair(num, 0));
				it = cellAttribCount.find(num);
			}
			else
			{
				++it->second;
			}
			if (it->second > curCellIndex)
			{
				ts.m_Cells.push_back(CellInTableStyle());
				curCellIndex = it->second;
			}
		};
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 40:
				ts.m_HorCellMargin = atof(buffer);
				break;
			case 41:
				ts.m_VerCellMargin = atof(buffer);
				break;
			case 280:
				if (!first280IsRead)
				{
					first280IsRead = true;
				}
				else
				{
					ts.m_HasNoTitle = !!atoi(buffer);
				}
				break;
			case 281:
				ts.m_HasNoColumnHeading = !!atoi(buffer);
				break;
			case 7:
				ts.m_TextStyle = buffer;
				break;
			case 140:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_TextHeight = atof(buffer);
				break;
			case 170:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_Alignment = AcCellAlignment(atoi(buffer));
				break;
			case 62:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_TextColor = AcColor(atoi(buffer));
				break;
			case 63:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_BgColor = AcColor(atoi(buffer));
				break;
			case 283:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_BgColorEnabled = !!atoi(buffer);
				break;
			case 90:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_CellDataType = AcValueDataType(atoi(buffer));
				break;
			case 91:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_CellUnitType = AcValueUnitType(atoi(buffer));
				break;
			case 274:
			case 275:
			case 276:
			case 277:
			case 278:
			case 279:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_BorderLineWeight[num - 274] = AcLineWeight(atoi(buffer));
				break;
			case 284:
			case 285:
			case 286:
			case 287:
			case 288:
			case 289:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_BorderVisible[num - 284] = !!atoi(buffer);
				break;
			case 64:
			case 65:
			case 66:
			case 67:
			case 68:
			case 69:
				PushCellIfFirst(num);
				ts.m_Cells.back().m_BorderColor[num - 64] = AcColor(atoi(buffer));
				break;
			default:
				break;
			}
		}
	}

	bool DxfReader::ReadObjects()
	{
		TRACE_FUNCTION("()");

		if (m_OnlyReadModelSpace)
			return true;
		bool bSuccess = AdvanceToSection("OBJECTS");

		// Get handle of dictionary ACAD_MLEADERSTYLE and ACAD_TABLESTYLE
		bSuccess = bSuccess && NextObject("DICTIONARY");
		int mLeaderStyleDicHandle = 0;
		int tableStyleDicHandle = 0;
		ReadDictionaryEntries([&mLeaderStyleDicHandle, &tableStyleDicHandle](const NameHandle& curDicEntry) {
			if (curDicEntry.name == "ACAD_MLEADERSTYLE")
			{
				mLeaderStyleDicHandle = curDicEntry.handle;
				return false; // continue
			}
			else if (curDicEntry.name == "ACAD_TABLESTYLE")
			{
				tableStyleDicHandle = curDicEntry.handle;
				return true; // break
			}
			return false; // continue
		});

		while (bSuccess && NextObject("DICTIONARY"))
		{
			if (m_CurObjectHandle == mLeaderStyleDicHandle)
			{
				ReadDictionaryEntries([this](const NameHandle& curDicEntry) {
					m_MLeaderStyleHandleTable[curDicEntry.handle] = curDicEntry.name;
					return false; // continue
				});
				break;
			}
		}

		// DICTIONARY isn't continuous.
		while (AdvanceToObjectStopAtLayout("DICTIONARY"))
		{
			if (bSuccess && NextObject("DICTIONARY"))
			{
				if (m_CurObjectHandle == tableStyleDicHandle)
				{
					ReadDictionaryEntries([this](const NameHandle& curDicEntry) {
						m_TableStyleHandleTable[curDicEntry.handle] = curDicEntry.name;
						return false; // continue
					});
					break;
				}
			}
		}

		std::string layoutName;
		std::map<std::string, std::shared_ptr<LayoutData>> layouts;
		bSuccess = bSuccess && AdvanceToObject("LAYOUT");
		while (bSuccess && NextObject("LAYOUT"))
		{
			LayoutData lo;
			ReadLayout(layoutName, lo);
			PrintDebugInfo("Read Layout %s", layoutName.c_str());
			if (strnicmp(lo.m_BlockName.c_str(), g_PaperSpace, g_LenOfPaperSpace) == 0)
			{
				auto pLayout = LookupLayout(lo.m_BlockName.c_str());
				if (pLayout)
				{
					pLayout->m_LayoutOrder = lo.m_LayoutOrder;
					pLayout->m_PlotSettings = lo.m_PlotSettings;
					layouts[layoutName] = pLayout;
				}
			}
		}
		// Now Layouts's key is layout name.
		m_Graph.m_Layouts.swap(layouts);

		// MLEADERSTYLE is after LAYOUT
		bSuccess = bSuccess && AdvanceToObject("MLEADERSTYLE");
		while (bSuccess && NextObject("MLEADERSTYLE"))
		{
			MLeaderStyle mls;
			ReadMLeaderStyle(mls);
			string name = LookupMLeaderStyleName(m_CurObjectHandle);
			PrintDebugInfo("Read MLEADERSTYLE %s", name.c_str());
			m_Graph.m_MLeaderStyles[name] = mls;
		}

		// TABLESTYLE is after MLEADERSTYLE
		bSuccess = bSuccess && AdvanceToObject("TABLESTYLE");
		while (bSuccess && NextObject("TABLESTYLE"))
		{
			TableStyle tbls;
			ReadTableStyle(tbls);
			string name = LookupTableStyleName(m_CurObjectHandle);
			PrintDebugInfo("Read TABLESTYLE %s", name.c_str());
			m_Graph.m_TableStyles[name] = tbls;
		}

		return bSuccess;
	}

	string DxfReader::LookupTextStyleName(int handle)
	{
		auto it = m_TextStyleHandleTable.find(handle);
		if (it != m_TextStyleHandleTable.end())
			return it->second;
		PRINT_DEBUG_INFO("Can't convert handle 0x%X to TextStyle name", handle);
		return string();
	}

	string DxfReader::LookupBlockEntryName(int handle)
	{
		auto it = m_BlockEntryHandleTable.find(handle);
		if (it != m_BlockEntryHandleTable.end())
			return it->second;
		PRINT_DEBUG_INFO("Can't convert handle 0x%X to Block name", handle);
		return string();
	}

	std::string DxfReader::LookupMLeaderStyleName(int handle)
	{
		auto it = m_MLeaderStyleHandleTable.find(handle);
		if (it != m_MLeaderStyleHandleTable.end())
			return it->second;
		PRINT_DEBUG_INFO("Can't convert handle 0x%X to MLeaderStyle name", handle);
		return std::string();
	}

	std::string DxfReader::LookupLinetypeName(int handle)
	{
		auto it = m_LinetypeHandleTable.find(handle);
		if (it != m_LinetypeHandleTable.end())
			return it->second;
		PRINT_DEBUG_INFO("Can't convert handle 0x%X to Linetype name", handle);
		return std::string();
	}

	std::string DxfReader::LookupTableStyleName(int handle)
	{
		auto it = m_TableStyleHandleTable.find(handle);
		if (it != m_TableStyleHandleTable.end())
			return it->second;
		PRINT_DEBUG_INFO("Can't convert handle 0x%X to TableStyle name", handle);
		return std::string();
	}

	std::shared_ptr<LayoutData> DxfReader::LookupLayout(const char* blockName)
	{
		for (const auto& lo : m_Graph.m_Layouts)
		{
			if (stricmp(lo.second->m_BlockName.c_str(), blockName) == 0)
				return lo.second;
		}
		PRINT_DEBUG_INFO("Can't find layout which block name is %s", blockName);
		return std::shared_ptr<LayoutData>();
	}

	void DxfReader::ConvertHandle()
	{
		TRACE_FUNCTION("()");

		for (auto pair : m_ToBeConvertedReactors)
		{
			int handle = pair.first;
			auto ent = m_Graph.FindEntity(handle);
			if (ent != nullptr)
			{
				for (auto reactorHandle : pair.second)
				{
					auto reactor = m_Graph.FindEntity(reactorHandle);
					if (reactor != nullptr)
					{
						ent->m_Reactors.push_back(reactor);
					}
					else
					{
						PRINT_DEBUG_INFO("Can't convert handle 0x%X to Reactor", reactorHandle);
					}
				}
			}
		}

		for (auto pair : m_ToBeConvertedViewportClipEnts)
		{
			int handle = pair.first;
			auto vp = std::dynamic_pointer_cast<AcadViewport>(m_Graph.FindEntity(handle));
			if (vp != nullptr)
			{
				int clipEntHandle = pair.second;
				auto clipEnt = m_Graph.FindEntity(clipEntHandle);
				if (clipEnt != nullptr)
				{
					vp->m_ClipEnt = clipEnt;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to Clip entity", clipEntHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedLeaderAnnotation)
		{
			int handle = pair.first;
			auto leader = std::dynamic_pointer_cast<AcadLeader>(m_Graph.FindEntity(handle));
			if (leader != nullptr)
			{
				int annotationHandle = pair.second;
				auto annotation = m_Graph.FindEntity(annotationHandle);
				if (annotation != nullptr)
				{
					leader->m_Annotation = annotation;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to Annotation", annotationHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedMLeaderAttDefs)
		{
			int handle = pair.first;
			auto mleader = std::dynamic_pointer_cast<AcadMLeader>(m_Graph.FindEntity(handle));
			if (mleader != nullptr)
			{
				for (auto attDefHandle : pair.second)
				{
					auto attDef = std::dynamic_pointer_cast<AcadAttDef>(m_Graph.FindEntity(attDefHandle));
					if (attDef != nullptr)
					{
						mleader->m_AttrDefs.push_back(attDef);
					}
					else
					{
						PRINT_DEBUG_INFO("Can't convert handle 0x%X to AttDef", attDef);
					}
				}
			}
		}

		for (auto pair : m_ToBeConvertedMLeaderStyleName)
		{
			int handle = pair.first;
			auto mleader = std::dynamic_pointer_cast<AcadMLeader>(m_Graph.FindEntity(handle));
			if (mleader != nullptr)
			{
				int styleHandle = pair.second;
				string name = LookupMLeaderStyleName(styleHandle);
				if (!name.empty())
				{
					mleader->m_LeaderStyle = name;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to MLeaderStyleName", styleHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedTableStyleName)
		{
			int handle = pair.first;
			auto table = std::dynamic_pointer_cast<AcadTable>(m_Graph.FindEntity(handle));
			if (table != nullptr)
			{
				int styleHandle = pair.second;
				string name = LookupTableStyleName(styleHandle);
				if (!name.empty())
				{
					table->m_TableStyle = name;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to TableStyleName", styleHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedTextStyleName)
		{
			auto it = m_Graph.m_DimStyles.find(pair.first);
			if (it != m_Graph.m_DimStyles.end())
			{
				int styleHandle = pair.second;
				string name = LookupTextStyleName(styleHandle);
				if (!name.empty())
				{
					it->second.TextStyle = name;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to TextStyle", styleHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedArrow1Name)
		{
			auto it = m_Graph.m_DimStyles.find(pair.first);
			if (it != m_Graph.m_DimStyles.end())
			{
				int blockHandle = pair.second;
				string name = LookupBlockEntryName(blockHandle);
				int arrowType = GetArrowHeadType(name.c_str());
				if (!name.empty())
				{
					it->second.ArrowHead1Block = name;
					it->second.ArrowHead1Type = arrowType;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to BlockName", blockHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedArrow2Name)
		{
			auto it = m_Graph.m_DimStyles.find(pair.first);
			if (it != m_Graph.m_DimStyles.end())
			{
				int blockHandle = pair.second;
				string name = LookupBlockEntryName(blockHandle);
				int arrowType = GetArrowHeadType(name.c_str());
				if (!name.empty())
				{
					it->second.ArrowHead2Block = name;
					it->second.ArrowHead2Type = arrowType;
				}
				else
				{
					PRINT_DEBUG_INFO("Can't convert handle 0x%X to BlockName", blockHandle);
				}
			}
		}

		for (auto pair : m_ToBeConvertedHatchAssociatedEnts)
		{
			int handle = pair.first;
			auto hatch = std::dynamic_pointer_cast<AcadHatch>(m_Graph.FindEntity(handle));
			if (hatch != nullptr)
			{
				auto list = pair.second;
				for (auto handleLoopIndex : list)
				{
					auto boundaryEntHandle = handleLoopIndex.first;
					auto loopIndex = handleLoopIndex.second;
					auto boundaryEnt = m_Graph.FindEntity(boundaryEntHandle);
					if (boundaryEnt != nullptr)
					{
						hatch->AddAssociatedEntity(boundaryEnt, loopIndex);
					}
					else
					{
						PRINT_DEBUG_INFO("Source handle 0x%X of loop %d : Hatch is invalid.", handle, loopIndex);
					}
				}
			}
		}
	}

	void DxfReader::ReadAttribute()
	{
		int num;
		char buffer[BUFSIZE];
		// Reset to default value.
		m_Attribute.m_IsInPaperspace = false;
		m_Attribute.m_Layer = "0";
		m_Attribute.m_Color = acByLayer;
		m_Attribute.m_Linetype = "ByLayer";
		m_Attribute.m_LinetypeScale = 1.0;
		m_Attribute.m_Lineweight = acLnWtByLayer;
		m_Attribute.m_ReactorHandles.clear();
		bool bReactor = false;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 100 && stricmp(buffer, "AcDbEntity") != 0
				|| num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 67:
				m_Attribute.m_IsInPaperspace = atoi(buffer) == 1;
				break;
			case 5:
				m_Attribute.m_Handle = hextoi(buffer);
				break;
			case 8:
				m_Attribute.m_Layer = buffer;
				break;
			case 6:
				m_Attribute.m_Linetype = buffer;
				break;
			case 62:
				m_Attribute.m_Color = AcColor(atol(buffer));
				break;
			case 370:
				m_Attribute.m_Lineweight = AcLineWeight(atol(buffer));
				break;
			case 48:
				m_Attribute.m_LinetypeScale = atof(buffer);
				break;
			case 102:
				if (strcmp("{ACAD_REACTORS", buffer) == 0)
					bReactor = true;
				else if (bReactor)
					bReactor = false;
				break;
			case 330:
				if (bReactor)
					m_Attribute.m_ReactorHandles.push_back(hextoi(buffer));
				else
				{
					// int blockEntryHandle = hextoi(buffer);
				}
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::SetAttribute(EntAttribute* pObj)
	{
		pObj->m_Handle = m_Attribute.m_Handle;
		pObj->m_IsInPaperspace = m_Attribute.m_IsInPaperspace;
		pObj->m_Layer = m_Attribute.m_Layer;
		pObj->m_Linetype = m_Attribute.m_Linetype;
		pObj->m_Color = m_Attribute.m_Color;
		pObj->m_Lineweight = m_Attribute.m_Lineweight;
		pObj->m_LinetypeScale = m_Attribute.m_LinetypeScale;
		m_ToBeConvertedReactors[m_Attribute.m_Handle] = m_Attribute.m_ReactorHandles;
	}

	void DxfReader::Reset()
	{
		m_TextStyleHandleTable.clear();
		m_BlockEntryHandleTable.clear();
		m_MLeaderStyleHandleTable.clear();
		m_LinetypeHandleTable.clear();
		m_TableStyleHandleTable.clear();
		//m_ConvertHandlePtrFunctors.clear();
		m_ToBeConvertedReactors.clear();
		m_ToBeConvertedViewportClipEnts.clear();
		m_ToBeConvertedLeaderAnnotation.clear();
		m_ToBeConvertedMLeaderAttDefs.clear();
		m_ToBeConvertedMLeaderStyleName.clear();
		m_ToBeConvertedTableStyleName.clear();
		m_ToBeConvertedTextStyleName.clear();
		m_ToBeConvertedArrow1Name.clear();
		m_ToBeConvertedArrow2Name.clear();
		m_ToBeConvertedHatchAssociatedEnts.clear();
	}

	void DxfReader::ReadArc(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pArc = std::make_shared<AcadArc>();
		SetAttribute(pArc.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pArc->m_Center.x = atof(buffer);
				break;
			case 20:
				pArc->m_Center.y = atof(buffer);
				break;
			case 40:
				pArc->m_Radius = atof(buffer);
				break;
			case 50:
				pArc->m_StartAngle = atof(buffer);
				break;
			case 51:
				pArc->m_EndAngle = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pArc);
	}

	void DxfReader::ReadAttDef(EntityList * pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pAttDef = std::make_shared<AcadAttDef>();
		SetAttribute(pAttDef.get());
		bool bFirst280 = true;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pAttDef->m_BaseLeftPoint.x = atof(buffer);
				break;
			case 20:
				pAttDef->m_BaseLeftPoint.y = atof(buffer);
				break;
			case 11:
				pAttDef->m_InsertionPoint.x = atof(buffer);
				break;
			case 21:
				pAttDef->m_InsertionPoint.y = atof(buffer);
				break;
			case 40:
				pAttDef->m_TextHeight = atof(buffer);
				break;
			case 50:
				pAttDef->m_RotationAngle = atof(buffer);
				break;
			case 7:
				pAttDef->m_TextStyle = buffer;
				break;
			case 72:
				pAttDef->m_HorAlign = atoi(buffer);
				break;
			case 280:
				if (bFirst280)
					bFirst280 = false;
				else
					pAttDef->m_DuplicateFlag = atoi(buffer);
				break;
			case 1:
				pAttDef->m_Text = buffer;
				break;
			case 2:
				pAttDef->m_Tag = buffer;
				break;
			case 3:
				pAttDef->m_Prompt = buffer;
				break;
			case 70:
				pAttDef->m_Flags = AcAttributeMode(atoi(buffer));
				break;
			case 74:
				pAttDef->m_VerAlign = atoi(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pAttDef);
	}

	void DxfReader::ReadAttrib(EntityList * pObjList)
	{
		AcadAttrib att;
		int num;
		char buffer[BUFSIZE];
		SetAttribute(&att);
		bool bFirst280 = true;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				att.m_BaseLeftPoint.x = atof(buffer);
				break;
			case 20:
				att.m_BaseLeftPoint.y = atof(buffer);
				break;
			case 11:
				att.m_InsertionPoint.x = atof(buffer);
				break;
			case 21:
				att.m_InsertionPoint.y = atof(buffer);
				break;
			case 40:
				att.m_TextHeight = atof(buffer);
				break;
			case 50:
				att.m_RotationAngle = atof(buffer);
				break;
			case 7:
				att.m_TextStyle = buffer;
				break;
			case 72:
				att.m_HorAlign = atoi(buffer);
				break;
			case 280:
				if (bFirst280)
					bFirst280 = false;
				else
					att.m_DuplicateFlag = atoi(buffer);
				break;
			case 1:
				att.m_Text = buffer;
				break;
			case 2:
				att.m_Tag = buffer;
				break;
			case 70:
				att.m_Flags = AcAttributeMode(atoi(buffer));
				break;
			case 74:
				att.m_VerAlign = atoi(buffer);
				break;
			default:
				break;
			}
		}

		if (!pObjList->empty())
		{
			AcadBlockInstance* pInsert = dynamic_cast<AcadBlockInstance*>(pObjList->back().get());
			if (pInsert)
			{
				pInsert->m_Attribs.push_back(att);
				return;
			}
		}
		PRINT_DEBUG_INFO("ATTRIB must be after an INSERT.");
	}


	void DxfReader::ReadInsert(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pBlock = std::make_shared<AcadBlockInstance>();
		pBlock->m_Xscale = pBlock->m_Yscale = pBlock->m_Zscale = 1.0;
		SetAttribute(pBlock.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 2:
				pBlock->m_Name = buffer;
				break;
			case 10:
				pBlock->m_InsertionPoint.x = atof(buffer);
				break;
			case 20:
				pBlock->m_InsertionPoint.y = atof(buffer);
				break;
			case 11:
				pBlock->m_InsertionPoint.x = atof(buffer);
				break;
			case 21:
				pBlock->m_InsertionPoint.y = atof(buffer);
				break;
			case 41:
				pBlock->m_Xscale = atof(buffer);
				break;
			case 42:
				pBlock->m_Yscale = atof(buffer);
				break;
			case 50:
				pBlock->m_RotationAngle = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pBlock);
	}


	void DxfReader::ReadCircle(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pCircle = std::make_shared<AcadCircle>();
		SetAttribute(pCircle.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pCircle->m_Center.x = atof(buffer);
				break;
			case 20:
				pCircle->m_Center.y = atof(buffer);
				break;
			case 40:
				pCircle->m_Radius = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pCircle);
	}

	void DxfReader::ReadDimCommon()
	{
		int num;
		char buffer[BUFSIZE];
		int type = 0; //当不存在subtype时，缺省为转角标注
		CDblPoint defPoint;
		CDblPoint textPoint;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 100 && stricmp(buffer, "AcDbDimension") != 0 || num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 2:
				m_Attribute.m_BlockName = buffer;
				break;
			case 10:
				m_Attribute.m_DefPoint.x = atof(buffer);
				break;
			case 20:
				m_Attribute.m_DefPoint.y = atof(buffer);
				break;
			case 11:
				m_Attribute.m_TextPosition.x = atof(buffer);
				m_Attribute.m_ValidMembersFlag[TEXTPOSITION] = true;
				break;
			case 21:
				m_Attribute.m_TextPosition.y = atof(buffer);
				break;
			case 70:
				m_Attribute.m_DimType = atoi(buffer);
				break;
			case 1:
				m_Attribute.OverrideText(buffer);
				break;
			case 3:
				m_Attribute.m_DimStyleName = buffer;
				break;
			case 53:
				m_Attribute.OverrideTextRotation(atof(buffer));
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::SetDimCommon(AcadDim *pDim)
	{
		pDim->m_DimStyleName = m_Attribute.m_DimStyleName;
		pDim->m_BlockName = m_Attribute.m_BlockName;
		pDim->m_DefPoint = m_Attribute.m_DefPoint;
		pDim->m_TextPosition = m_Attribute.m_TextPosition;
		pDim->m_DimType = m_Attribute.m_DimType;
		pDim->m_Attachment = m_Attribute.m_Attachment;
		pDim->m_TextRotation = m_Attribute.m_TextRotation;
		pDim->m_ValidMembersFlag = m_Attribute.m_ValidMembersFlag;
	}

	void DxfReader::ReadDimension(EntityList* pObjList)
	{
		ReadDimCommon();
		int type = m_Attribute.m_DimType & 0xF;
		switch (type)
		{
		case 0:
			ReadDimRot(pObjList);
			break;
		case 1:
			ReadDimAln(pObjList);
			break;
		case 2:
			ReadDimAng(pObjList);
			break;
		case 3:
			ReadDimDia(pObjList);
			break;
		case 4:
			ReadDimRad(pObjList);
			break;
		case 5:
			ReadDimAng3P(pObjList);
			break;
		case 6:
			ReadDimOrd(pObjList);
			break;
		}
	}

	void DxfReader::ReadDimAln(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimAln>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 13:
				pDim->m_ExtLine1Point.x = atof(buffer);
				break;
			case 23:
				pDim->m_ExtLine1Point.y = atof(buffer);
				break;
			case 14:
				pDim->m_ExtLine2Point.x = atof(buffer);
				break;
			case 24:
				pDim->m_ExtLine2Point.y = atof(buffer);
				break;
			case 50:
				pDim->m_RotationAngle = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimOrd(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimOrd>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 13:
				pDim->m_OrdPoint.x = atof(buffer);
				break;
			case 23:
				pDim->m_OrdPoint.y = atof(buffer);
				break;
			case 14:
				pDim->m_LeaderPoint.x = atof(buffer);
				break;
			case 24:
				pDim->m_LeaderPoint.y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimRot(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimRot>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 13:
				pDim->m_ExtLine1Point.x = atof(buffer);
				break;
			case 23:
				pDim->m_ExtLine1Point.y = atof(buffer);
				break;
			case 14:
				pDim->m_ExtLine2Point.x = atof(buffer);
				break;
			case 24:
				pDim->m_ExtLine2Point.y = atof(buffer);
				break;
			case 50:
				pDim->m_RotationAngle = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimDia(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimDia>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 15:
				pDim->m_ChordPoint.x = atof(buffer);
				break;
			case 25:
				pDim->m_ChordPoint.y = atof(buffer);
				break;
			case 40:
				pDim->m_LeaderLength = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimRad(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimRad>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 15:
				pDim->m_ChordPoint.x = atof(buffer);
				break;
			case 25:
				pDim->m_ChordPoint.y = atof(buffer);
				break;
			case 40:
				pDim->m_LeaderLength = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimAng3P(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimAng3P>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 13:
				pDim->m_FirstEnd.x = atof(buffer);
				break;
			case 23:
				pDim->m_FirstEnd.y = atof(buffer);
				break;
			case 14:
				pDim->m_SecondEnd.x = atof(buffer);
				break;
			case 24:
				pDim->m_SecondEnd.y = atof(buffer);
				break;
			case 15:
				pDim->m_AngleVertex.x = atof(buffer);
				break;
			case 25:
				pDim->m_AngleVertex.y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadDimAng(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pDim = std::make_shared<AcadDimAng>();
		SetAttribute(pDim.get());
		SetDimCommon(pDim.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 13:
				pDim->m_FirstStart.x = atof(buffer);
				break;
			case 23:
				pDim->m_FirstStart.y = atof(buffer);
				break;
			case 14:
				pDim->m_FirstEnd.x = atof(buffer);
				break;
			case 24:
				pDim->m_FirstEnd.y = atof(buffer);
				break;
			case 15:
				pDim->m_SecondStart.x = atof(buffer);
				break;
			case 25:
				pDim->m_SecondStart.x = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pDim);
	}

	void DxfReader::ReadEllipse(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pEllipse = std::make_shared<AcadEllipse>();
		SetAttribute(pEllipse.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pEllipse->m_Center.x = atof(buffer);
				break;
			case 20:
				pEllipse->m_Center.y = atof(buffer);
				break;
			case 11:
				pEllipse->m_MajorAxisPoint.x = atof(buffer);
				break;
			case 21:
				pEllipse->m_MajorAxisPoint.y = atof(buffer);
				break;
			case 40:
				pEllipse->m_MinorAxisRatio = atof(buffer);
				break;
			case 41:
				pEllipse->m_StartAngle = atof(buffer);
				break;
			case 42:
				pEllipse->m_EndAngle = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pEllipse);
	}

	void DxfReader::ReadHatch(EntityList* pObjList)
	{
		enum BoundaryEntType { Line = 1, Arc, EllipseArc, Spline };
		int num;
		char buffer[BUFSIZE];
		auto pHatch = std::make_shared<AcadHatch>();
		SetAttribute(pHatch.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 2:
				pHatch->m_PatternName = buffer;
				if (RightCompareNoCase(pHatch->m_PatternName, 3, ",_O") || RightCompareNoCase(pHatch->m_PatternName, 3, ",_I"))
					pHatch->m_PatternName = pHatch->m_PatternName.substr(0, pHatch->m_PatternName.length() - 3);
				break;
			case 70:
				pHatch->m_FillFlag = (AcadHatch::FillFlag)atoi(buffer);
				break;
			case 71:
				// 关联性标志（关联 = 1；无关联 = 0）；
				break;
			case 75:
				pHatch->m_HatchStyle = (AcHatchStyle)atol(buffer);
				break;
			case 76:
				pHatch->m_PatternType = (AcPatternType)atol(buffer);
				break;
			case 52:
				pHatch->m_PatternAngle = atof(buffer);
				break;
			case 41:
				pHatch->m_PatternScale = atof(buffer);
				break;
			case 91:
			{
				// 解析边界路径
				int loopCount = atoi(buffer);
				// loopIndex == -1, outer loop; loopIndex >=0, inner loops
				int loopIndex = -2;
				BoundaryEntType entType;
				BoundaryPathFlag pathFlag;
				CDblPoint point1, point2;
				int entCount;
				while (!m_DxfFile.fail())
				{
					pos = m_DxfFile.tellg();
					ReadPair(num, buffer);
					// Last boundary loop
					if (loopIndex == loopCount - 2)
					{
						if (num == 75 || num == 76 || num == 70 || num == 71 || num == 52 || num == 41 || num == 77 || num == 78)
						{
							// 如果遇到这些属于Hatch但不属于边界路径的code，必须结束边界路径解析。
							m_DxfFile.seekg(pos);
							break;
						}
					}
					if (num == 0)
					{
						// 边界路径解析没能在上面的情形退出，那么至少在遇到0时必须退出。一般不会发生这种情况。
						m_DxfFile.seekg(pos);
						break;
					}
					union
					{
						AcadLWPLine* pLWPLine;
						AcadLine* pLine;
						AcadArc* pArc;
						AcadEllipse* pEllipseArc;
						AcadSpline* pSpline;
					};
					switch (num)
					{
						// 边界路径类型标志（按位编码）：
						// 0 = 默认；1 = 外部；2 = 多段线
						// 4 = 导出；8 = 文本框；16 = 最外层
					case 92:
						++loopIndex;
						pLWPLine = nullptr; // This clears all union members.
						pathFlag = BoundaryPathFlag(atoi(buffer));
						if (loopIndex == -1)
							pHatch->m_OuterFlag = pathFlag;
						else
							pHatch->m_InnerFlags.push_back(pathFlag);
						if (pathFlag & BoundaryPathFlag::LWPline)
						{
							auto spLWPLine = std::make_shared<AcadLWPLine>();
							pHatch->AddEntity(spLWPLine, loopIndex);
							pLWPLine = spLWPLine.get();
						}
						break;
						// 该边界路径中的边数（仅当边界不是多段线时）
						// 多段线顶点数
					case 93:
						entCount = atoi(buffer);
						break;
						// 源边界对象数/样条曲线拟合数据数目
					case 97:
						break;
						// 330 - 源边界对象的参照（多个条目）
					case 330:
					{
						int handle = hextoi(buffer);
						m_ToBeConvertedHatchAssociatedEnts[pHatch->m_Handle].push_back(std::make_pair(handle, loopIndex));
					}
					break;
					// 边类型（当边界不是多段线时）：
					// 1 = 直线；2 = 圆弧；3 = 椭圆弧；4 = 样条曲线
					// “有凸度”标志（当边界是多段线时）
					case 72:
						if ((pathFlag & BoundaryPathFlag::LWPline) == 0)
						{
							entType = BoundaryEntType(atoi(buffer));
							--entCount;
							switch (entType)
							{
							case Line:
							{
								auto spLine = std::make_shared<AcadLine>();
								pHatch->AddEntity(spLine, loopIndex);
								pLine = spLine.get();
							}
							break;
							case Arc:
							{
								auto spArc = std::make_shared<AcadArc>();
								pHatch->AddEntity(spArc, loopIndex);
								pArc = spArc.get();
							}
							break;
							case EllipseArc:
							{
								auto spEllipse = std::make_shared<AcadEllipse>();
								pHatch->AddEntity(spEllipse, loopIndex);
								pEllipseArc = spEllipse.get();
							}
							break;
							case Spline:
							{
								auto spSpline = std::make_shared<AcadSpline>();
								pHatch->AddEntity(spSpline, loopIndex);
								pSpline = spSpline.get();
							}
							break;
							}
						}
						break;
					case 73:
						if (pathFlag & BoundaryPathFlag::LWPline)
						{
							pLWPLine->m_Closed = atoi(buffer) != 0;
						}
						else if (entType == BoundaryEntType::Spline)
						{
							int value = atoi(buffer);
							if (value)
								pSpline->m_Flag |= 4;
						}
						break;
					case 74:
						if (entType == BoundaryEntType::Spline)
						{
							int value = atoi(buffer);
							if (value)
								pSpline->m_Flag |= 4;
						}
						break;
					case 95:
						break;
					case 96:
						break;
						// 多段线顶点/线段起点/[椭]圆弧中心/样条曲线控制点（在 OCS 中）
					case 10:
						point1.x = atof(buffer);
						break;
					case 20:
						point1.y = atof(buffer);
						if (pathFlag & BoundaryPathFlag::LWPline)
						{
							pLWPLine->m_Vertices.push_back(point1);
						}
						else if (entType == BoundaryEntType::Line)
						{
							pLine->m_StartPoint = point1;
						}
						else if (entType == BoundaryEntType::Arc)
						{
							pArc->m_Center = point1;
						}
						else if (entType == BoundaryEntType::EllipseArc)
						{
							pEllipseArc->m_Center = point1;
						}
						else if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_ControlPoints.push_back(point1);
						}
						break;
						// 线段终点/椭圆相对于中心点的长轴端点/样条线拟合点（在 OCS 中）
					case 11:
						point2.x = atof(buffer);
						break;
					case 21:
						point2.y = atof(buffer);
						if (entType == BoundaryEntType::Line)
						{
							pLine->m_EndPoint = point2;
						}
						else if (entType == BoundaryEntType::EllipseArc)
						{
							pEllipseArc->m_MajorAxisPoint = point2;
						}
						else if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_FitPoints.push_back(point2);
						}
						break;
					case 12:
						if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_StartTangent.x = atof(buffer);
						}
						break;
					case 22:
						if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_StartTangent.y = atof(buffer);
						}
						break;
					case 13:
						if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_EndTangent.x = atof(buffer);
						}
						break;
					case 23:
						if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_EndTangent.y = atof(buffer);
						}
						break;
						// 多段线凸度（可选；默认值 = 0）
					case 42:
						if (pathFlag & BoundaryPathFlag::LWPline)
						{
							pLWPLine->SetBulge(pLWPLine->m_Vertices.size() - 1, atof(buffer));
						}
						break;
						// 圆的半径/短轴的长度（占长轴长度的比例）/节点值（多个条目）
					case 40:
						if (entType == BoundaryEntType::Arc)
						{
							pArc->m_Radius = atof(buffer);
						}
						else if (entType == BoundaryEntType::EllipseArc)
						{
							pEllipseArc->m_MinorAxisRatio = atof(buffer);
						}
						else if (entType == BoundaryEntType::Spline)
						{
							pSpline->m_Knots.push_back(atof(buffer));
						}
						break;
						// 起点角度
					case 50:
						if (entType == BoundaryEntType::Arc)
						{
							pArc->m_StartAngle = atof(buffer);
						}
						else if (entType == BoundaryEntType::EllipseArc)
						{
							pEllipseArc->m_StartAngle = atof(buffer);
						}
						break;
						// 终点角度
					case 51:
						if (entType == BoundaryEntType::Arc)
						{
							pArc->m_EndAngle = atof(buffer);
						}
						else if (entType == BoundaryEntType::EllipseArc)
						{
							pEllipseArc->m_EndAngle = atof(buffer);
						}
						break;
					}
				}
			}
			break;
			case 47:
				pHatch->m_PixelSize = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pHatch);
	}


	void DxfReader::ReadLeader(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pLeader = std::make_shared<AcadLeader>();
		SetAttribute(pLeader.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 3:
				pLeader->m_StyleName = buffer;
				break;
			case 71:
				pLeader->m_Type = AcLeaderType(pLeader->m_Type & (atoi(buffer) << 1));
				break;
			case 72:
				pLeader->m_Type = AcLeaderType(pLeader->m_Type & atoi(buffer));
				break;
			case 10:
				pLeader->m_Vertices.push_back(CDblPoint(atof(buffer), 0));
				break;
			case 20:
				pLeader->m_Vertices[pLeader->m_Vertices.size() - 1].y = atof(buffer);
				break;
			case 40:
				pLeader->m_CharHeight = atof(buffer);
				break;
			case 41:
				pLeader->m_TextWidth = atof(buffer);
				break;
			case 340:
			{
				int handle = hextoi(buffer);
				m_ToBeConvertedLeaderAnnotation[pLeader->m_Handle] = handle;
			}
			break;
			case 1040:
				pLeader->m_ArrowSize = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pLeader);
	}


	void DxfReader::ReadMLeader(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pLeader = std::make_shared<AcadMLeader>();
		SetAttribute(pLeader.get());
		ReadContextData(pLeader->m_ContextData);
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 340:
				//此时转换MLeaderStyle handle到name不会成功。
				m_ToBeConvertedMLeaderStyleName[pLeader->m_Handle] = hextoi(buffer);
				break;
			case 170:
				pLeader->m_LeaderType = AcMLeaderType(atoi(buffer));
				break;
			case 341:
				pLeader->m_LineType = LookupLinetypeName(hextoi(buffer));
				break;
			case 171:
				pLeader->m_LineWeight = AcLineWeight(atoi(buffer));
				break;
			case 290:
				pLeader->m_EnableLanding = !!atoi(buffer);
				break;
			case 291:
				pLeader->m_EnableDogleg = !!atoi(buffer);
				break;
				//case 41:
				//	pLeader->m_DoglegLength = atof(buffer);
				//	break;
				//case 42:
				//	pLeader->m_ArrowSize = atof(buffer);
				//	break;
				//case 343:
				//	pLeader->m_TextStyle = LookupTextStyleName(hextoi(buffer));
				//	break;
			case 330:
				m_ToBeConvertedMLeaderAttDefs[pLeader->m_Handle].push_back(hextoi(buffer));
				break;
			case 302:
				pLeader->m_AttrValues.push_back(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pLeader);
	}

	void DxfReader::ReadContextData(CONTEXT_DATA & cd)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			// Begin of CONTEXT_DATA
			if (num == 300)
				break;
		}
		ContentType ct = GetContentData(cd);
		if (ct == BLOCK_TYPE)
		{
			cd.m_Content = std::make_shared<BlockPart>();
			ReadBlockPart(*static_cast<BlockPart*>(cd.m_Content.get()));
		}
		else
		{
			cd.m_Content = std::make_shared<MTextPart>();
			ReadMTextPart(*static_cast<MTextPart*>(cd.m_Content.get()));
		}
		ReadLeaderPart(cd.m_Leader);
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			// End of CONTEXT_DATA
			if (num == 301)
				break;
		}
	}

	void DxfReader::ReadLeaderPart(LeaderPart & leader)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 303)
				break;
			if (num == 304)
			{
				leader.m_Lines.resize(leader.m_Lines.size() + 1);
				ReadLeaderLine(leader.m_Lines.back());
			}
			switch (num)
			{
			case 10:
				leader.m_CommonPoint.x = atof(buffer);
				break;
			case 20:
				leader.m_CommonPoint.y = atof(buffer);
				break;
			case 40:
				leader.m_DoglegLength = atof(buffer);
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadLeaderLine(LeaderLine & leaderLine)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			if (num == 305)
				break;
			switch (num)
			{
			case 10:
				leaderLine.m_Points.resize(leaderLine.m_Points.size() + 1);
				leaderLine.m_Points.back().x = atof(buffer);
				break;
			case 20:
				leaderLine.m_Points.back().y = atof(buffer);
				break;
			default:
				break;
			}
		}
	}

	ContentType DxfReader::GetContentData(CONTEXT_DATA & cd)
	{
		int pos = m_DxfFile.tellg();
		int num;
		char buffer[BUFSIZE];
		ContentType ct = MTEXT_TYPE;
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			// Start of LEADER
			if (num == 302)
				break;
			switch (num)
			{
			case 10:
				cd.m_LandingPosition.x = atof(buffer);
				break;
			case 20:
				cd.m_LandingPosition.y = atof(buffer);
				break;
			case 41:
				cd.m_TextHeight = atof(buffer);
				break;
			case 140:
				cd.m_ArrowSize = atof(buffer);
				break;
			case 145:
				cd.m_LandingGap = atof(buffer);
				break;
			case 290:
				if (atoi(buffer))
					ct = MTEXT_TYPE;
				break;
			case 296:
				if (atoi(buffer))
					ct = BLOCK_TYPE;
				break;
			default:
				break;
			}
		}

		m_DxfFile.seekg(pos);
		return ct;
	}

	void DxfReader::ReadBlockPart(BlockPart & block)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			// Start of LEADER
			if (num == 302)
				break;
			switch (num)
			{
			case 341:
				block.m_BlockName = LookupBlockEntryName(hextoi(buffer));
				break;
			case 93:
				block.m_BlockColor = AcColor(atoi(buffer));
				break;
			case 15:
				block.m_BlockPosition.x = atof(buffer);
				break;
			case 25:
				block.m_BlockPosition.y = atof(buffer);
				break;
			case 16:
				block.m_BlockScale.x = atof(buffer);
				break;
			case 26:
				block.m_BlockScale.y = atof(buffer);
				break;
			case 46:
				block.m_BlockRotation = atof(buffer);
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadMTextPart(MTextPart & mtext)
	{
		int num;
		char buffer[BUFSIZE];
		while (!m_DxfFile.fail())
		{
			ReadPair(num, buffer);
			// Start of LEADER
			if (num == 302)
				break;
			switch (num)
			{
			case 304:
				mtext.m_Text = buffer;
				break;
			case 43:
				mtext.m_TextWidth = atof(buffer);
				break;
			case 340:
				mtext.m_TextStyle = LookupTextStyleName(hextoi(buffer));
				break;
			case 90:
				mtext.m_TextColor = AcColor(atoi(buffer));
				break;
			case 12:
				mtext.m_TextLocation.x = atof(buffer);
				break;
			case 22:
				mtext.m_TextLocation.y = atof(buffer);
				break;
			case 42:
				mtext.m_TextRotation = atof(buffer);
				break;
			case 171:
				mtext.m_AttachmentPoint = AcAttachmentPoint(atoi(buffer));
				break;
			default:
				break;
			}
		}
	}

	void DxfReader::ReadLine(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pLine = std::make_shared<AcadLine>();
		SetAttribute(pLine.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pLine->m_StartPoint.x = atof(buffer);
				break;
			case 20:
				pLine->m_StartPoint.y = atof(buffer);
				break;
			case 11:
				pLine->m_EndPoint.x = atof(buffer);
				break;
			case 21:
				pLine->m_EndPoint.y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pLine);
	}


	void DxfReader::ReadLwpolyline(EntityList* pObjList)
	{
		int num;
		double temp;
		char buffer[BUFSIZE];
		auto pLine = std::make_shared<AcadLWPLine>();
		SetAttribute(pLine.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 70:
				pLine->m_Closed = atoi(buffer) & 1;
				break;
			case 43:
				pLine->SetConstWidth(atof(buffer));
				break;
			case 10:
				temp = atof(buffer);
				pLine->m_Vertices.push_back(CDblPoint(temp, 0));
				break;
			case 20:
				pLine->m_Vertices[pLine->m_Vertices.size() - 1].y = atof(buffer);
				break;
			case 40:
				pLine->SetStartWidth(pLine->m_Vertices.size() - 1, atof(buffer));
				break;
			case 41:
				pLine->SetEndWidth(pLine->m_Vertices.size() - 1, atof(buffer));
				break;
			case 42:
				pLine->SetBulge(pLine->m_Vertices.size() - 1, atof(buffer));
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pLine);
	}


	void DxfReader::ReadMtext(EntityList* pObjList)
	{
		int num;
		double cosRot, sinRot;
		char buffer[BUFSIZE];
		auto pMText = std::make_shared<AcadMText>();
		SetAttribute(pMText.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 1:
			case 3:
				pMText->m_Text += buffer;
				break;
			case 7:
				pMText->m_StyleName = buffer;
				break;
			case 10:
				pMText->m_InsertionPoint.x = atof(buffer);
				break;
			case 20:
				pMText->m_InsertionPoint.y = atof(buffer);
				break;
			case 11:
				cosRot = atof(buffer);
				break;
			case 21:
				sinRot = atof(buffer);
				pMText->m_RotationAngle = atan2(sinRot, cosRot) * 180.0 / M_PI;
				break;
			case 40:
				pMText->m_CharHeight = atof(buffer);
				break;
			case 41:
				pMText->m_Width = atof(buffer);
				break;
			case 44:
				pMText->m_LineSpacingFactor = atof(buffer);
				break;
			case 50:
				pMText->m_RotationAngle = atof(buffer);
				break;
			case 71:
				pMText->m_AttachmentPoint = (AcAttachmentPoint)atol(buffer);
				break;
			case 72:
				pMText->m_DrawingDirection = (AcDrawingDirection)atoi(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pMText);
	}


	void DxfReader::ReadPoint(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pPoint = std::make_shared<AcadPoint>();
		SetAttribute(pPoint.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pPoint->m_Point.x = atof(buffer);
				break;
			case 20:
				pPoint->m_Point.y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pPoint);
	}


	void DxfReader::ReadSolid(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pSolid = std::make_shared<AcadSolid>();
		SetAttribute(pSolid.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 10:
				pSolid->m_Point1.x = atof(buffer);
				break;
			case 20:
				pSolid->m_Point1.y = atof(buffer);
				break;
			case 11:
				pSolid->m_Point2.x = atof(buffer);
				break;
			case 21:
				pSolid->m_Point2.y = atof(buffer);
				break;
			case 12:
				pSolid->m_Point3.x = atof(buffer);
				break;
			case 22:
				pSolid->m_Point3.y = atof(buffer);
				break;
			case 13:
				pSolid->m_Point4.x = atof(buffer);
				break;
			case 23:
				pSolid->m_Point4.y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pSolid);
	}


	void DxfReader::ReadSpline(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		int numberOfKnots;
		int numberOfControlPoints;
		int numberOfFitPoints;
		auto pSpline = std::make_shared<AcadSpline>();
		SetAttribute(pSpline.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 70:
				pSpline->m_Flag = atoi(buffer);
				break;
			case 71:
				pSpline->m_Degree = atoi(buffer);
				break;
			case 72:
				numberOfKnots = atoi(buffer);
				break;
			case 73:
				numberOfControlPoints = atoi(buffer);
				break;
			case 74:
				numberOfFitPoints = atoi(buffer);
				break;
			case 12:
				pSpline->m_StartTangent.x = atof(buffer);
				break;
			case 22:
				pSpline->m_StartTangent.y = atof(buffer);
				break;
			case 13:
				pSpline->m_EndTangent.x = atof(buffer);
				break;
			case 23:
				pSpline->m_EndTangent.y = atof(buffer);
				break;
			case 40:
				pSpline->m_Knots.push_back(atof(buffer));
				break;
			case 41:
				pSpline->m_Weights.push_back(atof(buffer));
				break;
			case 10:
				pSpline->m_ControlPoints.push_back(CDblPoint(atof(buffer), 0));
				break;
			case 20:
				pSpline->m_ControlPoints[pSpline->m_ControlPoints.size() - 1].y = atof(buffer);
				break;
			case 11:
				pSpline->m_FitPoints.push_back(CDblPoint(atof(buffer), 0));
				break;
			case 21:
				pSpline->m_FitPoints[pSpline->m_FitPoints.size() - 1].y = atof(buffer);
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pSpline);
	}


	void DxfReader::ReadText(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto pText = std::make_shared<AcadText>();
		SetAttribute(pText.get());
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 1:
				pText->m_Text = buffer;
				break;
			case 7:
				pText->m_StyleName = buffer;
				break;
			case 10:
				pText->m_BaseLeftPoint.x = atof(buffer);
				break;
			case 20:
				pText->m_BaseLeftPoint.y = atof(buffer);
				break;
			case 11:
				pText->m_InsertionPoint.x = atof(buffer);
				break;
			case 21:
				pText->m_InsertionPoint.y = atof(buffer);
				break;
			case 40:
				pText->m_Height = atof(buffer);
				break;
			case 41:
				pText->m_WidthFactor = atof(buffer);
				break;
			case 50:
				pText->m_RotationAngle = atof(buffer);
				break;
			case 51:
				pText->SetObliqueAngle(atof(buffer));
				break;
			case 72:
				pText->m_HorAlign = atol(buffer) & 0x7;
				break;
			case 73:
				pText->m_VerAlign = (atol(buffer) & 0x3) << 16;
				break;
			default:
				break;
			}
		}
		pObjList->push_back(pText);
	}


	void DxfReader::ReadViewport(EntityList* pObjList)
	{
		int num;
		char buffer[BUFSIZE];
		auto vp = std::make_shared<AcadViewport>();
		SetAttribute(vp.get());
		bool HasClipEnt = false;
		int clipEntHandle = 0;
		int viewPortID = 0;
		while (!m_DxfFile.fail())
		{
			int pos = m_DxfFile.tellg();
			ReadPair(num, buffer);
			if (num == 0)
			{
				m_DxfFile.seekg(pos);
				break;
			}
			switch (num)
			{
			case 69:
				viewPortID = atoi(buffer);
				break;
			case 10:
				vp->m_PaperspaceCenter.x = atof(buffer);
				break;
			case 20:
				vp->m_PaperspaceCenter.y = atof(buffer);
				break;
			case 40:
				vp->m_PaperspaceWidth = atof(buffer);
				break;
			case 41:
				vp->m_PaperspaceHeight = atof(buffer);
				break;
			case 12:
				vp->m_ModelSpaceCenter.x = atof(buffer);
				break;
			case 22:
				vp->m_ModelSpaceCenter.y = atof(buffer);
				break;
			case 45:
				vp->m_ModelSpaceHeight = atof(buffer);
				break;
			case 51:
				vp->m_TwistAngle = atof(buffer);
				break;
			case 90:
			{
				int flag = atoi(buffer);
				HasClipEnt = (flag & 0x10000) != 0;
				vp->m_locked = (flag & 0x4000) != 0;
			}
			break;
			case 340:
			{
				int handle = hextoi(buffer);
				m_ToBeConvertedViewportClipEnts[vp->m_Handle] = handle;
			}
			break;
			default:
				break;
			}
		}
		pObjList->push_back(vp);
	}

}
