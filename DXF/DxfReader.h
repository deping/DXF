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

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "ifstream.h"
#include "AcadEntities.h"
#include "ImpExpMacro.h"

namespace DXF {

class DxfData;
struct BlockDef;
struct EntAttribute;
struct LayoutData;
struct NameHandle;

class DXF_API DxfReader
{
public:
	DxfReader(DxfData& graph);
	DxfReader(const DxfReader&) = delete;
	//返回true，表示成功;如果返回false,表示失败,graph处于不可用状态．
	bool ReadDxf(const char* dxfFileName, bool OnlyReadModelSpace);
	~DxfReader(void);

private:
	bool ReadHeader();
	bool ReadTables();
	bool ReadBlocks();
	bool ReadEntities();
	bool ReadObjects();

private:
	enum { BUFSIZE = 256 };
	void ReadPair(int& groupCode, char* value, int bufsize = BUFSIZE);
	bool AdvanceToSection(const char* sectionName);
	bool AdvanceToObject(const char* objectType);
	bool AdvanceToObjectStopAtLayout(const char* objectType);
	bool NextVariable(char* varName, char* varVal);
	bool NextTable(char* tableName);
	bool NextTableEntry(const char* entryName);
	void ReadTableEntry(std::string&name, void* data);
	void (DxfReader::*HandleTableEntryCode)(std::string& name, void* data, int groupCode, const char* value);
	void HandleBlockRecordEntryCode(std::string& name, void* data, int groupCode, const char* value);
	void HandleLayerEntryCode(std::string& name, void* data, int groupCode, const char* value);
	void HandleLTypeEntryCode(std::string& name, void* data, int groupCode, const char* value);
	void HandleStyleEntryCode(std::string& name, void* data, int groupCode, const char* value);
	void HandleDimStyleEntryCode(std::string& name, void* data, int groupCode, const char* value);
	bool NextBlock();
	bool NextObject(const char* objectType);
	void ReadDictionaryEntries(std::function<bool(const NameHandle&)> HandleDicEntry);
	LayoutData* ReadBlockBegin(std::string& blockName, /*in,out*/BlockDef* block);
	bool NextEntity(char* entityType);
	void ReadAllEntities(EntityList* pObjList, bool bIn_ENTITIES_Section);

	void ReadAttribute();
	void SetAttribute(EntAttribute* pEnt);
	void ReadArc(EntityList* pObjList);
	void ReadAttDef(EntityList* pObjList);
	void ReadAttrib(EntityList* pObjList);
	void ReadInsert(EntityList* pObjList);
	void ReadCircle(EntityList* pObjList);
	void ReadDimCommon();
	void SetDimCommon(AcadDim* pDim);
	void ReadDimension(EntityList* pObjList);
	void ReadDimRot(EntityList* pObjList);
	void ReadDimOrd(EntityList* pObjList);
	void ReadDimAln(EntityList* pObjList);
	void ReadDimDia(EntityList* pObjList);
	void ReadDimRad(EntityList* pObjList);
	void ReadDimAng3P(EntityList* pObjList);
	void ReadDimAng(EntityList* pObjList);
	void ReadEllipse(EntityList* pObjList);
	void ReadHatch(EntityList* pObjList);
	void ReadLeader(EntityList* pObjList);
	void ReadMLeader(EntityList* pObjList);
	void ReadContextData(CONTEXT_DATA& cd);
	void ReadLeaderPart(LeaderPart& leader);
	void ReadLeaderLine(LeaderLine& leaderLine);
	ContentType GetContentData(CONTEXT_DATA & cd);
	void ReadBlockPart(BlockPart& block);
	void ReadMTextPart(MTextPart& mtext);
	void ReadLine(EntityList* pObjList);
	void ReadLwpolyline(EntityList* pObjList);
	void ReadMtext(EntityList* pObjList);
	void ReadPoint(EntityList* pObjList);
	void ReadSolid(EntityList* pObjList);
	void ReadSpline(EntityList* pObjList);
	void ReadText(EntityList* pObjList);
	void ReadViewport(EntityList* pObjList);
	void ReadLayout(std::string& layoutName, LayoutData&);
	void ReadMLeaderStyle(MLeaderStyle&);
	void ReadStyleBlockPart(StyleBlockPart& block);
	void ReadStyleMTextPart(StyleMTextPart& mtext);
	void ReadTable(EntityList* pObjList);
	void ReadCell(Cell& cell);
	void ReadTableStyle(TableStyle& ts);

	std::string LookupTextStyleName(int handle);
	std::string LookupBlockEntryName(int handle);
	std::string LookupMLeaderStyleName(int handle);
	std::string LookupLinetypeName(int handle);
	std::string LookupTableStyleName(int handle);
	std::shared_ptr<LayoutData> LookupLayout(const char* blockName);

	void ConvertHandle();
	void Reset();

private:
	DXF::ifstream m_DxfFile;
	DxfData& m_Graph;
	AcadDimInternal m_Attribute;
	bool m_OnlyReadModelSpace;
	std::map<int, std::string> m_TextStyleHandleTable;
	std::map<int, std::string> m_BlockEntryHandleTable;
	std::map<int, std::string> m_MLeaderStyleHandleTable;
	std::map<int, std::string> m_LinetypeHandleTable;
	std::map<int, std::string> m_TableStyleHandleTable;

    std::map<int, std::vector<int>> m_ToBeConvertedReactors;
    std::map<int, int> m_ToBeConvertedViewportClipEnts;
    std::map<int, int> m_ToBeConvertedLeaderAnnotation;
    std::map<int, std::vector<int>> m_ToBeConvertedMLeaderAttDefs;
    std::map<int, int> m_ToBeConvertedMLeaderStyleName;
    std::map<int, int> m_ToBeConvertedTableStyleName;
    std::map<std::string, int> m_ToBeConvertedTextStyleName;
    std::map<std::string, int> m_ToBeConvertedArrow1Name;
    std::map<std::string, int> m_ToBeConvertedArrow2Name;
    std::map<int, std::vector<std::pair<int, int>>> m_ToBeConvertedHatchAssociatedEnts;

    int m_CurObjectHandle;
};

}
