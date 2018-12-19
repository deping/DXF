// DxfTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DxfData.h"
#include "DxfReader.h"
#include "DxfWriter.h"
#include <Windows.h>

int main(int argc, char* argv[])
{
	using namespace DXF;

	// Test AdvanceUtf8
	SetConsoleOutputCP(CP_UTF8);
	//std::string text(u8"一二三四五六七八九二二三四五六七八九三二三四五六七八九四二三四五六七八九五二三四五六七八九六二三四五六七八九七二三四五六七八九八二三四五六七八九九二三四五六七八九十二三四五六七八九");
	//size_t len = text.length();
	//const char* pStart = text.data();
	//const char* pEnd = pStart + len;
	//const char* pCur = pStart;
	//while (pCur < pEnd)
	//{
	//	pCur = AdvanceUtf8(pStart, pEnd, 250);
	//	if (pCur == pEnd)
	//	{
	//		std::string tmp(pStart, pCur - pStart);
	//		printf(tmp.c_str());
	//		printf("\n");
	//		break;
	//	}
	//	std::string tmp(pStart, pCur - pStart);
	//	printf(tmp.c_str());
	//	printf("\n");
	//	pStart = pCur;
	//}

	//// Test DXF read/write
	EnableDebugInfo(true);
	DxfData dxf;
	////auto pDim = std::make_shared<AcadDimAln>();
	////pDim->m_DefPoint.SetPoint(80, 120);
	////pDim->m_TextPosition.SetPoint(30, 70);
	////pDim->m_ExtLine1Point.SetPoint(0, 0);
	////pDim->m_ExtLine2Point.SetPoint(100, 100);
	////pDim->m_DimStyleName = "ISO-25";
	////dxf.m_Objects.push_back(pDim);
	////auto pTable = std::make_shared<AcadTable>();
	////pTable->m_InsertionPoint.SetPoint(0, 0);
	////pTable->InitializeCells(2, 2, DXF::CellType::TEXT_CELL);
	////pTable->SetColWidth(600);
	////pTable->SetRowHeight(330);
	////for (int i=0; i<pTable->m_RowCount; ++i)
	////	for (int j = 0; j < pTable->m_ColCount; ++j)
	////	{
	////		Cell& c = pTable->GetCell(i, j);
	////		c.m_TextHeight = 250;
	////		std::shared_ptr<CellText>& ct = dynamic_pointer_cast<CellText>(c.m_Content);
	////		ct->m_Text = "0*0";
	////	}
	////dxf.m_Objects.push_back(pTable);
	//auto pH = std::make_shared<AcadHatch>();
	//auto pC = std::make_shared<AcadCircle>();
	//pC->m_Center.SetPoint(10, 10);
	//pC->m_Radius = 50;
	//pH->AddEntity(pC, -1);
	//pH->AddAssociatedEntity(pC, -1);
	//pC->m_Reactors.push_back(pH);
	//dxf.m_Objects.push_back(pC);
	//dxf.m_Objects.push_back(pH);
	//DxfWriter dxfWriter(dxf);
	//dxfWriter.WriteDxf(R"(G:\SharedWithVM\test2.dxf)", R"(..\DXF\acadiso.lin)", R"(..\DXF\acadiso.pat)", CDblPoint(0, 0), 100);
	//if (argc == 2)
	//{
	//	DxfReader dxfReader(dxf);
	//	if (dxfReader.ReadDxf(argv[1], false))
	//	{
	//		dxfWriter.WriteDxf(R"(G:\SharedWithVM\test3.dxf)", R"(..\DXF\acadiso.lin)", R"(..\DXF\acadiso.pat)", CDblPoint(2000, 1400), 3300);
	//	}
	//}
    std::shared_ptr<DXF::AcadCircle>ciPtr;
    ciPtr.reset(new DXF::AcadCircle);
    {
        ciPtr->m_Handle = 0; // m_Handle will be assigned value on writing.
        ciPtr->m_IsInPaperspace = false;
        ciPtr->m_Color = (AcColor)acByLayer;
        ciPtr->m_Lineweight = (AcLineWeight)acLnWtByLayer;
        ciPtr->m_LinetypeScale = 1.;
        ciPtr->m_Layer = "0";
        ciPtr->m_Linetype = "";
        //ciPtr->m_debug = "";
    }
    ciPtr->m_Center = DXF::CDblPoint(0., 0.);
    ciPtr->m_Radius = 22.;
    dxf.m_Objects.push_back(ciPtr);

    std::shared_ptr<DXF::AcadHatch>haPtr;
    haPtr.reset(new DXF::AcadHatch);
    {
        haPtr->m_Handle = 0; // m_Handle will be assigned value on writing.
        haPtr->m_IsInPaperspace = false;
        haPtr->m_Color = (AcColor)acByLayer;
        haPtr->m_Lineweight = (AcLineWeight)acLnWtByLayer;
        haPtr->m_LinetypeScale = 1.;
        haPtr->m_Layer = "0";
        haPtr->m_Linetype = "";
        //haPtr->m_debug = "";
    }
    //enum FillFlag { PatternFill, SolidFill }; // 图案填充, 实体填充
    bool state = haPtr->AddEntity(ciPtr);
    haPtr->AddAssociatedEntity(ciPtr);
    haPtr->m_FillFlag = AcadHatch::SolidFill;
    ciPtr->m_Reactors.push_back(haPtr);
    dxf.m_Objects.push_back(haPtr);
    DxfWriter dxfWriter(dxf);
    dxfWriter.WriteDxf("F:\\test.dxf", R"(..\DXF\acadiso.lin)", R"(..\DXF\acadiso.pat)", CDblPoint(0, 0), 100);
    DxfReader dxfReader(dxf);
    dxfReader.ReadDxf("G:\\SharedWithVM\\test3cpp.dxf", false);
    DxfWriter dxfWriter0(dxf);
    dxfWriter0.WriteDxf("F:\\test1.dxf", R"(..\DXF\acadiso.lin)", R"(..\DXF\acadiso.pat)", CDblPoint(0, 0), 100);

    return 0;
}

