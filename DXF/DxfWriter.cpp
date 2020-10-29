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

#include <time.h>
#include <algorithm>
#include <string>

#include "utility.h"
#include "ACADConst.h"
#include "AcadEntities.h"
#include "DxfData.h"
#include "DxfWriter.h"
using namespace std;

#pragma warning(disable:4996)

namespace DXF {

	VectorOfEntityList DxfWriter::s_PredefinedBlocks;
	void InitPredefinedBlocks(VectorOfEntityList& g_PredefinedBlocks);

	DxfWriter::DxfWriter(DxfData& graph)
		: m_Graph(graph)
		, m_Handle(0x2FF)
		, m_LayoutIndex(0)
	{
	}

	void DxfWriter::WriteDxf(const char* dxfFile, const char* linFile, const char* patFile, const CDblPoint& center, double height)
	{
		srand((unsigned)time(NULL));

		try
		{
			m_LinetypeManager.SetLinFile(linFile);
			if (!m_LinetypeManager.IsValid())
			{
				return;
			}
			m_PatternManager.SetPatFile(patFile);
			m_DxfFile.open(dxfFile);
			if (!m_PatternManager.IsValid())
			{
				return;
			}
			if (!m_DxfFile.is_open())
			{
				PRINT_DEBUG_INFO("Can't open file %s，Maybe because it is opened by AutoCAD.", dxfFile);
				return;
			}

			// Clear writing state
			Reset();
			InitPredefinedBlocks(s_PredefinedBlocks);
			int handle = m_Graph.PrepareBeforeWrite();
			PrepareBeforeWrite(handle);

			// Header
			HeaderSection();

			// Section TABLES
			BeginSection("TABLES");
			{
				VPortTable(center, height);

				BeginLTypeTable(3 + int(m_Graph.m_Linetypes.size()));
				LTypeEntry_ByBlock_ByLayer_Continuous();
				for (const std::string& linetype : m_Graph.m_Linetypes)
				{
					LTypeEntry(linetype.c_str(), ++m_Handle);
				}
				EndTable();

				BeginLayerTable(int(m_Graph.m_Layers.size()));
				for (const auto& layer : m_Graph.m_Layers)
				{
					LayerEntry(layer, ++m_Handle);
				}
				EndTable();

				BeginStyleTable(int(m_Graph.m_TextStyles.size()));
				for (const auto& textStyle : m_Graph.m_TextStyles)
				{
					StyleEntry(textStyle, ++m_Handle);
				}
				EndTable();

				ViewTable();
				UCSTable();
				AppIdTable();

				int predefinedBlockCount = 0;
				for (int i = 0; i < 20; ++i)
				{
					const char* blockName = GetPredefinedArrowHeadBlockName(i);
					// If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
					if (m_Graph.m_RealBlockDefs.find(blockName) != m_Graph.m_RealBlockDefs.end())
						continue;
					++predefinedBlockCount;
				}
				BeginBlock_RecordTable(1 + int(m_Graph.m_Layouts.size() + m_Graph.m_RealBlockDefs.size())
					+ predefinedBlockCount);
				BlockRecordEntry_ModelSpace();
				for (const auto& pair : m_Graph.m_Layouts)
				{
					BlockRecordEntry(pair.second->m_BlockName.c_str(), ++m_Handle, ++m_Handle);
				}
				for (const auto & pair : m_Graph.m_RealBlockDefs)
				{
					BlockRecordEntry(pair.first.c_str(), ++m_Handle, 0);
				}
				for (int i = 0; i < 20; ++i)
				{
					const char* blockName = GetPredefinedArrowHeadBlockName(i);
					// If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
					if (m_Graph.m_RealBlockDefs.find(blockName) != m_Graph.m_RealBlockDefs.end())
						continue;
					BlockRecordEntry(blockName, ++m_Handle, 0);
				}
				EndTable();

				BeginDimStyleTable(int(m_Graph.m_DimStyles.size()));
				for (const auto& dimStyle : m_Graph.m_DimStyles)
				{
					DimStyleEntry(dimStyle, ++m_Handle);
				}
				EndTable();
			}
			EndSection();

			BeginSection("BLOCKS");
			{
				Block_ModelSpace();
				for (const auto& pair : m_Graph.m_Layouts)
				{
					const auto& pLayout = pair.second;
					BeginBlock(pLayout->m_BlockName.c_str(), 0, 0, ++m_Handle);
					// *Paper_Space 块定义内不允许有图元
					if (pLayout->m_BlockName != "*Paper_Space")
					{
						// viewport need write ID which starts from 1
						m_CurrentViewportID = 0;
						m_CurrentBlockEntryHandle = LookupBlockEntryHandle(pLayout->m_BlockName);
						pLayout->WriteDxf(*this);
					}
					EndBlock(pLayout->m_BlockName.c_str(), ++m_Handle);
				}
				for (const auto& pair : m_Graph.m_RealBlockDefs)
				{
					m_CurrentBlockEntryHandle = LookupBlockEntryHandle(pair.first);
					Block(pair, false);
				}
				for (int i = 0; i < int(s_PredefinedBlocks.size()); ++i)
				{
					const char* blockName = GetPredefinedArrowHeadBlockName(i);
					// If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
					if (m_Graph.m_RealBlockDefs.find(blockName) != m_Graph.m_RealBlockDefs.end())
						continue;
					BeginBlock(blockName, 0, 0, ++m_Handle);
					for (const auto& pEnt : *s_PredefinedBlocks[i])
					{
						m_CurrentBlockEntryHandle = LookupBlockEntryHandle(blockName);
						pEnt->WriteDxf(*this, false);
					}
					EndBlock(blockName, ++m_Handle);
				}
			}
			EndSection();

			BeginSection("ENTITIES");
			{
				m_CurrentBlockEntryHandle = LookupBlockEntryHandle("*Model_Space");
				for (const auto& pEnt : m_Graph.m_Objects)
				{
					pEnt->WriteDxf(*this, false);
				}
				// *Paper_Space 块定义内不允许有图元
				// *Paper_Space 块中的图元定义在ENTITIES段
				for (const auto& pair : m_Graph.m_Layouts)
				{
					if (pair.second->m_BlockName == "*Paper_Space")
					{
						// viewport need write ID which starts from 1
						m_CurrentViewportID = 0;
						m_CurrentBlockEntryHandle = LookupBlockEntryHandle("*Paper_Space");
						pair.second->WriteDxf(*this);
					}
				}
			}
			EndSection();

			BeginSection("OBJECTS");
			{
				Dictionaries();

				// Write Layouts
				std::pair<std::string, std::shared_ptr<LayoutData>> modelLayout = { "Model", std::make_shared<LayoutData>() };
				modelLayout.second->m_BlockName = "*Model_Space";
				Layout(modelLayout);
				for (const auto& pair : m_Graph.m_Layouts)
				{
					Layout(pair);
				}

				// Write MLEADERSTYLE
				for (auto& ls : m_Graph.m_MLeaderStyles)
				{
					ls.second.WriteDxf(*this, ls.first, m_MLeaderStyleHandleTable[ls.first]);
				}

				// Write TABLESTYLE
				for (auto& ts : m_Graph.m_TableStyles)
				{
					ts.second.WriteDxf(*this, ts.first, m_TableStyleHandleTable[ts.first]);
				}
			}
			EndSection();

			dxfEOF();

			m_DxfFile.close();
		}
		catch (const std::logic_error&)
		{
		}
		catch (const std::runtime_error&)
		{
		}
	}

	void DxfWriter::Reset()
	{
		m_Handle = 0x2FF;
		m_LayoutIndex = 0;
		m_TextStyleHandleTable.clear();
		m_DimStyleHandleTable.clear();
		m_BlockEntryHandleTable.clear();
		m_LayoutHandleTable.clear();
		m_MLeaderStyleHandleTable.clear();
		m_TableStyleHandleTable.clear();
		m_LinetypeHandleTable.clear();
		m_MLeaderStyleEntities.clear();
		m_TableStyleEntities.clear();
	}

	/**
	* Writes a real (double) variable to the DXF file.
	*
	* @param gc Group code.
	* @param value Double value
	*/
	void DxfWriter::dxfReal(int gc, double value)
	{
		char str[512];
		sprintf_s(str, sizeof(str), "%f", value);
		char* pEnd = strchr(str, '.');
		pEnd += 2;
		char* pStart = pEnd + 4;
		for (; pStart >= pEnd; --pStart)
		{
			if (*pStart == '0')
				*pStart = '\0';
			else
				break;
		}
		dxfString(gc, str);
	}

	/**
	* Writes an int variable to the DXF file.
	*
	* @param gc Group code.
	* @param value Int value
	*/
	void DxfWriter::dxfInt(int gc, int value)
	{
		m_DxfFile << gc << "\n" << value << "\n";
	}

	/**
	* Writes a hex int variable to the DXF file.
	*
	* @param gc Group code.
	* @param value Int value
	*/
	void DxfWriter::dxfHex(int gc, int value)
	{
		char str[12];
		sprintf(str, "%0X", value);
		dxfString(gc, str);
	}

	/**
	* Writes a string variable to the DXF file.
	*
	* @param gc Group code.
	* @param value String
	*/
	void DxfWriter::dxfString(int gc, const char* value)
	{
		//ASSERT_DEBUG_INFO(value);
		m_DxfFile << gc << "\n" << value << "\n";
	}

	void DxfWriter::dxfString(int gc, const char * value, size_t count)
	{
		m_DxfFile << gc << "\n";
		m_DxfFile.write(value, count);
		m_DxfFile << "\n";
	}

	void DxfWriter::dxfText(int gc, int gcLast, const std::string & text)
	{
		size_t len = text.length();
		const char* pStart = text.data();
		const char* pEnd = pStart + len;
		const char* pCur = pStart;
		while (pCur <= pEnd)
		{
			pCur = AdvanceUtf8(pStart, pEnd, 250);
			if (pCur == pEnd)
			{
				dxfString(gcLast, pStart, pCur - pStart);
				break;
			}
			dxfString(gc, pStart, pCur - pStart);
			pStart = pCur;
		}
	}

	int DxfWriter::PrepareBeforeWrite(int handle)
	{
		for (const auto& msi : m_Graph.m_MLeaderStyles)
		{
			m_MLeaderStyleHandleTable[msi.first] = ++handle;
		}
		for (const auto& tsi : m_Graph.m_TableStyles)
		{
			m_TableStyleHandleTable[tsi.first] = ++handle;
		}
		return handle;
	}

	int DxfWriter::LookupTextStyleHandle(const std::string& name)
	{
		auto it = m_TextStyleHandleTable.find(name);
		if (it != m_TextStyleHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupDimStyleHandle(const std::string& name)
	{
		auto it = m_DimStyleHandleTable.find(name);
		if (it != m_DimStyleHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupBlockEntryHandle(const std::string& blockName)
	{
		auto it = m_BlockEntryHandleTable.find(blockName);
		if (it != m_BlockEntryHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupLayoutHandle(const std::string& blockName)
	{
		auto it = m_LayoutHandleTable.find(blockName);
		if (it != m_LayoutHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupMLeaderStyleHandle(const std::string & name)
	{
		auto it = m_MLeaderStyleHandleTable.find(name);
		if (it != m_MLeaderStyleHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupTableStyleHandle(const std::string & name)
	{
		auto it = m_TableStyleHandleTable.find(name);
		if (it != m_TableStyleHandleTable.end())
			return it->second;
		return 0;
	}

	int DxfWriter::LookupLinetypeHandle(const std::string & name)
	{
		auto it = m_LinetypeHandleTable.find(name);
		if (it != m_LinetypeHandleTable.end())
			return it->second;
		return 0;
	}

	void DxfWriter::AddMLeaderToItsStyle(const std::string & mleaderStyle, const EntAttribute * pMLeader)
	{
		if (!mleaderStyle.empty())
		{
			m_MLeaderStyleEntities[mleaderStyle].push_back(pMLeader);
		}
	}

	const vector<const EntAttribute*>& DxfWriter::GetMLeaders(const std::string & mleaderStyle)
	{
		ASSERT_DEBUG_INFO(!mleaderStyle.empty());
		return m_MLeaderStyleEntities[mleaderStyle];
	}

	void DxfWriter::AddTableToItsStyle(const std::string & tableStyle, const EntAttribute * pTable)
	{
		if (!tableStyle.empty())
		{
			m_TableStyleEntities[tableStyle].push_back(pTable);
		}
	}

	const vector<const EntAttribute*>& DxfWriter::GetTables(const std::string & tableStyle)
	{
		ASSERT_DEBUG_INFO(!tableStyle.empty());
		return m_TableStyleEntities[tableStyle];
	}

	void DxfWriter::HeaderSection()
	{
		ASSERT_DEBUG_INFO(m_Graph.m_LinetypeScale > 0.0);
		BeginSection("HEADER");
		dxfString(9, "$ACADVER");
		dxfString(1, "AC1024"); // AutoCAD 2010, DXF is UTF-8 file.
		dxfString(9, "$DWGCODEPAGE");
		dxfString(3, "ANSI_936");
		//创建无关联标注对象；
		//标注的各种元素组成单一对象，如果对象上的定义点移动，标注值将被更新
		dxfString(9, "$DIMASSOC");
		dxfInt(280, 1);
		dxfString(9, "$LTSCALE");
		dxfReal(40, m_Graph.m_LinetypeScale);
		dxfString(9, "$PSLTSCALE");
		dxfInt(70, 0);
		dxfString(9, "$MEASUREMENT");
		dxfInt(70, 1);//使用公制而不是英制
		dxfString(9, "$HANDSEED");
		dxfHex(5, 0xFFFFF);
		EndSection();
	}

	void DxfWriter::BeginTable(const char* name, int num, int handle)
	{
		dxfString(0, "TABLE");
		dxfString(2, name);
		dxfHex(5, handle);
		dxfHex(330, 0);
		dxfString(100, "AcDbSymbolTable");
		dxfInt(70, num);
	}

	void DxfWriter::TableEntryHead(const char* name, int handle, int tableHandle)
	{
		dxfString(0, name);
		int gc;
		if (stricmp("DIMSTYLE", name) == 0)
			gc = 105;
		else
			gc = 5;
		dxfHex(gc, handle);
		dxfHex(330, tableHandle);
		dxfString(100, "AcDbSymbolTableRecord");
	}

	/**
	* Writes a viewport table.
	*/
	void DxfWriter::VPortTable(const CDblPoint& center, double height)
	{
		BeginTable("VPORT", 1, 8);

		TableEntryHead("VPORT", ++m_Handle, 8);
		dxfString(100, "AcDbViewportTableRecord");
		dxfString(2, "*Active");
		dxfInt(70, 0);
		dxfReal(10, 0.0);
		dxfReal(20, 0.0);
		dxfReal(11, 1.0);
		dxfReal(21, 1.0);
		dxfReal(12, center.x);
		dxfReal(22, center.y);
		dxfReal(13, 0.0);
		dxfReal(23, 0.0);
		dxfReal(14, 10.0);
		dxfReal(24, 10.0);
		dxfReal(15, 10.0);
		dxfReal(25, 10.0);
		dxfReal(16, 0.0);
		dxfReal(26, 0.0);
		dxfReal(36, 1.0);
		dxfReal(17, 0.0);
		dxfReal(27, 0.0);
		dxfReal(37, 0.0);
		dxfReal(40, height);
		dxfReal(41, 1.928);
		dxfReal(42, 50.0);
		dxfReal(43, 0.0);
		dxfReal(44, 0.0);
		dxfReal(50, 0.0);
		dxfReal(51, 0.0);
		dxfInt(71, 0);
		dxfInt(72, 100);
		dxfInt(73, 1);
		dxfInt(74, 3);
		dxfInt(75, 0);//关闭捕捉
		dxfInt(76, 1);
		dxfInt(77, 0);
		dxfInt(78, 0);
		dxfInt(281, 0);
		dxfInt(65, 1);
		dxfReal(110, 0.0);
		dxfReal(120, 0.0);
		dxfReal(130, 0.0);
		dxfReal(111, 1.0);
		dxfReal(121, 0.0);
		dxfReal(131, 0.0);
		dxfReal(112, 0.0);
		dxfReal(122, 1.0);
		dxfReal(132, 0.0);
		dxfInt(79, 0);
		dxfReal(146, 0.0);

		EndTable();
	}

	void DxfWriter::LTypeEntry_ByBlock_ByLayer_Continuous()
	{
		Linetype lt;
		lt.line_name = "ByBlock";
		LTypeEntry(lt, 0x14);
		lt.line_name = "ByLayer";
		LTypeEntry(lt, 0x15);
		lt.line_name = "Continuous";
		lt.line_descrip = "Solid line";
		LTypeEntry(lt, 0x16);
	}

	void DxfWriter::LTypeEntry(const char* name, int handle)
	{
		Linetype* lt = FindLinetype(name);
		ASSERT_DEBUG_INFO(lt);
		LTypeEntry(*lt, handle);
	}

	void DxfWriter::LTypeEntry(const Linetype& lt, int handle)
	{
		m_LinetypeHandleTable[lt.line_name] = handle;
		TableEntryHead("LTYPE", handle, 5);
		dxfString(100, "AcDbLinetypeTableRecord");
		dxfString(2, lt.line_name);
		dxfInt(70, 0);
		dxfString(3, lt.line_descrip);
		dxfInt(72, 65);
		dxfInt(73, (int)lt.descriptors.size());
		double len = 0;
		for (int i = 0; i < (int)lt.descriptors.size(); ++i)
			len += fabs(lt.descriptors[i].scale_length);
		dxfReal(40, len);
		for (int i = 0; i < (int)lt.descriptors.size(); ++i)
		{
			dxfReal(49, lt.descriptors[i].scale_length);
			dxfInt(74, 0);
		}
	}

	/**
	* Writes a layer to the file. Layers are stored in the
	* tables section of a DXF file.
	*
	* @param name: Layer name
	* @param attrib: Attributes
	*/
	void DxfWriter::LayerEntry(const std::pair<std::string, LayerData>& pair, int handle)
	{
		const auto& name = pair.first;
		ASSERT_DEBUG_INFO(!name.empty());

		const auto& data = pair.second;
		int color = data.m_Color != 0 ? data.m_Color : 7;
		string ltname = data.m_Linetype.empty() ? "CONTINUOUS" : data.m_Linetype;

		TableEntryHead("LAYER", handle, 2);

		dxfString(100, "AcDbLayerTableRecord");
		dxfString(2, name);
		dxfInt(70, 0);
		dxfInt(62, color);
		dxfString(6, ltname);
		if (!data.m_Plottable)
		{
			// layer cannot be plotted
			dxfInt(290, 0);
		}
		if (data.m_LineWeight != acLnWtByLayer)
		{
			dxfInt(370, data.m_LineWeight);
		}
		dxfHex(390, 0xF);//PlotStyleName 对象的硬指针 ID/句柄 
	}

	struct File2Face
	{
		const char* FileName;
		const char* CadFaceName;
		const char* WinFaceName;
	};

	//以下表格的第二列是大小写敏感的
	File2Face F2FMap[] =
	{
		{"SimSun.ttf",		"SimSun",					"宋体"},
		{"SIMHEI.TTF",		"SimHei",					"黑体"},
		{"SIMKAI.TTF",		"KaiTi_GB2312",				"楷体_GB2312"},
		{"SIMLI.TTF",		"LiSu",						"隶书"},
		{"SIMFANG.TTF",		"FangSong_GB2312",			"仿宋_GB2312"},
		{"SIMYOU.TTF",		"YouYuan",					"幼圆"},
		{"STCAIYUN.TTF",	"STCaiyun",					"华文彩云"},
		{"STFANGSO.TTF",	"STFangsong",				"华文仿宋"},
		{"STXIHEI.TTF",		"STXihei",					"华文细黑"},
		{"STXINGKA.TTF",	"STXingkai",				"华文行楷"},
		{"STXINWEI.TTF",	"STXinwei",					"华文新魏"},
		{"STZHONGS.TTF",	"STZhongsong",				"华文中宋"},
		{"FZSTK.TTF",		"FZShuTi",					"方正舒体"},
		{"FZYTK.TTF",		"FZYaoTi",					"方正姚体"},
		{"SURSONG.TTF",		"Simsun (Founder Extended)","宋体-方正超大字符集"},
		{"NSimSun.ttf",		"NSimSun",					"新宋体"}
	};

	const char* WinFaceFromFileName(const char* FileName)
	{
		for (int i = 0; i < sizeof(F2FMap) / sizeof(F2FMap[0]); ++i)
		{
			if (stricmp(F2FMap[i].FileName, FileName) == 0)
				return F2FMap[i].WinFaceName;
		}
		return F2FMap[0].WinFaceName;
	}

	const char* FileNameFromWinFace(const char* WinFaceName)
	{
		for (int i = 0; i < sizeof(F2FMap) / sizeof(F2FMap[0]); ++i)
		{
			if (stricmp(F2FMap[i].WinFaceName, WinFaceName) == 0)
				return F2FMap[i].FileName;
		}
		return F2FMap[0].FileName;
	}

	const char* CadFaceFromFileName(const char* FileName)
	{
		for (int i = 0; i < sizeof(F2FMap) / sizeof(F2FMap[0]); ++i)
		{
			if (stricmp(F2FMap[i].FileName, FileName) == 0)
				return F2FMap[i].CadFaceName;
		}
		return F2FMap[0].CadFaceName;
	}

	const char* FileNameFromCadFace(const char* cadFace)
	{
		for (int i = 0; i < sizeof(F2FMap) / sizeof(F2FMap[0]); ++i)
		{
			if (strcmp(F2FMap[i].CadFaceName, cadFace) == 0)
				return F2FMap[i].FileName;
		}
		return F2FMap[0].FileName;
	}

	void DxfWriter::StyleEntry(const std::pair<std::string, TextStyleData>& pair, int handle)
	{
		const auto& name = pair.first;
		const auto& data = pair.second;
		m_TextStyleHandleTable[name] = handle;
		TableEntryHead("STYLE", handle, 3);
		dxfString(100, "AcDbTextStyleTableRecord");
		dxfString(2, name);
		dxfInt(70, 0);
		dxfReal(40, data.m_Height);
		dxfReal(41, data.m_WidthFactor);
		dxfReal(50, data.m_ObliqueAngle);
		dxfInt(71, 0);
		dxfReal(42, 2.5);
		if (!data.m_TrueType.empty())
		{
			dxfString(3, "");
			dxfString(4, "");
			dxfString(1001, "ACAD");
			dxfString(1000, data.m_TrueType);
			dxfInt(1071, 257);
		}
		else
		{
			dxfString(3, data.m_PrimaryFontFile);
			dxfString(4, data.m_BigFontFile);
		}
	}

	void DxfWriter::ViewTable()
	{
		BeginTable("VIEW", 0, 6);
		EndTable();
	}

	void DxfWriter::UCSTable()
	{
		BeginTable("UCS", 0, 7);
		EndTable();
	}

	void DxfWriter::AppIdTable()
	{
		BeginTable("APPID", 2, 9);
		TableEntryHead("APPID", 0x12, 9);
		dxfString(100, "AcDbRegAppTableRecord");
		dxfString(2, "ACAD");
		dxfInt(70, 0);
		TableEntryHead("APPID", ++m_Handle, 9);
		dxfString(100, "AcDbRegAppTableRecord");
		dxfString(2, "ACADLL");
		dxfInt(70, 0);
		EndTable();
	}

	void DxfWriter::DimStyleEntry(const std::pair<std::string, DimStyleData>& pair, int handle)
	{
		const auto& name = pair.first;
		const DimStyleData& data = pair.second;
		m_DimStyleHandleTable[name] = handle;
		TableEntryHead("DIMSTYLE", handle, 0xA);
		dxfString(100, "AcDbDimStyleTableRecord");
		dxfString(2, name);
		dxfInt(70, 0);
		if (!data.Text.empty())
		{
			dxfString(3, data.Text);
			dxfString(4, data.Text);
		}
		dxfReal(41, data.ArrowHeadSize);
		dxfReal(42, data.ExtensionLineOffset);
		dxfReal(43, 3.75);//DIMDLI(尺寸线增量)
		dxfReal(44, data.ExtensionLineExtend);
		dxfReal(140, data.TextHeight);
		double centermarksize;
		if (data.CenterType == acCenterMark)
			centermarksize = data.CenterMarkSize;
		else if (data.CenterType == acCenterNone)
			centermarksize = 0;
		else
			centermarksize = -data.CenterMarkSize;
		dxfReal(141, centermarksize);
		if (data.LinearScaleFactor != 1.0)
			dxfReal(144, data.LinearScaleFactor);
		if (acAbove == data.VerticalTextPosition)
			dxfInt(77, 1);
		else
			dxfInt(145, data.VerticalTextPosition);
		dxfReal(147, data.TextGap);
		dxfInt(73, data.TextAlign ? 1 : 0);
		dxfInt(74, data.TextAlign ? 1 : 0);
		dxfInt(75, data.ExtLine1Suppress ? 1 : 0);
		dxfInt(76, data.ExtLine2Suppress ? 1 : 0);
		int flag = 0;
		if (data.SuppressLeadingZeros)
			flag |= 4;
		if (data.SuppressTrailingZeros)
			flag |= 8;
		dxfInt(78, flag);
		flag = 0;
		if (data.AngleSuppressLeadingZeros)
			flag |= 1;
		if (data.AngleSuppressTrailingZeros)
			flag |= 2;
		dxfInt(79, flag);
		dxfInt(171, data.UnitsPrecision);
		dxfInt(172, data.ForceLineInside ? 1 : 0);
		dxfInt(173, 1);//非零时使用单独的箭头块
		dxfInt(174, data.TextInside ? 1 : 0);
		dxfInt(176, data.DimensionLineColor);
		dxfInt(177, data.ExtensionLineColor);
		dxfInt(178, data.TextColor);
		dxfInt(179, data.AnglePrecision);
		dxfInt(271, data.UnitsPrecision);
		dxfInt(273, data.UnitsFormat);
		dxfInt(275, data.AngleFormat);
		dxfInt(277, data.UnitsFormat);
		dxfInt(278, '.');
		dxfInt(279, data.TextMovement);
		dxfInt(280, data.HorizontalTextPosition);
		dxfInt(281, data.DimLine1Suppress ? 1 : 0);
		dxfInt(282, data.DimLine2Suppress ? 1 : 0);
		dxfInt(289, data.Fit);
		dxfHex(340, LookupTextStyleHandle(string(data.TextStyle)));
		if (data.ArrowHead1Type == 20)//acArrowUserDefined
		{
			dxfHex(343, LookupBlockEntryHandle(string(data.ArrowHead1Block)));
		}
		else
		{
			const char* blockName = GetPredefinedArrowHeadBlockName(data.ArrowHead1Type);
			dxfHex(343, LookupBlockEntryHandle(blockName));
		}
		if (data.ArrowHead2Type == 20)//acArrowUserDefined
		{
			dxfHex(344, LookupBlockEntryHandle(string(data.ArrowHead2Block)));
		}
		else
		{
			const char* blockName = GetPredefinedArrowHeadBlockName(data.ArrowHead2Type);
			dxfHex(344, LookupBlockEntryHandle(blockName));
		}
		dxfInt(371, data.DimensionLineWeight);
		dxfInt(372, data.ExtensionLineWeight);
	}

	void DxfWriter::BlockRecordEntry(const char* blockname, int handle, int layoutHandle)
	{
		if (layoutHandle != 0)
		{
			m_LayoutHandleTable[blockname] = layoutHandle;
		}
		m_BlockEntryHandleTable[blockname] = handle;
		TableEntryHead("BLOCK_RECORD", handle, 1);
		dxfString(100, "AcDbBlockTableRecord");
		dxfString(2, blockname);
		dxfHex(340, layoutHandle);
	}

	void DxfWriter::BeginBlock(const char* blockName, double x, double y, int handle,
		bool bDimObliqueBlock)
	{
		dxfString(0, "BLOCK");
		dxfHex(5, handle);
		dxfHex(330, LookupBlockEntryHandle(blockName));
		dxfString(100, "AcDbEntity");
		if (strncmp(blockName, "*Paper_Space", strlen("*Paper_Space")) == 0)
			dxfInt(67, 1);
		dxfString(8, "0");
		dxfString(100, "AcDbBlockBegin");
		dxfString(2, blockName);
		dxfInt(70, bDimObliqueBlock ? 1 : 0);
		coord(10, x, y);
		dxfString(3, blockName);
		dxfString(1, "");
	}

	void DxfWriter::EndBlock(const char* blockname, int handle)
	{
		dxfString(0, "ENDBLK");
		dxfHex(5, handle);
		dxfHex(330, LookupBlockEntryHandle(blockname));
		dxfString(100, "AcDbEntity");

		if (string(blockname).find("*Paper_Space") != string::npos)
			dxfInt(67, 1);
		dxfString(8, "0");
		dxfString(100, "AcDbBlockEnd");
	}

	void DxfWriter::Block_ModelSpace()
	{
		BeginBlock("*Model_Space", 0, 0, 0x20);
		EndBlock("*Model_Space", 0x21);
		//BeginBlock("*Paper_Space", 0, 0, ++m_Handle);
		//EndBlock("*Paper_Space", ++m_Handle);
	}

	void DxfWriter::Block(const std::pair<std::string, std::shared_ptr<BlockDef>>& pair, bool bInPaperSpace)
	{
		const auto& blockName = pair.first;
		const auto& blockDef = pair.second;
		BeginBlock(blockName.c_str(), blockDef->m_InsertPoint.x, blockDef->m_InsertPoint.y, ++m_Handle);
		for (const auto& pEnt : blockDef->m_Objects)
		{
			pEnt->WriteDxf(*this, bInPaperSpace);
		}
		EndBlock(blockName.c_str(), ++m_Handle);
	}

	void DxfWriter::Dictionary(int handle, int parentHandle)
	{
		dxfString(0, "DICTIONARY");
		dxfHex(5, handle);
		if (0 != parentHandle)
		{
			dxfString(102, "{ACAD_REACTORS");
			dxfHex(330, parentHandle);
			dxfString(102, "}");
		}
		dxfHex(330, parentHandle);
		dxfString(100, "AcDbDictionary");
		dxfInt(281, 1);
	}

	void DxfWriter::Dictionaries()
	{
		// Root dictionary handle is always 0xC
		Dictionary(0xC, 0);
		//  Root dictionary's children dictionary entries
		DictionaryEntry("ACAD_COLOR", 0x6B);
		DictionaryEntry("ACAD_GROUP", 0xD);
		DictionaryEntry("ACAD_LAYOUT", 0x1A);
		DictionaryEntry("ACAD_MATERIAL", 0x6A);
		DictionaryEntry("ACAD_MLEADERSTYLE", 0x12D);
		DictionaryEntry("ACAD_MLINESTYLE", 0x17);
		DictionaryEntry("ACAD_PLOTSETTINGS", 0x19);
		DictionaryEntry("ACAD_PLOTSTYLENAME", 0xE);
		DictionaryEntry("ACAD_SCALELIST", 0x10C);
		DictionaryEntry("ACAD_TABLESTYLE", 0x7E);
		DictionaryEntry("ACAD_VISUALSTYLE", 0xEF);
		//DictionaryEntry("ACDB_RECOMPOSE_DATA", 0x2F0);
		DictionaryEntry("AcDbVariableDictionary", 0x5E);

		Dictionary(0x6B, 0xC); // ACAD_COLOR
		Dictionary(0xD, 0xC); // ACAD_GROUP
		// Begin of ACAD_LAYOUT
		Dictionary(0x1A, 0xC);
		DictionaryEntry("Model", LookupLayoutHandle("*Model_Space"));
		for (const auto& pair : m_Graph.m_Layouts)
		{
			const auto& pLayout = pair.second;
			DictionaryEntry(pair.first.c_str(), LookupLayoutHandle(pLayout->m_BlockName));
		}
		// End of ACAD_LAYOUT
		Dictionary(0x6A, 0xC); // ACAD_MATERIAL
		// Begin of ACAD_MLEADERSTYLE
		Dictionary(0x12D, 0xC);
		for (const auto& pair : m_MLeaderStyleHandleTable)
		{
			DictionaryEntry(pair.first.c_str(), pair.second);
		}
		// End of ACAD_MLEADERSTYLE
		Dictionary(0x17, 0xC); // ACAD_MLINESTYLE
		Dictionary(0x19, 0xC); // ACAD_PLOTSETTINGS
		// Begin of ACAD_PLOTSTYLENAME
		dxfString(0, "ACDBDICTIONARYWDFLT");
		dxfHex(5, 0xE);
		dxfString(100, "AcDbDictionary");
		dxfInt(281, 1);
		dxfString(3, "Normal");
		dxfHex(350, 0xF);
		dxfString(100, "AcDbDictionaryWithDefault");
		dxfHex(340, 0xF);
		dxfString(0, "ACDBPLACEHOLDER");
		dxfHex(5, 0xF);
		// End of ACAD_PLOTSTYLENAME
		Dictionary(0x10C, 0xC); // ACAD_SCALELIST
		// Begin of ACAD_TABLESTYLE
		Dictionary(0x7E, 0xC);
		for (const auto& pair : m_TableStyleHandleTable)
		{
			DictionaryEntry(pair.first.c_str(), pair.second);
		}
		// End of ACAD_TABLESTYLE
		Dictionary(0xEF, 0xC); // ACAD_VISUALSTYLE
		//Dictionary(0x2F0, 0xC); // ACDB_RECOMPOSE_DATA
		Dictionary(0x5E, 0xC); // AcDbVariableDictionary
	}

	void DxfWriter::Layout(const std::pair<std::string, std::shared_ptr<LayoutData>>& pair)
	{
		const auto& layoutName = pair.first;
		const auto& pLayout = pair.second;
		const bool isPaperSpace = pLayout->m_BlockName != "*Model_Space";
		dxfString(0, "LAYOUT");
		dxfHex(5, m_LayoutHandleTable[pLayout->m_BlockName]);
		dxfString(102, "{ACAD_REACTORS");
		dxfHex(330, 0x1A);
		dxfString(102, "}");
		dxfHex(330, 0x1A);
		dxfString(100, "AcDbPlotSettings");
		dxfString(1, "");
		dxfString(2, pLayout->m_PlotSettings.m_PlotConfigFile);
		dxfString(4, pLayout->m_PlotSettings.m_PaperName);
		dxfString(6, "");
		dxfReal(40, pLayout->m_PlotSettings.m_LeftMargin);
		dxfReal(41, pLayout->m_PlotSettings.m_BottomMargin);
		dxfReal(42, pLayout->m_PlotSettings.m_RightMargin);
		dxfReal(43, pLayout->m_PlotSettings.m_TopMargin);
		dxfReal(44, pLayout->m_PlotSettings.m_Width);
		dxfReal(45, pLayout->m_PlotSettings.m_Height);
		dxfReal(46, pLayout->m_PlotSettings.m_PlotOrigin.x);
		dxfReal(47, pLayout->m_PlotSettings.m_PlotOrigin.y);
		dxfReal(48, 0);
		dxfReal(49, 0);
		dxfReal(140, 0);
		dxfReal(141, 0);
		dxfReal(142, 1.0);
		dxfReal(143, 1.0);
		if (isPaperSpace)
			dxfInt(70, 640);
		else
			dxfInt(70, 1712);
		dxfInt(72, 1);
		dxfInt(73, pLayout->m_PlotSettings.m_PlotRotation);
		if (isPaperSpace)
			dxfInt(74, 5);
		else
			dxfInt(74, 0);
		dxfString(7, "");
		if (isPaperSpace)
		{
			dxfInt(75, 0);
			dxfInt(76, 1);
		}
		else
		{
			dxfInt(75, 0);
			dxfInt(76, 0);
		}
		dxfInt(77, 2);
		dxfInt(78, 300);
		dxfReal(147, 1.0);
		dxfReal(148, 0);
		dxfReal(149, 0);
		dxfString(100, "AcDbLayout");
		dxfString(1, layoutName);
		//Group Code:70控制以下各项的标志（按位编码）：
		//	1 = 表示当此布局处于当前状态时的 PSLTSCALE 值
		//	2 = 表示当此布局处于当前状态时的 LIMCHECK 值
		dxfInt(70, 0);
		dxfInt(71, pLayout->m_LayoutOrder);
		coord(10, pLayout->m_MinLim.x, pLayout->m_MinLim.y);
		coord(11, pLayout->m_MaxLim.x, pLayout->m_MaxLim.y);
		coord(12, 0, 0);
		coord(14, pLayout->m_MinExt.x, pLayout->m_MinExt.y);
		coord(15, pLayout->m_MaxExt.x, pLayout->m_MaxExt.y);
		dxfReal(146, 0);
		coord(13, 0, 0);
		coord(16, 1.0, 0);
		coord(17, 0, 1.0);
		dxfInt(76, 0);
		int blockEntryHandle;
		if (isPaperSpace)
		{
			blockEntryHandle = LookupBlockEntryHandle(pLayout->m_BlockName);
		}
		else
		{
			blockEntryHandle = LookupBlockEntryHandle("*Model_Space");
		}
		dxfHex(330, blockEntryHandle);
		dxfHex(331, 0);

		++m_LayoutIndex;
	}

}
