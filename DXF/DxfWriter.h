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

#include <iostream>
#include <math.h>
#include <vector>
#include <map>
#include <fstream>

#include "AcadEntities.h"
#include "Linetype.h"
#include "Pattern.h"

namespace DXF
{

	class DxfData;
	class DXF_API DxfWriter
	{
	public:
		DxfWriter(DxfData &graph);
		DxfWriter(const DxfWriter &) = delete;
		// linFile - file name of line type (*.lin)
		// patFile - file name of pattern (*.pat)
		// center, height - center and height of model view when you open this dxf file.
		void WriteDxf(const char *dxfFile, const char *linFile, const char *patFile, const CDblPoint &center, double height);

	private:
		void Reset();

		void BeginSection(const char *name)
		{
			dxfString(0, "SECTION");
			dxfString(2, name);
		}

		void EndSection()
		{
			dxfString(0, "ENDSEC");
		}

		void HeaderSection();

		void BeginTable(const char *name, int num, int handle);

		void EndTable()
		{
			dxfString(0, "ENDTAB");
		}

		void TableEntryHead(const char *name, int handle, int tableHandle);

		void VPortTable(const CDblPoint &center, double height);

		void BeginLTypeTable(int num)
		{
			BeginTable("LTYPE", num, 5);
		}
		void LTypeEntry(const Linetype &lt, int handle);
		void LTypeEntry(const char *name, int handle);
		void LTypeEntry_ByBlock_ByLayer_Continuous();

		void BeginLayerTable(int num)
		{
			BeginTable("LAYER", num, 2);
		}
		void LayerEntry(const std::pair<std::string, LayerData> &pair, int handle);

		void BeginStyleTable(int num)
		{
			BeginTable("STYLE", num, 3);
		}
		void StyleEntry(const std::pair<std::string, TextStyleData> &pair, int handle);

		void ViewTable();

		void UCSTable();

		void AppIdTable();

		void BeginDimStyleTable(int num)
		{
			BeginTable("DIMSTYLE", num, 0xA);
			dxfString(100, "AcDbDimStyleTable");
			dxfInt(71, 0);
			//下面一句似乎是指定缺省的或者当前的DimStyle
			dxfHex(340, 0x27); //handle of dimstyle ISO-25
		}

		void DimStyleEntry(const std::pair<std::string, DimStyleData> &pair, int handle);

		//num = number of layouts + number of blocks + 2(*Model_Space,*Paper_Space)
		void BeginBlock_RecordTable(int num)
		{
			BeginTable("BLOCK_RECORD", num, 1);
		}

		void BlockRecordEntry(const char *blockname, int handle, int layoutHandle);

		//还有一些标注箭头块需要添加
		void BlockRecordEntry_ModelSpace()
		{
			BlockRecordEntry("*Model_Space", 0x1F, 0x22);
		}

		void BeginBlock(const char *blockName, double x, double y, int handle,
						bool bDimObliqueBlock = false);
		void EndBlock(const char *blockname, int handle);
		void Block_ModelSpace();

		void Block(const std::pair<std::string, std::shared_ptr<BlockDef>> &blockDef, bool bInPaperSpace);

		void Dictionary(int handle, int parentHandle);
		//handle是条目所指对象的句柄
		void DictionaryEntry(const char *entryName, int handle)
		{
			dxfString(3, entryName);
			dxfHex(350, handle);
		}
		void Dictionaries();

		//注意Layout的调用顺序。
		void Layout(const std::pair<std::string, std::shared_ptr<LayoutData>> &pLayout);

		//在ENTITIES段中先输出输出模型空间图元，再*Paper_Space图纸空间图元。
		//区分图元属于模型空间和图纸空间的标志是组码 67。
		//*Paper_Space(n)图纸空间图元输出在BLOCKS段中

	public: // Used internally
		int LookupTextStyleHandle(const std::string &name);
		int LookupDimStyleHandle(const std::string &name);
		int LookupBlockEntryHandle(const std::string &name);
		int LookupLayoutHandle(const std::string &name);
		int LookupMLeaderStyleHandle(const std::string &name);
		int LookupTableStyleHandle(const std::string &name);
		int LookupLinetypeHandle(const std::string &name);
		void AddMLeaderToItsStyle(const std::string &mleaderStyle, const EntAttribute *pMLeader);
		const vector<const EntAttribute *> &GetMLeaders(const std::string &mleaderStyle);
		void AddTableToItsStyle(const std::string &tableStyle, const EntAttribute *pTable);
		const vector<const EntAttribute *> &GetTables(const std::string &tableStyle);

		void dxfReal(int gc, double value);
		void dxfInt(int gc, int value);
		void dxfHex(int gc, int value);
		void dxfString(int gc, const char *value);
		void dxfString(int gc, const char *value, size_t count);
		void dxfString(int groupCode, const std::string &value)
		{
			dxfString(groupCode, value.c_str());
		}
		//二进制DXF不得调用
		void Comment(const char *text)
		{
			dxfString(999, text);
		}

		void dxfEOF()
		{
			dxfString(0, "EOF");
		}

		void coord(int gc, double x, double y)
		{
			dxfReal(gc, x);
			dxfReal(gc + 10, y);
		}

		void coord(int gc, const double *value)
		{
			_ASSERTE(value);
			coord(gc, value[0], value[1]);
		}

		void dxfText(int gc, int gcLast, const std::string &text);

		Linetype *FindLinetype(const char *lineTypeName)
		{
			return m_LinetypeManager.FindLinetype(lineTypeName);
		}

		Pattern *FindPattern(const char *ptname)
		{
			return m_PatternManager.FindPattern(ptname);
		}

		static VectorOfEntityList s_PredefinedBlocks;

		int m_CurrentBlockEntryHandle;
		int m_CurrentViewportID;

	private:
		int PrepareBeforeWrite(int handle);

		std::ofstream m_DxfFile;
		DxfData &m_Graph;
		LinetypeManager m_LinetypeManager;
		PatternManager m_PatternManager;
		int m_Handle;
		int m_LayoutIndex;
		std::map<std::string, int> m_TextStyleHandleTable;
		std::map<std::string, int> m_DimStyleHandleTable;
		// block name -> block entry handle
		std::map<std::string, int> m_BlockEntryHandleTable;
		// layout block name -> layout handle
		std::map<std::string, int> m_LayoutHandleTable;
		std::map<std::string, int> m_MLeaderStyleHandleTable;
		std::map<std::string, int> m_TableStyleHandleTable;
		std::map<std::string, int> m_LinetypeHandleTable;
		std::map<std::string, std::vector<const EntAttribute *>> m_MLeaderStyleEntities;
		std::map<std::string, std::vector<const EntAttribute *>> m_TableStyleEntities;
	};

} // namespace DXF
