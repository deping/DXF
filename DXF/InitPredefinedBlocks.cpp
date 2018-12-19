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
#include "AcadEntities.h"
#include <mutex>

namespace
{

using namespace DXF;
void Solid(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 0);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadSolid>();
	ent0->m_Layer = "0";
	ent0->m_Color = acByBlock;
	ent0->m_Point1.SetPoint(-1, 1.0 / 6.0);
	ent0->m_Point2.SetPoint(0, 0);
	ent0->m_Point3.SetPoint(-1, -1.0 / 6.0);
	ent0->m_Point4.SetPoint(-1, 1.0 / 6.0);
	g_PredefinedBlocks[0]->push_back(ent0);
}

void ClosedBlank(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 1);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-1, 1.0 / 6.0);
	ent0->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[1]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(0, 0);
	ent1->m_EndPoint.SetPoint(-1, -1.0 / 6.0);
	g_PredefinedBlocks[1]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(-1, 1.0 / 6.0);
	ent2->m_EndPoint.SetPoint(-1, -1.0 / 6.0);
	g_PredefinedBlocks[1]->push_back(ent2);
}

void Closed(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 2);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-1, 1.0 / 6.0);
	ent0->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[2]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(0, 0);
	ent1->m_EndPoint.SetPoint(-1, -1.0 / 6.0);
	g_PredefinedBlocks[2]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(-1, 1.0 / 6.0);
	ent2->m_EndPoint.SetPoint(-1, -1.0 / 6.0);
	g_PredefinedBlocks[2]->push_back(ent2);

	auto ent3 = std::make_shared<AcadLine>();
	ent3->m_Layer = "0";
	ent3->m_Linetype = "ByBlock";
	ent3->m_Color = acByBlock;
	ent3->m_Lineweight = acLnWtByBlock;
	ent3->m_StartPoint.SetPoint(0, 0);
	ent3->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[2]->push_back(ent3);
}

void Dot(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 3);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLWPLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Closed = TRUE;
	ent0->SetConstWidth(0.5);
	ent0->m_Vertices.push_back(CDblPoint(-0.25, 0));
	ent0->SetBulge(0, 1.0);
	ent0->m_Vertices.push_back(CDblPoint(0.25, 0));
	ent0->SetBulge(1, 1.0);
	g_PredefinedBlocks[3]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-0.5, 0);
	ent1->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[3]->push_back(ent1);
}

void ArchTick(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 4);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLWPLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Closed = FALSE;
	ent0->SetConstWidth(0.15);
	ent0->m_Vertices.push_back(CDblPoint(-0.5, -0.5));
	ent0->m_Vertices.push_back(CDblPoint(0.5, 0.5));
	g_PredefinedBlocks[4]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-1, 0);
	ent1->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[4]->push_back(ent1);
}

void Oblique(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 5);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-0.5, -0.5);
	ent0->m_EndPoint.SetPoint(0.5, 0.5);
	g_PredefinedBlocks[5]->push_back(ent0);
}

void Open(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 6);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-1, 1.0 / 6.0);
	ent0->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[6]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-1, -1.0 / 6.0);
	ent1->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[6]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(0, 0);
	ent2->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[6]->push_back(ent2);
}

void Origin(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 7);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadCircle>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_Center.SetPoint(0, 0);
	ent0->m_Radius = 0.5;
	g_PredefinedBlocks[7]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(0, 0);
	ent1->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[7]->push_back(ent1);
}

void Origin2(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 8);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadCircle>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_Center.SetPoint(0, 0);
	ent0->m_Radius = 0.5;
	g_PredefinedBlocks[8]->push_back(ent0);

	auto copy = std::make_shared<AcadCircle>(*ent0);
	copy->m_Radius = 0.25;
	g_PredefinedBlocks[8]->push_back(copy);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-0.5, 0);
	ent1->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[8]->push_back(ent1);
}

void Open90(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 9);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-0.5, 0.5);
	ent0->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[9]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(0, 0);
	ent1->m_EndPoint.SetPoint(-0.5, -0.5);
	g_PredefinedBlocks[9]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(0, 0);
	ent2->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[9]->push_back(ent2);
}

void Open30(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 10);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-1, 0.26794919);
	ent0->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[10]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-1, -0.26794919);
	ent1->m_EndPoint.SetPoint(0, 0);
	g_PredefinedBlocks[10]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(0, 0);
	ent2->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[10]->push_back(ent2);
}

void DotSmall(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 11);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLWPLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Closed = TRUE;
	ent0->SetConstWidth(0.5);
	ent0->m_Vertices.push_back(CDblPoint(-0.0625, 0));
	ent0->SetBulge(0, 1.0);
	ent0->m_Vertices.push_back(CDblPoint(0.0625, 0));
	ent0->SetBulge(1, 1.0);
	g_PredefinedBlocks[11]->push_back(ent0);
}

void DotBlank(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 12);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadCircle>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_Center.SetPoint(0, 0);
	ent0->m_Radius = 0.5;
	g_PredefinedBlocks[12]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-0.5, 0);
	ent1->m_EndPoint.SetPoint(-1.0, 0);
	g_PredefinedBlocks[12]->push_back(ent1);
}

void Small(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 13);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadCircle>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_Center.SetPoint(0, 0);
	ent0->m_Radius = 0.25;
	g_PredefinedBlocks[13]->push_back(ent0);
}

void BoxBlank(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 14);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(-0.5, -0.5);
	ent0->m_EndPoint.SetPoint(0.5, -0.5);
	g_PredefinedBlocks[14]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(0.5, -0.5);
	ent1->m_EndPoint.SetPoint(0.5, 0.5);
	g_PredefinedBlocks[14]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(0.5, 0.5);
	ent2->m_EndPoint.SetPoint(-0.5, 0.5);
	g_PredefinedBlocks[14]->push_back(ent2);

	auto ent3 = std::make_shared<AcadLine>();
	ent3->m_Layer = "0";
	ent3->m_Linetype = "ByBlock";
	ent3->m_Color = acByBlock;
	ent3->m_Lineweight = acLnWtByBlock;
	ent3->m_StartPoint.SetPoint(-0.5, 0.5);
	ent3->m_EndPoint.SetPoint(-0.5, -0.5);
	g_PredefinedBlocks[14]->push_back(ent3);

	auto ent4 = std::make_shared<AcadLine>();
	ent4->m_Layer = "0";
	ent4->m_Linetype = "ByBlock";
	ent4->m_Color = acByBlock;
	ent4->m_Lineweight = acLnWtByBlock;
	ent4->m_StartPoint.SetPoint(-0.5, 0);
	ent4->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[14]->push_back(ent4);
}

void BoxFilled(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 15);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadSolid>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Point1.SetPoint(-0.5, 0.5);
	ent0->m_Point2.SetPoint(0.5, 0.5);
	ent0->m_Point3.SetPoint(-0.5, -0.5);
	ent0->m_Point4.SetPoint(0.5, -0.5);
	g_PredefinedBlocks[15]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-0.5, 0);
	ent1->m_EndPoint.SetPoint(-1, 0);
	g_PredefinedBlocks[15]->push_back(ent1);
}

void DatumBlank(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 16);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadLine>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_StartPoint.SetPoint(0, 0.5773502700000001);
	ent0->m_EndPoint.SetPoint(-1.0, 0);
	g_PredefinedBlocks[16]->push_back(ent0);

	auto ent1 = std::make_shared<AcadLine>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_StartPoint.SetPoint(-1.0, 0);
	ent1->m_EndPoint.SetPoint(0, -0.5773502700000001);
	g_PredefinedBlocks[16]->push_back(ent1);

	auto ent2 = std::make_shared<AcadLine>();
	ent2->m_Layer = "0";
	ent2->m_Linetype = "ByBlock";
	ent2->m_Color = acByBlock;
	ent2->m_Lineweight = acLnWtByBlock;
	ent2->m_StartPoint.SetPoint(0, 0.5773502700000001);
	ent2->m_EndPoint.SetPoint(0, -0.5773502700000001);
	g_PredefinedBlocks[16]->push_back(ent2);
}

void DatumFilled(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 17);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadSolid>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Point1.SetPoint(0, 0.5773502700000001);
	ent0->m_Point2.SetPoint(-1.0, 0);
	ent0->m_Point3.SetPoint(0, -0.5773502700000001);
	ent0->m_Point4.SetPoint(0, -0.5773502700000001);
	g_PredefinedBlocks[17]->push_back(ent0);
}

void Integral(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 18);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
	auto ent0 = std::make_shared<AcadArc>();
	ent0->m_Layer = "0";
	ent0->m_Linetype = "ByBlock";
	ent0->m_Color = acByBlock;
	ent0->m_Lineweight = acLnWtByBlock;
	ent0->m_Center.SetPoint(0.44488802, -0.09133463);
	ent0->m_Radius = 0.4541666700000002;
	ent0->m_StartAngle = 101.9999999980395;
	ent0->m_EndAngle = 167.9999999799193;
	g_PredefinedBlocks[18]->push_back(ent0);

	auto ent1 = std::make_shared<AcadArc>();
	ent1->m_Layer = "0";
	ent1->m_Linetype = "ByBlock";
	ent1->m_Color = acByBlock;
	ent1->m_Lineweight = acLnWtByBlock;
	ent1->m_Center.SetPoint(-0.44488802, 0.09133463);
	ent1->m_Radius = 0.4541666700000002;
	ent1->m_StartAngle = 282.0000000215427;
	ent1->m_EndAngle = 348.0000000034225;
	g_PredefinedBlocks[18]->push_back(ent1);
}

void None(VectorOfEntityList& g_PredefinedBlocks)
{
	ASSERT_DEBUG_INFO(g_PredefinedBlocks.size() == 19);
	g_PredefinedBlocks.push_back(std::make_shared<EntityList>());
}
}

namespace DXF {

void InitPredefinedBlocks(VectorOfEntityList& g_PredefinedBlocks)
{
	static std::mutex g_mutex;
	std::lock_guard<std::mutex> lock(g_mutex);
	if (!g_PredefinedBlocks.empty())
		return;
	Solid(g_PredefinedBlocks);
	ClosedBlank(g_PredefinedBlocks);
	Closed(g_PredefinedBlocks);
	Dot(g_PredefinedBlocks);
	ArchTick(g_PredefinedBlocks);
	Oblique(g_PredefinedBlocks);
	Open(g_PredefinedBlocks);
	Origin(g_PredefinedBlocks);
	Origin2(g_PredefinedBlocks);
	Open90(g_PredefinedBlocks);
	Open30(g_PredefinedBlocks);
	DotSmall(g_PredefinedBlocks);
	DotBlank(g_PredefinedBlocks);
	Small(g_PredefinedBlocks);
	BoxBlank(g_PredefinedBlocks);
	BoxFilled(g_PredefinedBlocks);
	DatumBlank(g_PredefinedBlocks);
	DatumFilled(g_PredefinedBlocks);
	Integral(g_PredefinedBlocks);
	None(g_PredefinedBlocks);
}

}
