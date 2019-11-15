/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
#include "AcadEntities.h"
#include <stdarg.h>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <cmath>

#include "DxfData.h"
#include "DxfWriter.h"
#include "Pattern.h"

namespace DXF {

const char* PredefinedArrowheadBlock[] = {
	"_Solid"
	,"_ClosedBlank"
	,"_Closed"
	,"_Dot"
	,"_ArchTick"
	,"_Oblique"
	,"_Open"
	,"_Origin"
	,"_Origin2"
	,"_Open90"
	,"_Open30"
	,"_DotSmall"
	,"_DotBlank"
	,"_Small"
	,"_BoxBlank"
	,"_BoxFilled"
	,"_DatumBlank"
	,"_DatumFilled"
	,"_Integral"
	,"_None"
};

const char* GetPredefinedArrowHeadBlockName(int predefinedArrowheadType)
{
	ASSERT_DEBUG_INFO(predefinedArrowheadType >= 0 && predefinedArrowheadType < _countof(PredefinedArrowheadBlock));
	return PredefinedArrowheadBlock[predefinedArrowheadType];
}

int GetArrowHeadType(const char* value)
{
	for (int i = 0; i < _countof(PredefinedArrowheadBlock); ++i)
	{
		if (strcmp(PredefinedArrowheadBlock[i], value) == 0)
			return i;
	}
	return acArrowUserDefined;
}

EntAttribute::EntAttribute()
	: m_Handle(0)
	, m_IsInPaperspace(false)
	, m_Color(acByLayer)
	, m_Lineweight(acLnWtByLayer)
	, m_LinetypeScale(1.0)
	, m_Layer("0")
	, m_Linetype("ByLayer")
{
}

void EntAttribute::AssignHandle(int & handle)
{
	m_Handle = ++handle;
}

void EntAttribute::WriteAttribute(DxfWriter& writer, bool bInPaperSpace) const
{
	writer.dxfHex(5, m_Handle);
	std::vector<std::shared_ptr<EntAttribute>> reactors;
	for (const auto& reactor : m_Reactors)
	{
		auto spReactor = reactor.lock();
		if (spReactor)
			reactors.push_back(spReactor);
	}
	if (!reactors.empty())
	{
		writer.dxfString(102, "{ACAD_REACTORS");
		for (const auto& reactor : reactors)
			writer.dxfHex(330, reactor->m_Handle);
		writer.dxfString(102, "}");
	}
	writer.dxfHex(330, writer.m_CurrentBlockEntryHandle);
	writer.dxfString(100, "AcDbEntity");
	if (bInPaperSpace)
		writer.dxfInt(67, 1);
	writer.dxfString(8, m_Layer);
	if (m_Linetype != "ByLayer" && !m_Linetype.empty())
		writer.dxfString(6, m_Linetype);
	if (m_Color != acByLayer)
		writer.dxfInt(62, m_Color);
	if (m_Lineweight != acLnWtByLayer)
		writer.dxfInt(370, m_Lineweight);
	if (m_LinetypeScale != 1.0)
		writer.dxfReal(48, m_LinetypeScale);
}

AcadCircle::AcadCircle()
	: m_Radius(0)
{
}

AcadArc::AcadArc()
	: m_StartAngle(0)
	, m_EndAngle(360)
{
}

AcadAttDef::AcadAttDef()
	: m_TextHeight(0.25)
	, m_Flags(acAttributeModeNormal)
	, m_RotationAngle(0.0)
	, m_HorAlign(0)
	, m_VerAlign(0)
	, m_DuplicateFlag(1)
{
}

AcadAttrib::AcadAttrib()
	: m_TextHeight(0.25)
	, m_Flags(acAttributeModeNormal)
	, m_RotationAngle(0.0)
	, m_HorAlign(0)
	, m_VerAlign(0)
	, m_DuplicateFlag(1)
{
}

AcadBlockInstance::AcadBlockInstance()
	: m_Xscale(1), m_Yscale(1), m_Zscale(1)
	, m_RotationAngle(0)
{
}

AcadDim::AcadDim()
	: m_DimStyleName("ISO-25")
{
}

AcadDimInternal::AcadDimInternal()
	: m_ReactorHandles()
{
}

AcadDimAln::AcadDimAln()
	: m_RotationAngle(0)
{
}

AcadDimAng3P::AcadDimAng3P()
{
}

AcadDimAng::AcadDimAng()
{
}

AcadDimDia::AcadDimDia()
	: m_LeaderLength(0)
{
}

AcadDimRad::AcadDimRad()
	: m_LeaderLength(0)
{
}

AcadDimRot::AcadDimRot()
{
}

AcadDimOrd::AcadDimOrd()
{
}

AcadEllipse::AcadEllipse()
	: m_MinorAxisRatio(0)
	, m_StartAngle(0)
	, m_EndAngle(2 * M_PI)
{
}

AcadHatch::AcadHatch()
	: m_FillFlag(PatternFill)
	, m_HatchStyle(acHatchStyleNormal)
	, m_PatternName("ANSI31")
	, m_PatternType(acHatchPatternTypePreDefined)
	, m_PatternScale(1)
	, m_PatternAngle(0)
	, m_PixelSize(2.5)
	, m_OuterFlag(BoundaryPathFlag(Outer | Export))
{
}

AcadLeader::AcadLeader()
	: m_StyleName("ISO-25")
	, m_Type(acLineWithArrow)
	, m_ArrowSize(10)
	, m_HasBaseLine(true)
	, m_BaseLineDirection(false)
	, m_CharHeight(10)
	, m_TextWidth(100)
{
}

StyleMTextPart::StyleMTextPart()
	: m_TextStyle("Standard")
	, m_TextAngleType(acHorizontalAngle)
	, m_TextColor(acByBlock)
	, m_TextHeight(0.18)
	, m_AlignSpace(4.0)
{
}

ContentType StyleMTextPart::GetContentType() const
{
	return ContentType::MTEXT_TYPE;
}

StyleBlockPart::StyleBlockPart()
	: m_BlockName("_DetailCallout")
	, m_BlockConnectionType(acConnectExtents)
	, m_BlockColor(acByLayer)
	, m_BlockScale(1.0, 1.0)
{
}

ContentType StyleBlockPart::GetContentType() const
{
	return ContentType::BLOCK_TYPE;
}

MLeaderStyle::MLeaderStyle()
	: m_LeaderType(acStraightLeader)
	, m_LineColor(acByBlock)
	, m_LineType("ByBlock")
	, m_LineWeight(acLnWtByBlock)
	, m_ArrowHead() // acArrowDefault
	, m_ArrowSize(0.18)
	, m_MaxLeaderPoints(2)
	, m_FirstSegAngleConstraint(0.0f)
	, m_SecondSegAngleConstraint(0.0f)
	, m_EnableLanding(true)
	, m_IsAnnotative(false)
	, m_LandingGap(0.09)
	, m_DoglegLength(8.0)
	, m_BreakGapSize(3.75)
	, m_Content(make_shared<StyleMTextPart>())
{
}

LeaderPart::LeaderPart()
	: m_DoglegLength(8.0)
{
}

MTextPart::MTextPart()
	: m_TextWidth()
	, m_TextStyle("Standard")
	, m_TextColor(acByBlock)
	, m_TextRotation(0.0)
	, m_AttachmentPoint(acAttachmentPointTopLeft)
{
}

BlockPart::BlockPart()
	: m_BlockColor(acByLayer)
	, m_BlockScale(1.0, 1.0)
	, m_BlockRotation(0.0)
{
}

CONTEXT_DATA::CONTEXT_DATA()
	: m_LandingGap(0.09)
	, m_TextHeight(4.0)
	, m_ArrowSize(4.0)
	, m_Content(std::make_shared<MTextPart>())
{
}

AcadMLeader::AcadMLeader()
	: m_LeaderStyle("Standard")
	, m_LeaderType(acStraightLeader)
	, m_LineType("ByBlock")
	, m_LineWeight(acLnWtByBlock)
	, m_EnableLanding(true)
	, m_EnableDogleg(true)
	//, m_ArrowSize(0.18)
	//, m_TextStyle("Standard")
	//, m_DoglegLength(4.0)
{
}

CellInTableStyle::CellInTableStyle()
	: m_TextHeight(0.18)
	, m_Alignment(acMiddleCenter)
	, m_TextColor(acByBlock)
	, m_BgColor(acWhite)
	, m_BgColorEnabled(false)
	, m_CellDataType(acGeneral)
	, m_CellUnitType(acUnitless)
{
	for (int i = 0; i < 6; ++i)
	{
		m_BorderLineWeight[i] = acLnWtByBlock;
		m_BorderVisible[i] = true;
		m_BorderColor[i] = acByBlock;
	}
}

TableStyle::TableStyle()
	: m_HorCellMargin(0.06)
	, m_VerCellMargin(0.06)
	, m_HasNoTitle(false)
	, m_HasNoColumnHeading(false)
	, m_TextStyle("Standard")
{
	m_Cells.resize(3);
}

CellText::CellText()
{
}

CellType CellText::GetContentType() const
{
	return CellType::TEXT_CELL;
}

CellBlock::CellBlock()
	: m_BlockScale(1.0)
{
}

CellType CellBlock::GetContentType() const
{
	return CellType::BLOCK_CELL;
}

Cell::Cell(CellType ct)
	: m_Flag(0)
	, m_Merged(false)
	, m_AutoFit(false)
	, m_ColSpan(1)
	, m_RowSpan(1)
	, m_OverrideFlag1(acAlignmentProperty | acContentLayout | acTextHeight)
	, m_VirtualEdgeFlag(0)
	, m_Rotation(0.0)
	, m_Alignment(acInvalidAlignment)
	, m_BgColor(acByBlock)
	, m_TextColor(acByBlock)
	, m_BgColorEnabled(false)
	, m_TextHeight(0.0)
	, m_Flag93(6)
	, m_Flag90(4)
	, m_OverrideFlag2(0)
	, m_LeftBorderVisible(true)
	, m_RightBorderVisible(true)
	, m_TopBorderVisible(true)
	, m_BottomBorderVisible(true)
{
	if (ct == TEXT_CELL)
		m_Content = std::make_shared<CellText>();
	else
		m_Content = std::make_shared<CellBlock>();
}

AcadTable::AcadTable()
	: m_TableStyle("Standard")
	, m_RowCount(0)
	, m_ColCount(0)
	, m_HorCellMargin(0.0)
	, m_VerCellMargin(0.0)
	, m_LeftBorderVisible(true)
	, m_RightBorderVisible(true)
	, m_TopBorderVisible(true)
	, m_BottomBorderVisible(true)
{
}

void AcadTable::InitializeCells(int rowCount, int colCount, CellType ct)
{
	if (rowCount <= 0 || colCount <= 0)
	{
		PRINT_DEBUG_INFO("Invalid argument: rowCount = %d, colCount = %d.", rowCount, colCount);
		return;
	}
	m_RowCount = rowCount;
	m_ColCount = colCount;
	m_Cells.resize(rowCount * colCount, Cell(ct));
}

void AcadTable::SetRowHeight(double height)
{
	m_RowHeights.resize(m_RowCount, height);
}

void AcadTable::SetColWidth(double width)
{
	m_ColWidths.resize(m_ColCount, width);
}

void AcadTable::MergeCells(int row, int col, int rowSpan, int colSpan)
{
#if defined(_DEBUG)
	ASSERT_DEBUG_INFO(row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
	ASSERT_DEBUG_INFO(rowSpan >= 1 && row + rowSpan <= m_RowCount && colSpan >= 1 && col + colSpan <= m_ColCount);
	ASSERT_DEBUG_INFO(m_Cells.size() == m_RowCount * m_ColCount);
#endif
	for (int i = row; i < row + rowSpan; ++i)
	{
		for (int j = col; j < col + colSpan; ++j)
		{
			m_Cells[i * m_ColCount + j].m_Merged = true;
		}
	}
	m_Cells[row * m_ColCount + col].m_Merged = false;
}

Cell& AcadTable::GetCell(int row, int col)
{
#if defined(_DEBUG)
	ASSERT_DEBUG_INFO(row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
	ASSERT_DEBUG_INFO(m_Cells.size() == m_RowCount * m_ColCount);
#endif
	return m_Cells[row * m_ColCount + col];
}

CellText& AcadTable::GetCellText(int row, int col)
{
#if defined(_DEBUG)
	ASSERT_DEBUG_INFO(row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
	ASSERT_DEBUG_INFO(m_Cells.size() == m_RowCount * m_ColCount);
#endif
	auto& cell = m_Cells[row * m_ColCount + col];
	if (!cell.m_Content || dynamic_cast<CellBlock*>(cell.m_Content.get()))
	{
		cell.m_Content = std::make_shared<CellText>();
	}
	CellText* pText = static_cast<CellText*>(cell.m_Content.get());
	return *pText;
}

CellBlock& AcadTable::GetCellBlock(int row, int col)
{
#if defined(_DEBUG)
	ASSERT_DEBUG_INFO(row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
	ASSERT_DEBUG_INFO(m_Cells.size() == m_RowCount * m_ColCount);
#endif
	auto& cell = m_Cells[row * m_ColCount + col];
	if (!cell.m_Content || dynamic_cast<CellText*>(cell.m_Content.get()))
	{
		cell.m_Content = std::make_shared<CellBlock>();
	}
	CellBlock* pBlock = static_cast<CellBlock*>(cell.m_Content.get());
	return *pBlock;
}

AcadLine::AcadLine()
{
}

AcadLWPLine::AcadLWPLine()
	: m_Closed(false)
	, m_Width(1.0)
{
}

AcadMText::AcadMText()
	: m_Width(100)
	, m_CharHeight(0)
	, m_LineSpacingFactor(0)
	, m_RotationAngle(0)
	, m_AttachmentPoint(acAttachmentPointTopLeft)
	, m_DrawingDirection(acLeftToRight)
{
}

AcadPoint::AcadPoint()
{
}

AcadSolid::AcadSolid()
{
}

AcadSpline::AcadSpline()
	: m_Flag(8)
	, m_Degree(3)
{
}

AcadText::AcadText()
	: m_RotationAngle(0.0)
	, m_HorAlign(0)
	, m_VerAlign(0)
	, m_Height(0)
	, m_WidthFactor(0.0)
	, m_IsObliqueAngleValid(false)
	, m_ObliqueAngle(0)
{
}

AcadViewport::AcadViewport()
	: m_PaperspaceCenter(128.5, 97.5)
	, m_PaperspaceWidth(205.6)
	, m_PaperspaceHeight(156.0)
	, m_ModelSpaceCenter(100, 100)
	, m_ModelSpaceHeight(100)
	, m_TwistAngle(0)
    , m_locked(false)
{
}

AcadXLine::AcadXLine()
{
}

//#pragma region Gets
long AcadDim::GetDimensionLineColor(const DxfData& graph) const
{
	if (m_ValidMembersFlag[DIMENSIONLINECOLOR])
		return m_DimStyleOverride.DimensionLineColor;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.DimensionLineColor;
	return acByBlock;
}

long AcadDim::GetDimensionLineWeight(const DxfData& graph) const
{
	if (m_ValidMembersFlag[DIMENSIONLINEWEIGHT])
		return m_DimStyleOverride.DimensionLineWeight;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.DimensionLineWeight;
	return acLnWtByBlock;
}

bool AcadDim::GetDimLine1Suppress(const DxfData& graph) const
{
	if (m_ValidMembersFlag[DIMLINE1SUPPRESS])
		return m_DimStyleOverride.DimLine1Suppress;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.DimLine1Suppress;
	return false;
}

bool AcadDim::GetDimLine2Suppress(const DxfData& graph) const
{
	if (m_ValidMembersFlag[DIMLINE2SUPPRESS])
		return m_DimStyleOverride.DimLine2Suppress;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.DimLine2Suppress;
	return false;
}

bool AcadDim::GetForceLineInside(const DxfData& graph) const
{
	if (m_ValidMembersFlag[FORCELINEINSIDE])
		return m_DimStyleOverride.ForceLineInside;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ForceLineInside;
	return false;
}

long AcadDim::GetExtensionLineColor(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTENSIONLINECOLOR])
		return m_DimStyleOverride.ExtensionLineColor;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtensionLineColor;
	return acByBlock;
}

long AcadDim::GetExtensionLineWeight(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTENSIONLINEWEIGHT])
		return m_DimStyleOverride.ExtensionLineWeight;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtensionLineWeight;
	return acLnWtByBlock;
}

double AcadDim::GetExtensionLineExtend(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTENSIONLINEEXTEND])
		return m_DimStyleOverride.ExtensionLineExtend;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtensionLineExtend;
	return 1.25;
}

double AcadDim::GetExtensionLineOffset(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTENSIONLINEOFFSET])
		return m_DimStyleOverride.ExtensionLineOffset;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtensionLineOffset;
	return 0.625;
}

bool AcadDim::GetExtLine1Suppress(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTLINE1SUPPRESS])
		return m_DimStyleOverride.ExtLine1Suppress;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtLine1Suppress;
	return false;
}

bool AcadDim::GetExtLine2Suppress(const DxfData& graph) const
{
	if (m_ValidMembersFlag[EXTLINE2SUPPRESS])
		return m_DimStyleOverride.ExtLine2Suppress;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ExtLine2Suppress;
	return false;
}

long AcadDim::GetArrowHead1Type(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ARROWHEAD1TYPE])
		return m_DimStyleOverride.ArrowHead1Type;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ArrowHead1Type;
	return acArrowDefault;
}

long AcadDim::GetArrowHead2Type(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ARROWHEAD2TYPE])
		return m_DimStyleOverride.ArrowHead2Type;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ArrowHead2Type;
	return acArrowDefault;
}

std::string AcadDim::GetArrowHead1Block(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ARROWHEAD1TYPE])
	{
		if (m_DimStyleOverride.ArrowHead1Type == acArrowUserDefined)
			return m_DimStyleOverride.ArrowHead1Block;
		else
			return GetPredefinedArrowHeadBlockName(m_DimStyleOverride.ArrowHead1Type);
	}
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
	{
		if (it->second.ArrowHead1Type == acArrowUserDefined)
			return it->second.ArrowHead1Block;
		else
			return GetPredefinedArrowHeadBlockName(it->second.ArrowHead1Type);
	}
	return GetPredefinedArrowHeadBlockName(acArrowDefault);
}

std::string AcadDim::GetArrowHead2Block(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ARROWHEAD2TYPE])
	{
		if (m_DimStyleOverride.ArrowHead2Type == acArrowUserDefined)
			return m_DimStyleOverride.ArrowHead2Block;
		else
			return GetPredefinedArrowHeadBlockName(m_DimStyleOverride.ArrowHead2Type);
	}
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
	{
		if (it->second.ArrowHead2Type == acArrowUserDefined)
			return it->second.ArrowHead2Block;
		else
			return GetPredefinedArrowHeadBlockName(it->second.ArrowHead2Type);
	}
	return GetPredefinedArrowHeadBlockName(acArrowDefault);
}

double AcadDim::GetArrowHeadSize(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ARROWHEADSIZE])
		return m_DimStyleOverride.ArrowHeadSize;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.ArrowHeadSize;
	return 2.5;
}

long AcadDim::GetCenterType(const DxfData& graph) const
{
	if (m_ValidMembersFlag[CENTERTYPE])
		return m_DimStyleOverride.CenterType;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.CenterType;
	return acCenterMark;
}

double AcadDim::GetCenterMarkSize(const DxfData& graph) const
{
	if (m_ValidMembersFlag[CENTERMARKSIZE])
		return m_DimStyleOverride.CenterMarkSize;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.CenterMarkSize;
	return 2.5;
}

std::string AcadDim::GetTextStyle(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTSTYLE])
		return m_DimStyleOverride.TextStyle;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextStyle;
	return "Standard";
}

double AcadDim::GetTextHeight(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTHEIGHT])
		return m_DimStyleOverride.TextHeight;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextHeight;
	return 2.5;
}

long AcadDim::GetTextColor(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTCOLOR])
		return m_DimStyleOverride.TextColor;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextColor;
	return acByBlock;
}

long AcadDim::GetVerticalTextPosition(const DxfData& graph) const
{
	if (m_ValidMembersFlag[VERTICALTEXTPOSITION])
		return m_DimStyleOverride.VerticalTextPosition;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.VerticalTextPosition;
	return acAbove;
}

long AcadDim::GetHorizontalTextPosition(const DxfData& graph) const
{
	if (m_ValidMembersFlag[HORIZONTALTEXTPOSITION])
		return m_DimStyleOverride.HorizontalTextPosition;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.HorizontalTextPosition;
	return acHorzCentered;
}

bool AcadDim::GetTextAlign(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTALIGN])
		return m_DimStyleOverride.TextAlign;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextAlign;
	return false;
}

double AcadDim::GetTextGap(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTGAP])
		return m_DimStyleOverride.TextGap;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextGap;
	return 0.625;
}

std::string AcadDim::GetText(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXT])
		return m_DimStyleOverride.Text;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.Text;
	return "";
}

long AcadDim::GetFit(const DxfData& graph) const
{
	if (m_ValidMembersFlag[FIT])
		return m_DimStyleOverride.Fit;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.Fit;
	return acBestFit;
}

long AcadDim::GetTextMovement(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTMOVEMENT])
		return m_DimStyleOverride.TextMovement;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextMovement;
	return acMoveTextNoLeader;
}

bool AcadDim::GetTextInside(const DxfData& graph) const
{
	if (m_ValidMembersFlag[TEXTINSIDE])
		return m_DimStyleOverride.TextInside;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.TextInside;
	return false;
}

long AcadDim::GetUnitsFormat(const DxfData& graph) const
{
	if (m_ValidMembersFlag[UNITSFORMAT])
		return m_DimStyleOverride.UnitsFormat;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.UnitsFormat;
	return acDimLDecimal;
}

long AcadDim::GetUnitsPrecision(const DxfData& graph) const
{
	if (m_ValidMembersFlag[UNITSPRECISION])
		return m_DimStyleOverride.UnitsPrecision;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.UnitsPrecision;
	return acDimPrecisionTwo;
}

bool AcadDim::GetSuppressLeadingZeros(const DxfData& graph) const
{
	if (m_ValidMembersFlag[SUPPRESSLEADINGZEROS])
		return m_DimStyleOverride.SuppressLeadingZeros;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.SuppressLeadingZeros;
	return false;
}

bool AcadDim::GetSuppressTrailingZeros(const DxfData& graph) const
{
	if (m_ValidMembersFlag[SUPPRESSTRAILINGZEROS])
		return m_DimStyleOverride.SuppressTrailingZeros;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.SuppressTrailingZeros;
	return true;
}

bool AcadDim::GetAngleSuppressLeadingZeros(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ANGLESUPPRESSLEADINGZEROS])
		return m_DimStyleOverride.AngleSuppressLeadingZeros;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.AngleSuppressLeadingZeros;
	return false;
}

bool AcadDim::GetAngleSuppressTrailingZeros(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ANGLESUPPRESSTRAILINGZEROS])
		return m_DimStyleOverride.AngleSuppressTrailingZeros;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.AngleSuppressTrailingZeros;
	return true;
}

long AcadDim::GetAngleFormat(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ANGLEFORMAT])
		return m_DimStyleOverride.AngleFormat;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.AngleFormat;
	return long();
}

long AcadDim::GetAnglePrecision(const DxfData& graph) const
{
	if (m_ValidMembersFlag[ANGLEPRECISION])
		return m_DimStyleOverride.AnglePrecision;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.AnglePrecision;
	return acDimPrecisionOne;
}

double AcadDim::GetLinearScaleFactor(const DxfData& graph) const
{
	if (m_ValidMembersFlag[LINEARSCALEFACTOR])
		return m_DimStyleOverride.LinearScaleFactor;
	auto it = graph.m_DimStyles.find(m_DimStyleName);
	if (it != graph.m_DimStyles.end())
		return it->second.LinearScaleFactor;
	return 1.0;
}
#pragma endregion

void AcadLWPLine::SetWidth(size_t Index, double startWidth, double endWidth)
{
	ASSERT_DEBUG_INFO(Index < m_Vertices.size());
	size_t count = m_startWidths.size();
    if (count < Index + 1)
    {
        m_startWidths.resize(size_t(Index + 1));
        m_endWidths.resize(size_t(Index + 1));
    }
    m_startWidths[Index] = startWidth;
    m_endWidths[Index] = endWidth;
}

void AcadLWPLine::SetStartWidth(size_t Index, double startWidth)
{
    ASSERT_DEBUG_INFO(Index < m_Vertices.size());
    size_t count = m_startWidths.size();
    if (count < Index + 1)
    {
        m_startWidths.resize(size_t(Index + 1));
        m_endWidths.resize(size_t(Index + 1));
    }
    m_startWidths[Index] = startWidth;
}

void AcadLWPLine::SetEndWidth(size_t Index, double endWidth)
{
    ASSERT_DEBUG_INFO(Index < m_Vertices.size());
    size_t count = m_startWidths.size();
    if (count < Index + 1)
    {
        m_startWidths.resize(size_t(Index + 1));
        m_endWidths.resize(size_t(Index + 1));
    }
    m_endWidths[Index] = endWidth;
}

void AcadLWPLine::SetWidths(const std::vector<double>& startWidths, const std::vector<double>& endWidths)
{
    m_startWidths = startWidths;
    m_endWidths = endWidths;
}

void AcadLWPLine::SetBulge(size_t Index, double Bulge)
{
	ASSERT_DEBUG_INFO(Index < m_Vertices.size());
	size_t count = m_Bulges.size();
	if (count < Index + 1)
		m_Bulges.resize(size_t(Index + 1));
	m_Bulges[Index] = Bulge;
}

void AcadLWPLine::PushBulge(double bulge)
{
	m_Bulges.push_back(bulge);
}

double AcadLWPLine::GetBulge(size_t Index) const
{
	size_t count = m_Vertices.size();
	ASSERT_DEBUG_INFO(Index < count);
	if (Index < m_Bulges.size())
		return m_Bulges[Index];
	return 0.0;
}

EntityList::iterator EntityList::FindEntity(int handle)
{
	return std::find_if(begin(), end(), [handle](const std::shared_ptr<EntAttribute>& pEnt) {
		return pEnt->m_Handle == handle;
	});
}

std::shared_ptr<AcadLWPLine> convert(const AcadCircle& c)
{
	auto lwp = std::make_shared<AcadLWPLine>();
	lwp->m_Vertices.push_back(CDblPoint(c.m_Center.x - c.m_Radius, c.m_Center.y));
	lwp->m_Vertices.push_back(CDblPoint(c.m_Center.x + c.m_Radius, c.m_Center.y));
	lwp->PushBulge(1.0);
	lwp->PushBulge(1.0);
	lwp->m_Closed = true;
	return lwp;
}

bool AcadHatch::AddEntity(const std::shared_ptr<EntAttribute>& pEnt, int loopIndex)
{
	ASSERT_DEBUG_INFO(pEnt != nullptr);
	ASSERT_DEBUG_INFO(loopIndex >= -1);
	auto pC = dynamic_pointer_cast<AcadCircle>(pEnt);
	std::shared_ptr<EntAttribute> pEnt2;
	if (pC)
	{
		pEnt2 = convert(*pC);
		if (loopIndex == -1)
			m_OuterFlag = BoundaryPathFlag(Outer | LWPline | Export);
		else
		{
			if (int(m_InnerFlags.size()) <= loopIndex)
			{
				m_InnerFlags.resize(loopIndex + 1);
			}
			m_InnerFlags[loopIndex] = BoundaryPathFlag(LWPline);
		}
	}
	else
	{
		pEnt2 = pEnt;
	}

	if (loopIndex == -1)
	{
		m_OuterLoop.push_back(pEnt2);
	}
	else
	{
		size_t origSize = m_InnerLoops.size();
		if (origSize <= size_t(loopIndex))
		{
			m_InnerLoops.resize(loopIndex + 1);
			m_InnerAssociatedEntities.resize(loopIndex + 1);
			for (size_t i = origSize; i <= size_t(loopIndex); ++i)
			{
				if (m_InnerAssociatedEntities[i] == nullptr)
				{
					m_InnerAssociatedEntities[i] = std::make_shared<std::vector<std::weak_ptr<EntAttribute>>>();
				}
				m_InnerLoops[i] = std::make_shared<EntityList>();
			}
		}
		m_InnerLoops[loopIndex]->push_back(pEnt2);
	}
	return true;
}

bool AcadHatch::AddAssociatedEntity(const std::shared_ptr<EntAttribute>& pEnt, int loopIndex)
{
	ASSERT_DEBUG_INFO(pEnt != nullptr);
	ASSERT_DEBUG_INFO(loopIndex >= -1);

	if (loopIndex == -1)
	{
		m_OuterAssociatedEntities.push_back(pEnt);
	}
	else
	{
		size_t origSize = m_InnerAssociatedEntities.size();
		if (origSize <= size_t(loopIndex))
		{
			m_InnerAssociatedEntities.resize(loopIndex + 1);
			for (size_t i = origSize; i <= size_t(loopIndex); ++i)
			{
				m_InnerAssociatedEntities[i] = std::make_shared<std::vector<std::weak_ptr<EntAttribute>>>();
			}
		}
		m_InnerAssociatedEntities[loopIndex]->push_back(pEnt);
	}
	return true;
}

void AcadLWPLine::SetBulges(std::vector<double> bulges)
{
	m_Bulges.swap(bulges);
}

void AcadCircle::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "CIRCLE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbCircle");
	writer.coord(10, m_Center.x, m_Center.y);
	writer.dxfReal(40, m_Radius);
}

void AcadArc::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "ARC");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbCircle");
	writer.coord(10, m_Center.x, m_Center.y);
	writer.dxfReal(40, m_Radius);
	writer.dxfString(100, "AcDbArc");
	writer.dxfReal(50, m_StartAngle);
	writer.dxfReal(51, m_EndAngle);
}

void AcadAttDef::WriteDxf(DxfWriter & writer, bool bInPaperSpace)
{
	writer.dxfString(0, "ATTDEF");

	WriteAttribute(writer, bInPaperSpace);

	writer.dxfString(100, "AcDbText");
	if (m_HorAlign == 0 && m_VerAlign == 0)
		writer.coord(10, &m_BaseLeftPoint.x);
	else
		writer.coord(11, &m_InsertionPoint.x);
	writer.dxfReal(40, m_TextHeight);
	writer.dxfString(1, m_Text);
	if (m_RotationAngle != 0.0)
		writer.dxfReal(50, m_RotationAngle);
	if (!m_TextStyle.empty())
		writer.dxfString(7, m_TextStyle);
	if (m_HorAlign != 0)
		writer.dxfInt(72, m_HorAlign);
	writer.dxfString(100, "AcDbAttributeDefinition");
	writer.dxfInt(280, 0/*ACAD 2010*/);
	writer.dxfString(3, m_Prompt);
	writer.dxfString(2, m_Tag);
	writer.dxfInt(70, m_Flags);
	if (m_VerAlign != 0)
		writer.dxfInt(74, m_VerAlign);
	writer.dxfInt(280, m_DuplicateFlag);
}

void AcadAttrib::WriteDxf(DxfWriter & writer, bool bInPaperSpace)
{
	// Not used.
	ASSERT_DEBUG_INFO(false);
}

void AcadAttrib::WriteDxf(DxfWriter & writer, bool bInPaperSpace, int parentHandle)
{
	writer.dxfString(0, "ATTRIB");

	//WriteAttribute(writer, bInPaperSpace);
	writer.dxfHex(5, m_Handle);
	writer.dxfHex(330, parentHandle);
	writer.dxfString(100, "AcDbEntity");
	if (bInPaperSpace)
		writer.dxfInt(67, 1);
	writer.dxfString(8, m_Layer);

	writer.dxfString(100, "AcDbText");
	if (m_HorAlign == 0 && m_VerAlign == 0)
		writer.coord(10, &m_BaseLeftPoint.x);
	else
		writer.coord(11, &m_InsertionPoint.x);
	writer.dxfReal(40, m_TextHeight);
	writer.dxfString(1, m_Text);
	if (m_RotationAngle != 0.0)
		writer.dxfReal(50, m_RotationAngle);
	if (!m_TextStyle.empty())
		writer.dxfString(7, m_TextStyle);
	if (m_HorAlign != 0)
		writer.dxfInt(72, m_HorAlign);
	writer.dxfString(100, "AcDbAttribute");
	writer.dxfInt(280, 0/*ACAD 2010*/);
	writer.dxfString(2, m_Tag);
	writer.dxfInt(70, m_Flags);
	if (m_VerAlign != 0)
		writer.dxfInt(74, m_VerAlign);
	writer.dxfInt(280, m_DuplicateFlag);
}

void AcadBlockInstance::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "INSERT");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbBlockReference");
	if (!m_Attribs.empty())
		writer.dxfInt(66, 1);
	writer.dxfString(2, m_Name);
	writer.coord(10, m_InsertionPoint.x, m_InsertionPoint.y);
	writer.dxfReal(41, m_Xscale);
	writer.dxfReal(42, m_Yscale);
	writer.dxfReal(43, m_Zscale);
	if (m_RotationAngle != 0.0)
		writer.dxfReal(50, m_RotationAngle);
	
	if (m_Attribs.empty())
		return;
	for (auto& attrib : m_Attribs)
		attrib.WriteDxf(writer, bInPaperSpace, m_Handle);

	// SEQEND
	writer.dxfString(0, "SEQEND");
	writer.dxfHex(5, m_Attribs.back().m_Handle + 1);
	writer.dxfHex(330, m_Handle);
	writer.dxfString(100, "AcDbEntity");
	writer.dxfString(8, m_Layer);
}

void AcadBlockInstance::AssignHandle(int & handle)
{
	m_Handle = ++handle;
	if (m_Attribs.empty())
		return;
	for (auto& attrib : m_Attribs)
	{
		attrib.m_Handle = ++handle;
	}
	// Handle for SEQEND
	++handle;
}

void AcadDim::WriteHeader(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "DIMENSION");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbDimension");
	writer.dxfInt(280, 0); // 版本号：0 = 2010
	if (!m_BlockName.empty())
		writer.dxfString(2, m_BlockName);
	writer.coord(10, m_DefPoint.x, m_DefPoint.y);
	writer.coord(11, m_TextPosition.x, m_TextPosition.y);
}

void AcadDim::WriteOverride(DxfWriter& writer)
{
	if (m_ValidMembersFlag[TEXT])
		writer.dxfString(1, m_DimStyleOverride.Text);
	writer.dxfString(3, m_DimStyleName);
	if (m_ValidMembersFlag[EXTENSIONLINEOFFSET])
	{
		writer.dxfInt(1070, 42);
		writer.dxfReal(1040, m_DimStyleOverride.ExtensionLineOffset);
	}
	if (m_ValidMembersFlag[HORIZONTALTEXTPOSITION])
	{
		writer.dxfInt(1070, 280);
		writer.dxfInt(1070, m_DimStyleOverride.HorizontalTextPosition);
	}
	if (m_ValidMembersFlag[DIMENSIONLINECOLOR])
	{
		writer.dxfInt(1070, 176);
		writer.dxfInt(1070, m_DimStyleOverride.DimensionLineColor);
	}
	if (m_ValidMembersFlag[DIMENSIONLINEWEIGHT])
	{
		writer.dxfInt(1070, 371);
		writer.dxfInt(1070, m_DimStyleOverride.DimensionLineWeight);
	}
	if (m_ValidMembersFlag[ARROWHEADSIZE])
	{
		writer.dxfInt(1070, 41);
		writer.dxfReal(1040, m_DimStyleOverride.ArrowHeadSize);
	}
	if (m_ValidMembersFlag[TEXTHEIGHT])
	{
		writer.dxfInt(1070, 140);
		writer.dxfReal(1040, m_DimStyleOverride.TextHeight);
	}
	if (m_ValidMembersFlag[TEXTSTYLE])
	{
		writer.dxfInt(1070, 340);
		writer.dxfHex(1005, writer.LookupTextStyleHandle(m_DimStyleOverride.TextStyle));
	}
	if (m_ValidMembersFlag[TEXTCOLOR])
	{
		writer.dxfInt(1070, 178);
		writer.dxfInt(1070, m_DimStyleOverride.TextColor);
	}
	if (m_ValidMembersFlag[VERTICALTEXTPOSITION])
	{
		writer.dxfInt(1070, 145);
		writer.dxfInt(1070, m_DimStyleOverride.VerticalTextPosition);
	}
	if (m_ValidMembersFlag[TEXTALIGN])
	{
		writer.dxfInt(1070, 73);
		writer.dxfInt(1070, m_DimStyleOverride.TextAlign ? 1 : 0);
		writer.dxfInt(1070, 74);
		writer.dxfInt(1070, m_DimStyleOverride.TextAlign ? 1 : 0);
	}
	if (m_ValidMembersFlag[TEXTGAP])
	{
		writer.dxfInt(1070, 147);
		writer.dxfReal(1040, m_DimStyleOverride.TextGap);
	}
	if (m_ValidMembersFlag[FIT])
	{
		writer.dxfInt(1070, 289);
		writer.dxfInt(1070, m_DimStyleOverride.Fit);
	}
	if (m_ValidMembersFlag[TEXTINSIDE])
	{
		writer.dxfInt(1070, 174);
		writer.dxfInt(1070, m_DimStyleOverride.TextInside ? 1 : 0);
	}
	if (m_ValidMembersFlag[FORCELINEINSIDE])
	{
		writer.dxfInt(1070, 172);
		writer.dxfInt(1070, m_DimStyleOverride.ForceLineInside ? 1 : 0);
	}
	if (m_ValidMembersFlag[SUPPRESSLEADINGZEROS] ||
		m_ValidMembersFlag[SUPPRESSTRAILINGZEROS])
	{
		int flag = 0;
		if (m_DimStyleOverride.SuppressLeadingZeros)
			flag |= 1;
		if (m_DimStyleOverride.SuppressTrailingZeros)
			flag |= 2;
		if (dynamic_cast<AcadDimAng*>(this))
		{
			writer.dxfInt(1070, 79);
			writer.dxfInt(1070, flag);
		}
		else
		{
			flag <<= 2;
			writer.dxfInt(1070, 78);
			writer.dxfInt(1070, flag);
		}
	}
	if (m_ValidMembersFlag[DIMLINE1SUPPRESS])
	{
		writer.dxfInt(1070, 281);
		writer.dxfInt(1070, m_DimStyleOverride.DimLine1Suppress ? 1 : 0);
	}
	if (m_ValidMembersFlag[DIMLINE2SUPPRESS])
	{
		writer.dxfInt(1070, 282);
		writer.dxfInt(1070, m_DimStyleOverride.DimLine2Suppress ? 1 : 0);
	}
	if (m_ValidMembersFlag[TEXTROTATION])
		writer.dxfReal(53, m_TextRotation);
}

void AcadDimAln::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 1 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDbAlignedDimension");
	writer.coord(13, m_ExtLine1Point.x, m_ExtLine1Point.y);
	writer.coord(14, m_ExtLine2Point.x, m_ExtLine2Point.y);
	if (m_RotationAngle != 0.0)
		writer.dxfReal(50, m_RotationAngle);
}

void AcadDimAng3P::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 5 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDb3PointAngularDimension");
	writer.coord(13, m_FirstEnd.x, m_FirstEnd.y);
	writer.coord(14, m_SecondEnd.x, m_SecondEnd.y);
	writer.coord(15, m_AngleVertex.x, m_AngleVertex.y);
}

void AcadDimAng::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 2 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDb2LineAngularDimension");
	writer.coord(13, m_FirstStart.x, m_FirstStart.y);
	writer.coord(14, m_FirstEnd.x, m_FirstEnd.y);
	writer.coord(15, m_SecondStart.x, m_SecondStart.y);
}

void AcadDimDia::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 3 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDbDiametricDimension");
	writer.coord(15, m_ChordPoint.x, m_ChordPoint.y);
	writer.dxfReal(40, m_LeaderLength);
}

void AcadDimRad::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 4 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDbRadialDimension");
	writer.coord(15, m_ChordPoint.x, m_ChordPoint.y);
	writer.dxfReal(40, m_LeaderLength);
}

void AcadDimRot::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 0 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDbAlignedDimension");
	writer.coord(13, m_ExtLine1Point.x, m_ExtLine1Point.y);
	writer.coord(14, m_ExtLine2Point.x, m_ExtLine2Point.y);
	writer.dxfReal(50, m_RotationAngle);
	writer.dxfString(100, "AcDbRotatedDimension");
}

void AcadDimOrd::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	WriteHeader(writer, bInPaperSpace);
	WriteOverride(writer);
	int type = 6 | 0x20;
	writer.dxfInt(70, type);
	writer.dxfInt(71, 5);//文字正中对齐

	writer.dxfString(100, "AcDbOrdinateDimension");
	writer.coord(13, m_OrdPoint.x, m_OrdPoint.y);
	writer.coord(14, m_LeaderPoint.x, m_LeaderPoint.y);
}

void AcadEllipse::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "ELLIPSE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbEllipse");
	writer.coord(10, m_Center.x, m_Center.y);
	double radiusRatio = m_MinorAxisRatio;
	double dx = m_MajorAxisPoint.x;
	double dy = m_MajorAxisPoint.y;
	if (radiusRatio > 1.0)
	{
		double temp = dx;
		dx = -dy * radiusRatio;
		dy = temp * radiusRatio;
		radiusRatio = 1.0 / radiusRatio;
	}
	writer.coord(11, dx, dy);
	writer.dxfReal(40, radiusRatio);
	writer.dxfReal(41, m_StartAngle);
	writer.dxfReal(42, m_EndAngle);
}

bool AcadHatch::HasAssociatedEntity() const
{
	for (auto weakPtr : m_OuterAssociatedEntities)
	{
		if (!weakPtr.expired())
			return true;
	}
	for (auto innerLoop : m_InnerAssociatedEntities)
	{
		for (auto weakPtr : *innerLoop)
		{
			if (!weakPtr.expired())
				return true;
		}
	}
	return false;
}

void AcadHatch::WriteLoop(DxfWriter& writer, int loopIndex)
{
	const EntityList* pBoundaryData = nullptr;
	const std::vector<std::weak_ptr<EntAttribute>>* pSourceHandles = nullptr;
	if (loopIndex == -1)
	{
		pBoundaryData = &m_OuterLoop;
		pSourceHandles = &m_OuterAssociatedEntities;
	}
	else
	{
		pBoundaryData = m_InnerLoops[loopIndex].get();
		pSourceHandles = m_InnerAssociatedEntities[loopIndex].get();
	}
	ASSERT_DEBUG_INFO(pBoundaryData && !pBoundaryData->empty());
		
	do
	{
		int flag = 0;
		if (loopIndex == -1)
			flag = m_OuterFlag;
		else
			flag = m_InnerFlags[loopIndex];
		// pBoundaryData->size() == 1 && pBoundaryData[0] is closed AcadLWPLine
		if (pBoundaryData->size() == 1)
		{
			const auto& pEnt = pBoundaryData->at(0);
			auto pLWPLine = dynamic_cast<AcadLWPLine*>(pEnt.get());
			if (pLWPLine && pLWPLine->m_Closed)
			{
				writer.dxfInt(92, flag | LWPline);
				writer.dxfInt(72, pLWPLine->HasBulges() ? 1 : 0);
				writer.dxfInt(73, 1);
				writer.dxfInt(93, int(pLWPLine->m_Vertices.size()));
				for (size_t i = 0; i < pLWPLine->m_Vertices.size(); ++i)
				{
					writer.dxfReal(10, pLWPLine->m_Vertices[i].x);
					writer.dxfReal(20, pLWPLine->m_Vertices[i].y);
					if (pLWPLine->HasBulges())
						writer.dxfReal(42, pLWPLine->GetBulge(i));
				}
				break;
			}
		}
		// pBoundaryData->size() > 1 or pBoundaryData[0] is not closed AcadLWPLine
		{
			int edgeNum = int(pBoundaryData->size());
			writer.dxfInt(92, flag);
			writer.dxfInt(93, edgeNum);
			for (const auto& pEnt : *pBoundaryData)
			{
				auto pLine = dynamic_cast<AcadLine*>(pEnt.get());
				if (pLine)
				{
					writer.dxfInt(72, 1);
					writer.dxfReal(10, pLine->m_StartPoint.x);
					writer.dxfReal(20, pLine->m_StartPoint.y);
					writer.dxfReal(11, pLine->m_EndPoint.x);
					writer.dxfReal(21, pLine->m_EndPoint.y);
					continue;
				}

				auto pCircle = dynamic_cast<AcadCircle*>(pEnt.get());
				if (pCircle)
				{
					PRINT_DEBUG_INFO("Error: Circle isn't supported by DXF. It should be convert to LWPline. See convert function.");
					throw std::logic_error("Program error!");
					// Circle isn't supported by DXF. It should be convert to LWPline.
					//writer.dxfInt(72, 2);
					//writer.dxfReal(10, pCircle->m_Center.x);
					//writer.dxfReal(20, pCircle->m_Center.y);
					//writer.dxfReal(40, pCircle->m_Radius);
					//auto pArc = dynamic_cast<AcadArc*>(pEnt.get());
					//if (pArc)
					//{
					//	writer.dxfReal(50, pArc->m_StartAngle);
					//	writer.dxfReal(51, pArc->m_EndAngle);
					//}
					//else
					//{
					//	writer.dxfReal(50, 0);
					//	writer.dxfReal(51, 360);
					//}
					//writer.dxfInt(73, 1);
					continue;
				}

				auto pEllipse = dynamic_cast<AcadEllipse*>(pEnt.get());
				if (pEllipse)
				{
					writer.dxfInt(72, 3);
					writer.dxfReal(10, pEllipse->m_Center.x);
					writer.dxfReal(20, pEllipse->m_Center.y);
					writer.dxfReal(11, pEllipse->m_MajorAxisPoint.x);
					writer.dxfReal(21, pEllipse->m_MajorAxisPoint.x);
					writer.dxfReal(40, pEllipse->m_MinorAxisRatio);
					writer.dxfReal(50, pEllipse->m_StartAngle);
					writer.dxfReal(51, pEllipse->m_EndAngle);
					writer.dxfInt(73, 1);
					continue;
				}

				auto pSpline = dynamic_cast<AcadSpline*>(pEnt.get());
				if (pSpline)
				{
					writer.dxfInt(72, 4);
					writer.dxfInt(94, pSpline->m_Degree);
					writer.dxfInt(73, (pSpline->m_Flag & 4) ? 1 : 0);
					writer.dxfInt(74, (pSpline->m_Flag & 2) ? 1 : 0);

					writer.dxfInt(95, int(pSpline->m_Knots.size()));
					writer.dxfInt(96, int(pSpline->m_ControlPoints.size()));
					for (double knot : pSpline->m_Knots)
					{
						writer.dxfReal(40, knot);
					}

					for (const auto& cp : pSpline->m_ControlPoints)
					{
						writer.dxfReal(10, cp.x);
						writer.dxfReal(20, cp.y);
					}

					// m_Weights ?

					writer.dxfInt(97, int(pSpline->m_FitPoints.size()));
					for (const auto& fp : pSpline->m_FitPoints)
					{
						writer.dxfReal(11, fp.x);
						writer.dxfReal(21, fp.y);
					}

					writer.dxfReal(12, pSpline->m_StartTangent.x);
					writer.dxfReal(22, pSpline->m_StartTangent.y);
					writer.dxfReal(13, pSpline->m_EndTangent.x);
					writer.dxfReal(23, pSpline->m_EndTangent.y);
					continue;
				}

				// If execution reaches here, it is client programmer's error.
				ASSERT_DEBUG_INFO(false);
			}
		}
	} while (false);

	std::vector<std::shared_ptr<EntAttribute>> validSourceHandles;
	for (const auto& sourceHandle : *pSourceHandles)
	{
		auto validSourceHandle = sourceHandle.lock();
		if (validSourceHandle)
			validSourceHandles.push_back(validSourceHandle);
	}
	writer.dxfInt(97, int(validSourceHandles.size()));
	for (const auto& validSourceHandle : validSourceHandles)
	{
		writer.dxfHex(330, validSourceHandle->m_Handle);
	}
}

void AcadHatch::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	if (m_OuterLoop.empty())
		return;

	writer.dxfString(0, "HATCH");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbHatch");
	writer.coord(10, 0, 0);
	writer.dxfReal(210, 0);
	writer.dxfReal(220, 0);
	writer.dxfReal(230, 1);
	std::string PatternName(m_PatternName);
	if (m_HatchStyle == acHatchStyleOuter)
		PatternName += ",_O";
	else if (m_HatchStyle == acHatchStyleIgnore)
		PatternName += ",_I";
	writer.dxfString(2, PatternName);
	bool bSolid = SolidFill == m_FillFlag || _stricmp(m_PatternName.c_str(), "SOLID") == 0;
	writer.dxfInt(70, bSolid);
	bool associativity = HasAssociatedEntity();
	writer.dxfInt(71, associativity ? 1 : 0);
	//环数
	writer.dxfInt(91, int(m_InnerLoops.size() + 1));
	//边界路径数据
	WriteLoop(writer, -1);
	for (size_t i = 0; i < m_InnerLoops.size(); ++i)
	{
		WriteLoop(writer, int(i));
	}
	writer.dxfInt(75, m_HatchStyle);
	writer.dxfInt(76, m_PatternType);
	if (!bSolid)
	{
		writer.dxfReal(52, m_PatternAngle);
		writer.dxfReal(41, m_PatternScale);
		writer.dxfInt(77, 0);
		Pattern* pPat = writer.FindPattern(m_PatternName.c_str());
		if (pPat)
		{
			writer.dxfInt(78, int(pPat->dashlines.size()));
			for (size_t i = 0; i < pPat->dashlines.size(); ++i)
			{
				writer.dxfReal(53, pPat->dashlines[i]->angle + m_PatternAngle);
				CDblPoint origin(pPat->dashlines[i]->x_origin, pPat->dashlines[i]->y_origin);
				origin.x *= m_PatternScale;
				origin.y *= m_PatternScale;
				const CDblPoint zero;
				zero.Rotate(origin, m_PatternAngle * M_PI / 180.0);
				writer.dxfReal(43, origin.x);
				writer.dxfReal(44, origin.y);
				double angle = (pPat->dashlines[i]->angle + m_PatternAngle) * M_PI / 180.0;
				CDblPoint point(pPat->dashlines[i]->delta_x, pPat->dashlines[i]->delta_y);
				point.x *= m_PatternScale;
				point.y *= m_PatternScale;
				zero.Rotate(point, angle);
				writer.dxfReal(45, point.x);
				writer.dxfReal(46, point.y);
				writer.dxfInt(79, int(pPat->dashlines[i]->dashes.size()));
				for (size_t j = 0; j<pPat->dashlines[i]->dashes.size(); ++j)
					writer.dxfReal(49, pPat->dashlines[i]->dashes[j] * m_PatternScale);
			}
		}
		else
		{
			PRINT_DEBUG_INFO("Can not find pattern %s", m_PatternName.c_str());
		}
	}
	writer.dxfReal(47, m_PixelSize);
	writer.dxfInt(98, 1);
	writer.dxfReal(10, 0);
	writer.dxfReal(20, 0);
}

void AcadLeader::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "LEADER");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbLeader");
	writer.dxfString(3, m_StyleName);
	writer.dxfInt(71, m_Type & 0x2);
	writer.dxfInt(72, m_Type & 0x1);
	const auto& annotation = m_Annotation.lock();
	int val73 = 3;
	if (std::dynamic_pointer_cast<AcadMText>(annotation))
	{
		val73 = 0;
	}
	else if(std::dynamic_pointer_cast<AcadBlockInstance>(annotation))
	{
		val73 = 2;
	}
	writer.dxfInt(73, val73);
	writer.dxfInt(74, m_BaseLineDirection ? 1 : 0);
	writer.dxfInt(75, m_HasBaseLine ? 1 : 0);
	writer.dxfReal(40, m_CharHeight);
	writer.dxfReal(41, m_TextWidth);
	writer.dxfInt(76, int(m_Vertices.size()));
	for (const auto& vp : m_Vertices)
	{
		writer.dxfReal(10, vp.x);
		writer.dxfReal(20, vp.y);
	}
	if (annotation && annotation->m_Handle != 0)
	{
		writer.dxfHex(340, annotation->m_Handle);
	}
	if (m_ArrowSize != 0)
	{
		writer.dxfString(1001, "ACAD");
		writer.dxfString(1000, "DSTYLE");
		writer.dxfString(1002, "{");
		writer.dxfInt(1070, 41);
		writer.dxfReal(1040, m_ArrowSize);
		writer.dxfString(1002, "}");
	}
}

void StyleMTextPart::WriteDxf(DxfWriter & writer)
{
	writer.dxfHex(342, writer.LookupTextStyleHandle(m_TextStyle));
	writer.dxfInt(174, 1); // Text Left Attachment Type
	writer.dxfInt(178, 1); // ext Right Attachment Type
	writer.dxfInt(175, m_TextAngleType);
	writer.dxfInt(176, 0); // Text Alignment Type
	writer.dxfInt(93, m_TextColor);
	writer.dxfReal(45, m_TextHeight);
	writer.dxfInt(292, 0); // Enable Frame Text
	writer.dxfInt(297, 0); // Text Align Always Left
	writer.dxfReal(46, m_AlignSpace); // Align Space
}

void StyleBlockPart::WriteDxf(DxfWriter & writer)
{
	//CDblPoint m_BlockScale; /*142*/
	writer.dxfHex(343, writer.LookupBlockEntryHandle(m_BlockName));
	writer.dxfInt(94, m_BlockColor);
	writer.dxfReal(47, m_BlockScale.x);
	writer.dxfReal(49, m_BlockScale.y);
	writer.dxfReal(140, 1.0); // Block Content Scale on Z-axis
	writer.dxfInt(293, 1); // Enable Block Content Scale
	writer.dxfReal(141, 0.0); // Block Content Rotation
	writer.dxfInt(294, 1); // Enable Block Content Rotation
	writer.dxfInt(177, m_BlockConnectionType);
	writer.dxfReal(142, 1.0); // Scale
	writer.dxfInt(295, 1); // Overwrite Property Value
}

void MLeaderStyle::WriteDxf(DxfWriter& writer, const std::string& name, int handle)
{
	ASSERT_DEBUG_INFO(!name.empty() && m_Content);
	writer.dxfString(0, "MLEADERSTYLE");
	writer.dxfHex(5, handle);
	writer.dxfString(102, "{ACAD_REACTORS");
	writer.dxfHex(330, 0x12D); // ACAD_MLEADERSTYLE dictionary handle
	const auto& mls = writer.GetMLeaders(name);
	for (const auto* ml : mls)
	{
		writer.dxfHex(330, ml->m_Handle);
	}
	writer.dxfString(102, "}");
	writer.dxfHex(330, 0x12D);
	writer.dxfString(100, "AcDbMLeaderStyle");

	writer.dxfInt(179, 2); // ???
	writer.dxfInt(170, m_Content->GetContentType());
	writer.dxfInt(171, 1); // DrawMLeaderOrder Type
	writer.dxfInt(172, 0); // DrawLeaderOrder Type
	writer.dxfInt(90, m_MaxLeaderPoints);
	writer.dxfReal(40, m_FirstSegAngleConstraint);
	writer.dxfReal(41, m_SecondSegAngleConstraint);
	writer.dxfInt(173, m_LeaderType);
	writer.dxfInt(91, m_LineColor);
	writer.dxfHex(340, writer.LookupLinetypeHandle(m_LineType));
	writer.dxfInt(92, m_LineWeight);
	writer.dxfInt(290, m_EnableLanding);
	writer.dxfReal(42, m_LandingGap);
	writer.dxfInt(291, m_EnableDogleg ? 1 : 0);
	writer.dxfReal(43, m_DoglegLength);
	writer.dxfString(3, name); // Mleader Style Description
	if (!m_ArrowHead.empty())
	{
		writer.dxfHex(341, writer.LookupBlockEntryHandle(m_ArrowHead));
	}
	writer.dxfReal(44, m_ArrowSize);
	writer.dxfString(300, ""); // Default MText Contents

	m_Content->WriteDxf(writer);

	writer.dxfInt(295, 0);
	writer.dxfInt(296, m_IsAnnotative);
	writer.dxfReal(143, m_BreakGapSize);
	writer.dxfInt(271, 0); // Text attachment direction for MText contents: 0 = Horizontal, 1 = Vertical
	writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
	writer.dxfInt(273, 9); // Top text attachment direction:	9 = Center, 10 = Overline and Center
}

void LeaderPart::WriteDxf(DxfWriter & writer)
{
	writer.dxfString(302, "LEADER{");
	writer.dxfInt(290, 1); // Has Set Last Leader Line Point
	writer.dxfInt(291, 1); // Has Set Dogleg Vector
	writer.coord(10, m_CommonPoint.x, m_CommonPoint.y);
	writer.coord(11, 1.0, 0.0); // Dogleg Vector
	writer.dxfInt(90, 0); // Leader Branch Index
	writer.dxfReal(40, m_DoglegLength); // Dogleg Length
	int i = -1;
	for (const auto& ll : m_Lines)
	{
		writer.dxfString(304, "LEADER_LINE{");
		for (const auto& p : ll.m_Points)
		{
			writer.coord(10, p.x, p.y);
		}
		writer.dxfInt(91, ++i);
		writer.dxfString(305, "}");
	}
	writer.dxfInt(271, 0); // ???
	writer.dxfString(303, "}");
}

void MTextPart::WriteDxf(DxfWriter & writer)
{
	writer.dxfInt(174, 1); // Text Angle Type
	writer.dxfInt(175, 1); // Text Alignment Type
	writer.dxfInt(176, 0); // Block Content Connection Type
	writer.dxfInt(177, 0); // Block Attribute Index

	writer.dxfInt(290, 1); // hasMText
	writer.dxfString(304, m_Text);
	writer.dxfReal(11, 0.0); // Text Normal Direction
	writer.dxfReal(21, 0.0);
	writer.dxfReal(31, 1.0);
	writer.dxfHex(340, writer.LookupTextStyleHandle(m_TextStyle));
	writer.coord(12, m_TextLocation.x, m_TextLocation.y);
	writer.coord(13, cos(m_TextRotation), sin(m_TextRotation)); // Text Direction
	writer.dxfReal(42, m_TextRotation);
	writer.dxfReal(43, m_TextWidth);
	writer.dxfReal(44, 0.0); // Text Height, same as 41, don't know why.
	writer.dxfReal(45, 1.0); // Text Line Spacing Factor
	writer.dxfInt(170, 1); // Text Line Spacing Style
	writer.dxfInt(90, m_TextColor);
	writer.dxfInt(171, m_AttachmentPoint);
	writer.dxfInt(172, 5); // Text Flow Direction
	writer.dxfInt(91, -1073741824); // Text Background Color
	writer.dxfReal(141, 0.0);
	writer.dxfInt(92, 0); // Text Background Transparency
	writer.dxfInt(291, 0); // Is Text Background Color On
	writer.dxfInt(292, 0); // Is Text Background Fill On
	writer.dxfInt(173, 0); // Text Column Type
	writer.dxfInt(293, 0); // Use Text Auto Height
	writer.dxfReal(142, 0.0); // Text Column Width
	writer.dxfReal(143, 0.0); // Text Column Gutter Width
	writer.dxfInt(294, 0); // Text Column Flow Reversed
	writer.dxfInt(295, 0); // Text Use Word Break
	writer.dxfInt(296, 0); // HasBlock
}

ContentType MTextPart::GetContentType() const
{
	return ContentType::MTEXT_TYPE;
}

void BlockPart::WriteDxf(DxfWriter & writer)
{
	writer.dxfInt(174, 1); // Text Angle Type
	writer.dxfInt(175, 1); // Text Alignment Type
	writer.dxfInt(176, 0); // Block Content Connection Type
	writer.dxfInt(177, 0); // Block Attribute Index

	writer.dxfInt(290, 0); // hasMText
	writer.dxfInt(296, 1); // HasBlock
	writer.dxfHex(341, writer.LookupBlockEntryHandle(m_BlockName));
	writer.dxfReal(14, 0.0); // Block Content Normal Direction
	writer.dxfReal(24, 0.0);
	writer.dxfReal(34, 1.0);
	writer.coord(15, m_BlockPosition.x, m_BlockPosition.y);
	writer.coord(16, m_BlockScale.x, m_BlockScale.y);
	writer.dxfReal(46, m_BlockRotation);
	writer.dxfInt(93, m_BlockColor);
	double matrix[16] = { 1,0,0,m_BlockPosition.x,
						  0,1,0,m_BlockPosition.y,
						  0,0,1,0,
						  0,0,0,1 };
	for (int i = 0; i < 16; ++i)
		writer.dxfReal(47, matrix[i]);
}

ContentType BlockPart::GetContentType() const
{
	return ContentType::BLOCK_TYPE;
}

void CONTEXT_DATA::WriteDxf(DxfWriter & writer)
{
	writer.dxfString(300, "CONTEXT_DATA{");
	writer.dxfReal(40, 1.0);
	writer.coord(10, m_LandingPosition.x, m_LandingPosition.y);
	writer.dxfReal(41, m_TextHeight);
	writer.dxfReal(140, m_ArrowSize);
	writer.dxfReal(145, m_LandingGap);
	m_Content->WriteDxf(writer);
	// Already checked validity.
	writer.coord(110, &m_Leader.m_Lines[0].m_Points[0].x); // Mleader Plane Origin Point
	writer.coord(111, 1.0, 0.0); // MLeader Plane X-Axis Direction
	writer.coord(112, 0.0, 1.0); // MLeader Plane Y-Axis Direction
	writer.dxfInt(297, 0); // MLeader Plane Normal Reversed
	m_Leader.WriteDxf(writer);
	writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
	writer.dxfInt(273, 9); // Top text attachment direction:	9 = Center, 10 = Overline and Center
	writer.dxfString(301, "}");
}

void AcadMLeader::WriteDxf(DxfWriter & writer, bool bInPaperSpace)
{
	if (!m_ContextData.m_Content ||
		(m_ContextData.m_Leader.m_Lines.empty() || m_ContextData.m_Leader.m_Lines[0].m_Points.empty()) ||
		m_AttrDefs.size() != m_AttrValues.size())
	{
		PRINT_DEBUG_INFO("Invalid MultiLeader.");
		return;
	}
	writer.AddMLeaderToItsStyle(m_LeaderStyle, this);
	writer.dxfString(0, "MULTILEADER");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbMLeader");
	writer.dxfInt(270, 2);
	m_ContextData.WriteDxf(writer);
	writer.dxfHex(340, writer.LookupMLeaderStyleHandle(m_LeaderStyle));
	writer.dxfInt(90, 279552); // Property Override Flag
	writer.dxfInt(170, m_LeaderType);
	writer.dxfInt(91, -1056964608); // Leade LineColor
	writer.dxfHex(341, writer.LookupLinetypeHandle(m_LineType));
	writer.dxfInt(171, m_LineWeight);
	writer.dxfInt(290, m_EnableLanding ? 1 : 0);
	writer.dxfInt(291, m_EnableDogleg);
	//writer.dxfReal(41, m_DoglegLength);
	//writer.dxfReal(42, m_ArrowSize);
	writer.dxfInt(172, m_ContextData.m_Content->GetContentType());

	//writer.dxfHex(343, writer.LookupTextStyleHandle(m_TextStyle));
	writer.dxfInt(173, 1); // Text Left Attachment Type
	writer.dxfInt(95, 1); // Text Right Attachement Type
	writer.dxfInt(174, 1); // Text Angle Type
	writer.dxfInt(175, 0); // Text Alignment Type
	writer.dxfInt(92, -1056964608); // Text Color
	writer.dxfInt(292, 0); // Enable Frame Text
	if (m_ContextData.m_Content->GetContentType() == ContentType::BLOCK_TYPE)
	{
		BlockPart* pBlock = static_cast<BlockPart*>(m_ContextData.m_Content.get());
		writer.dxfHex(344, writer.LookupBlockEntryHandle(pBlock->m_BlockName));
	}
	writer.dxfInt(93, -1056964608); // Block Content Color
	writer.coord(10, 1.0, 1.0); // Block Content Scale
	writer.dxfReal(43, 0.0); // Block Content Rotation
	writer.dxfInt(176, 0); // Block Content Connection Type
	writer.dxfInt(293, 0); // Enable Annotation Scale
	for (size_t i = 0; i < m_AttrValues.size(); ++i)
	{
		writer.dxfHex(330, m_AttrDefs[i]->m_Handle);
		writer.dxfInt(177, int(i + 1)); // Block Attribute Index
		writer.dxfReal(44, 0.0); // Block Attribute Width
		writer.dxfString(302, m_AttrValues[i]);
	}
	writer.dxfInt(294, 0); // Text Direction Negative
	writer.dxfInt(178, 0); // Text Align in IPE
	writer.dxfInt(179, 1); // Text Attachment Point
	writer.dxfReal(45, 1.0); // 
	writer.dxfInt(271, 0); // Text attachment direction for MText contents: 0 = Horizontal, 1 = Vertical
	writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
	writer.dxfInt(273, 9); // Top text attachment direction:	9 = Center, 10 = Overline and Center
}

void CellInTableStyle::WriteDxf(DxfWriter& writer)
{
	writer.dxfReal(140, m_TextHeight);
	writer.dxfInt(170, m_Alignment);
	writer.dxfInt(62, m_TextColor);
	writer.dxfInt(63, m_BgColor);
	writer.dxfInt(283, m_BgColorEnabled ? 1 : 0);
	writer.dxfInt(90, m_CellDataType);
	writer.dxfInt(91, m_CellUnitType);
	writer.dxfString(1, ""); // ???
	for (int i = 0; i < 6; ++i)
	{
		writer.dxfInt(274 + i, m_BorderLineWeight[i]);
		writer.dxfInt(284 + i, m_BorderVisible[i] ? 1 : 0);
		writer.dxfInt(64 + i, m_BorderColor[i]);
	}
}

void TableStyle::WriteDxf(DxfWriter & writer, const std::string & name, int handle)
{
	ASSERT_DEBUG_INFO(!name.empty());
	writer.dxfString(0, "TABLESTYLE");
	writer.dxfHex(5, handle);
	writer.dxfString(102, "{ACAD_REACTORS");
	writer.dxfHex(330, 0x7E); // ACAD_TABLESTYLE dictionary handle
	const auto& tbls = writer.GetTables(name);
	for (const auto* tbl : tbls)
	{
		writer.dxfHex(330, tbl->m_Handle);
	}
	writer.dxfString(102, "}");
	writer.dxfHex(330, 0x7E);
	writer.dxfString(100, "AcDbTableStyle");
	writer.dxfInt(280, 0/*ACAD 2010*/);
	writer.dxfString(3, name); // Table style description
	writer.dxfInt(70, 0); // FlowDirection (integer): 0 = Down 1 = Up
	writer.dxfInt(71, 0); // Flags (bit-coded)
	writer.dxfReal(40, m_HorCellMargin);
	writer.dxfReal(41, m_VerCellMargin);
	writer.dxfInt(280, m_HasNoTitle ? 1 : 0);
	writer.dxfInt(281, m_HasNoColumnHeading ? 1 : 0);
	writer.dxfString(7, m_TextStyle);
	for (auto& cell : m_Cells)
	{
		cell.WriteDxf(writer);
	}
}

void CellText::WriteDxf(DxfWriter & writer)
{
	if (!m_Text.empty())
	{
		writer.dxfText(2, 1, m_Text);
	}
	if (!m_TextStyle.empty())
		writer.dxfString(7, m_TextStyle);
	writer.dxfInt(94, 0); // ???
	writer.dxfString(300, "");
	writer.dxfText(303, 302, m_Text);
}

void CellBlock::WriteDxf(DxfWriter & writer)
{
	ASSERT_DEBUG_INFO(m_AttrDefs.size() == m_AttrValues.size());
	writer.dxfHex(340, writer.LookupBlockEntryHandle(m_BlockName));
	writer.dxfReal(144, m_BlockScale);
	writer.dxfInt(179, int(m_AttrDefs.size()));

	if (m_AttrValues.empty())
		return;
	for (size_t i = 0; i < m_AttrDefs.size(); ++i)
	{
		writer.dxfHex(331, m_AttrDefs[i]->m_Handle);
		writer.dxfString(300, m_AttrValues[i]);
	}
}

void Cell::WriteDxf(DxfWriter & writer)
{
	ASSERT_DEBUG_INFO(m_Content);
	writer.dxfInt(171, m_Content->GetContentType());
	writer.dxfInt(172, m_Flag);
	writer.dxfInt(173, m_Merged ? 1 : 0);
	writer.dxfInt(174, m_AutoFit ? 1 : 0);
	writer.dxfInt(175, m_ColSpan);
	writer.dxfInt(176, m_RowSpan);
	writer.dxfInt(91, m_OverrideFlag1);
	writer.dxfInt(178, m_VirtualEdgeFlag);
	writer.dxfReal(145, m_Rotation);
	if (dynamic_cast<CellBlock*>(m_Content.get()))
		m_Content->WriteDxf(writer);
	if (m_Alignment != acInvalidAlignment)
		writer.dxfInt(170, m_Alignment);
	if (m_BgColor != acByBlock)
	{
		writer.dxfInt(63, m_BgColor);
	}
	if (m_TextColor != acByBlock)
		writer.dxfInt(64, m_TextColor);
	if (m_BgColor != acByBlock)
	{
		writer.dxfInt(283, m_BgColorEnabled);
	}
	if (m_TextHeight != 0.0)
		writer.dxfReal(140, m_TextHeight);
	writer.dxfInt(92, 0);/*Extended cell flags*/
	writer.dxfString(301, "CELL_VALUE");
	writer.dxfInt(93, m_Flag93); // ???
	writer.dxfInt(90, m_Flag90); // ???
	if (m_OverrideFlag2)
		writer.dxfInt(91, m_OverrideFlag2);
	if (dynamic_cast<CellText*>(m_Content.get()))
		m_Content->WriteDxf(writer);
	else
	{
		CellText().WriteDxf(writer);
	}
	if (!m_LeftBorderVisible)
		writer.dxfReal(288, 0);
	if (!m_RightBorderVisible)
		writer.dxfReal(285, 0);
	if (!m_TopBorderVisible)
		writer.dxfReal(289, 0);
	if (!m_BottomBorderVisible)
		writer.dxfReal(286, 0);
	writer.dxfString(304, "ACVALUE_END");
}

void AcadTable::WriteDxf(DxfWriter & writer, bool bInPaperSpace)
{
	if (m_RowCount <= 0 || m_ColCount <= 0
		|| m_RowHeights.size() != m_RowCount
		|| m_ColWidths.size() != m_ColCount
		|| m_Cells.size() != m_RowCount * m_ColCount)
	{
		PRINT_DEBUG_INFO("Invalid Table.");
		return;
	}
	writer.AddTableToItsStyle(m_TableStyle, this);
	writer.dxfString(0, "ACAD_TABLE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfInt(160, 3512); // ???
	writer.dxfString(100, "AcDbBlockReference");
	if (!m_TableBlockName.empty())
		writer.dxfString(2, m_TableBlockName);
	writer.coord(10, &m_InsertionPoint.x);
	writer.dxfString(100, "AcDbTable");
	writer.dxfInt(280, 0/*ACAD 2010*/);
	writer.dxfHex(342, writer.LookupTableStyleHandle(m_TableStyle));
	if (!m_TableBlockName.empty())
		writer.dxfHex(343, writer.LookupBlockEntryHandle(m_TableBlockName));
	writer.coord(11, 1.0, 0.0);
	writer.dxfInt(90, 22);
	writer.dxfInt(91, m_RowCount);
	writer.dxfInt(92, m_ColCount);
	writer.dxfInt(93, 0);
	writer.dxfInt(94, 0);
	writer.dxfInt(95, 0);
	writer.dxfInt(96, 0);
	for (auto h : m_RowHeights)
	{
		writer.dxfReal(141, h);
	}
	for (auto w : m_ColWidths)
	{
		writer.dxfReal(142, w);
	}
	for (auto& cell : m_Cells)
	{
		cell.WriteDxf(writer);
	}
	if (m_HorCellMargin != 0.0)
		writer.dxfReal(40, m_HorCellMargin);
	if (m_VerCellMargin != 0.0)
		writer.dxfReal(41, m_VerCellMargin);
	if (!m_LeftBorderVisible)
		writer.dxfReal(288, 0);
	if (!m_RightBorderVisible)
		writer.dxfReal(285, 0);
	if (!m_TopBorderVisible)
		writer.dxfReal(289, 0);
	if (!m_BottomBorderVisible)
		writer.dxfReal(286, 0);
}

void AcadLine::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "LINE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbLine");
	writer.dxfReal(10, m_StartPoint.x);
	writer.dxfReal(20, m_StartPoint.y);
	writer.dxfReal(11, m_EndPoint.x);
	writer.dxfReal(21, m_EndPoint.y);
}

void AcadLWPLine::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "LWPOLYLINE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbPolyline");
	writer.dxfInt(90, int(m_Vertices.size()));
	writer.dxfInt(70, (m_Closed ? 1 : 0) | 128);//128 = 启用线型生成 
	if (IsConstWidth())
		writer.dxfReal(43, m_Width);
	for (size_t i = 0; i < m_Vertices.size(); ++i)
	{
		writer.dxfReal(10, m_Vertices[i].x);
		writer.dxfReal(20, m_Vertices[i].y);
        if (!IsConstWidth())
        {
            writer.dxfReal(40, m_startWidths[i]);
            writer.dxfReal(41, m_endWidths[i]);
        }
		double bulge = GetBulge(i);
		if (bulge != 0.0)
			writer.dxfReal(42, bulge);
	}
}

void AcadMText::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "MTEXT");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbMText");
	writer.coord(10, m_InsertionPoint.x, m_InsertionPoint.y);
	if (m_CharHeight != 0.0)
		writer.dxfReal(40, m_CharHeight);
	writer.dxfReal(41, m_Width);
	writer.dxfInt(71, m_AttachmentPoint);
	writer.dxfInt(72, m_DrawingDirection);
	writer.dxfText(3, 1, m_Text);
	if (!m_StyleName.empty())
		writer.dxfString(7, m_StyleName);
	double rotAngle = m_RotationAngle * M_PI / 180.0;
	writer.dxfReal(11, cos(rotAngle));
	writer.dxfReal(21, sin(rotAngle));
	if (m_LineSpacingFactor != 0)
	{
		writer.dxfInt(73, 2);
		writer.dxfReal(44, m_LineSpacingFactor);
	}
	else
	{
		writer.dxfInt(73, 1);
	}
}

void AcadPoint::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "POINT");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbPoint");
	writer.dxfReal(10, m_Point.x);
	writer.dxfReal(20, m_Point.y);
}

void AcadSolid::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "SOLID");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbTrace");
	writer.coord(10, m_Point1.x, m_Point1.y);
	writer.coord(11, m_Point2.x, m_Point2.y);
	writer.coord(12, m_Point3.x, m_Point3.y);
	writer.coord(13, m_Point4.x, m_Point4.y);
}

void AcadSpline::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "SPLINE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbSpline");
	writer.dxfInt(70, m_Flag);
	writer.dxfInt(71, m_Degree);
	writer.dxfInt(72, int(m_Knots.size()));
	writer.dxfInt(73, int(m_ControlPoints.size()));
	writer.dxfInt(74, int(m_FitPoints.size()));
	writer.coord(12, m_StartTangent.x, m_StartTangent.y);
	writer.coord(13, m_EndTangent.x, m_EndTangent.y);
	for (auto knot : m_Knots)
		writer.dxfReal(40, knot);
	for (const auto& cp : m_ControlPoints)
	{
		writer.coord(10, cp.x, cp.y);
	}
	for (const auto& fp : m_FitPoints)
	{
		writer.coord(11, fp.x, fp.y);
	}
}

void AcadText::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "TEXT");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbText");
	if (m_HorAlign == 0 && m_VerAlign == 0)
		writer.coord(10, m_BaseLeftPoint.x, m_BaseLeftPoint.y);
	else
		writer.coord(11, m_InsertionPoint.x, m_InsertionPoint.y);
	if (m_Height != 0.0)
		writer.dxfReal(40, m_Height);
	writer.dxfString(1, m_Text);
	if (m_RotationAngle != 0.0)
		writer.dxfReal(50, m_RotationAngle);
	if (m_WidthFactor != 0.0)
		writer.dxfReal(41, m_WidthFactor);
	if (m_IsObliqueAngleValid && m_ObliqueAngle != 0.0)
		writer.dxfReal(51, m_ObliqueAngle);
	if (!m_StyleName.empty())
		writer.dxfString(7, m_StyleName);
	if (m_HorAlign != 0)
		writer.dxfInt(72, m_HorAlign);
	writer.dxfString(100, "AcDbText");
	if (m_VerAlign != 0)
		writer.dxfInt(73, m_VerAlign);
}

void AcadViewport::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "VIEWPORT");
	writer.dxfHex(5, m_Handle);
	writer.dxfString(100, "AcDbEntity");
	writer.dxfInt(67, 1);
	writer.dxfString(8, m_Layer);

	writer.dxfString(100, "AcDbViewport");
	writer.dxfReal(10, m_PaperspaceCenter.x);
	writer.dxfReal(20, m_PaperspaceCenter.y);
	writer.dxfReal(30, 0);

	writer.dxfReal(40, m_PaperspaceWidth);
	writer.dxfReal(41, m_PaperspaceHeight);

	// Increase viewport ID after write one viewport.
	++writer.m_CurrentViewportID;
	writer.dxfInt(68, writer.m_CurrentViewportID);
	writer.dxfInt(69, writer.m_CurrentViewportID);

	writer.dxfReal(12, m_ModelSpaceCenter.x);
	writer.dxfReal(22, m_ModelSpaceCenter.y);

	writer.dxfReal(13, 0);
	writer.dxfReal(23, 0);

	writer.dxfReal(14, 10);
	writer.dxfReal(24, 10);

	writer.dxfReal(15, 10);
	writer.dxfReal(25, 10);

	writer.dxfReal(16, 0);
	writer.dxfReal(26, 0);
	writer.dxfReal(36, 1);

	writer.dxfReal(17, 0);
	writer.dxfReal(27, 0);
	writer.dxfReal(37, 0);

	writer.dxfReal(42, 50);
	writer.dxfReal(43, 0);
	writer.dxfReal(44, 0);
	writer.dxfReal(45, m_ModelSpaceHeight);
	writer.dxfReal(50, 0);
	writer.dxfReal(51, m_TwistAngle);

	writer.dxfInt(72, 1000);

	auto clipEnt = m_ClipEnt.lock();
	if (clipEnt)
	{
        int status = 0x10000 | 0x8060;
        if (m_locked)
            status |= 0x4000;
		writer.dxfInt(90, status);
		writer.dxfHex(340, clipEnt->m_Handle);
	}
    else
    {
        int status = 0x8060;
        if (m_locked)
            status |= 0x4000;
		writer.dxfInt(90, status);
    }

	writer.dxfString(1, "");
	writer.dxfInt(281, 0);
	writer.dxfInt(71, 1);
	writer.dxfInt(74, 0);

	writer.dxfReal(110, 0);
	writer.dxfReal(120, 0);
	writer.dxfReal(130, 0);

	writer.dxfReal(111, 1);
	writer.dxfReal(121, 0);
	writer.dxfReal(131, 0);

	writer.dxfReal(112, 0);
	writer.dxfReal(122, 1);
	writer.dxfReal(132, 0);

	writer.dxfInt(79, 0);
	writer.dxfReal(146, 0);
	writer.dxfInt(170, 0);
}

void AcadXLine::WriteDxf(DxfWriter& writer, bool bInPaperSpace)
{
	writer.dxfString(0, "XLINE");
	WriteAttribute(writer, bInPaperSpace);
	writer.dxfString(100, "AcDbXline");
	writer.coord(10, m_First.x, m_First.y);
	writer.coord(11, m_Second.x, m_Second.y);
}

LayerData::LayerData()
	: m_Linetype("Continuous")
	, m_Color(acWhite)
	, m_LineWeight(acLnWtByLwDefault)
	, m_Plottable(false)
{
}

TextStyleData::TextStyleData()
	: m_PrimaryFontFile("txt.shx")
	, m_BigFontFile("")
	, m_Height(2.5)
	, m_WidthFactor(1.0)
	, m_ObliqueAngle(0)
{
}

DimStyleData::DimStyleData()
	:DimensionLineColor(acByBlock)
	, DimensionLineWeight(acLnWtByBlock)
	, ExtensionLineColor(acByBlock)
	, ExtensionLineWeight(acLnWtByBlock)
	, ExtensionLineExtend(0)
	, ExtensionLineOffset(0)
	, ExtLine1Suppress(false)
	, ExtLine2Suppress(false)
	, DimLine1Suppress(false)
	, DimLine2Suppress(false)
	, ArrowHead1Type(acArrowDefault)
	, ArrowHead2Type(acArrowDefault)
	, ArrowHeadSize(0.18)
	, CenterType(acCenterMark)
	, CenterMarkSize(0.18)
	, TextHeight(0.18)
	, TextColor(acByBlock)
	, VerticalTextPosition(acVertCentered)
	, HorizontalTextPosition(acHorzCentered)
	, TextAlign(false)
	, TextGap(0.045)
	, Fit(acBestFit)
	, TextMovement(acMoveTextNoLeader)
	, TextInside(false)
	, ForceLineInside(true)
	, UnitsFormat(acDimLDecimal)
	, UnitsPrecision(acDimPrecisionTwo)
	, SuppressLeadingZeros(false)
	, SuppressTrailingZeros(true)
	, AngleSuppressLeadingZeros(false)
	, AngleSuppressTrailingZeros(true)
	, AngleFormat(acDegrees)
	, AnglePrecision(acDimPrecisionOne)
	, LinearScaleFactor(1)
{
}

DimStyleData::DimStyleData(int/*unused*/)
	: DimensionLineColor(acByBlock)
	, DimensionLineWeight(acLnWtByBlock)
	, ExtensionLineColor(acByBlock)
	, ExtensionLineWeight(acLnWtByBlock)
	, ExtensionLineExtend(1.25)
	, ExtensionLineOffset(0.625)
	, ExtLine1Suppress(false)
	, ExtLine2Suppress(false)
	, DimLine1Suppress(false)
	, DimLine2Suppress(false)
	, ArrowHead1Type(acArrowDefault)
	, ArrowHead2Type(acArrowDefault)
	, ArrowHead1Block()
	, ArrowHead2Block()
	, ArrowHeadSize(2.5)
	, CenterType(acCenterMark)
	, CenterMarkSize(2.5)
	, TextStyle("Standard")
	, TextHeight(2.5)
	, TextColor(acWhite)
	, VerticalTextPosition(acAbove)
	, HorizontalTextPosition(acHorzCentered)
	, TextAlign(false)
	, TextGap(0.625)
	, Fit(acBestFit)
	, TextMovement(acDimLineWithText)
	, TextInside(false)
	, ForceLineInside(true)
	, UnitsFormat(acDimLDecimal)
	, UnitsPrecision(acDimPrecisionTwo)
	, SuppressLeadingZeros(false)
	, SuppressTrailingZeros(true)
	, AngleSuppressLeadingZeros(false)
	, AngleSuppressTrailingZeros(false)
	, AngleFormat(acDegrees)
	, AnglePrecision(acDimPrecisionOne)
	, LinearScaleFactor(1.0)
{
}

PlotSettings::PlotSettings()
	: m_PlotConfigFile("none_device")
	, m_PaperName("ISO_A4_(210.00_x_297.00_MM)")
	, m_LeftMargin(7.5)
	, m_BottomMargin(20)
	, m_RightMargin(7.5)
	, m_TopMargin(20)
	, m_Width(210)
	, m_Height(297)
	, m_PlotOrigin(0, 0)
	, m_PlotRotation(1)
{
}

LayoutData::LayoutData()
	: m_MinLim(-20, -7.5)
	, m_MaxLim(277, 202)
	, m_MinExt(25.7, 19.5)
	, m_MaxExt(231.3, 175.5)
	, m_LayoutOrder(0)
{
}

void LayoutData::WriteDxf(DxfWriter& writer)
{
	int viewportID = 0;
	// TODO: assign ID to viewport
	for (const auto& pEnt : m_Objects)
	{
		pEnt->WriteDxf(writer, true);
	}
}

void GetCenterWH(const DblPoints& polygon, CDblPoint& center, double& width, double& height)
{
    const size_t size = polygon.size();
    double minX, maxX, minY, maxY;
    auto xless = [](const CDblPoint& p1, const CDblPoint& p2)
    {
        return p1.x < p2.x;
    };
    auto yless = [](const CDblPoint& p1, const CDblPoint& p2)
    {
        return p1.y < p2.y;
    };
    minX = std::min_element(&polygon[0], &polygon[0] + size, xless)->x;
    minY = std::min_element(&polygon[0], &polygon[0] + size, yless)->y;
    maxX = std::max_element(&polygon[0], &polygon[0] + size, xless)->x;
    maxY = std::max_element(&polygon[0], &polygon[0] + size, yless)->y;
    center.x = (minX + maxX) / 2.0;
    center.y = (minY + maxY) / 2.0;
    width = maxX - minX;
    height = maxY - minY;
}

// if byWidth is true, scaleRelated is width in paper space;
// if byWidth is false, scaleRelated is scale from model space to paper space;
void CalPolygonPViewport(const DblPoints& polygonMS, const CDblPoint& centerPS, double scaleRelated, bool byWidth,
    double twistAngle, DblPoints& pointsPS, CDblPoint& centerMS, double& heightMS)
{
    // Get center, width, height in MS
    pointsPS = polygonMS;
    CDblPoint origin;
    double twistAngleRadians = twistAngle * M_PI / 180.0;
    for (auto& pt : pointsPS)
    {
        origin.Rotate(pt, twistAngleRadians);
    }
    double widthMS;
    GetCenterWH(pointsPS, centerMS, widthMS, heightMS);
    CDblPoint tmp(centerMS);
    origin.Rotate(centerMS, -twistAngleRadians);
    double dxInMS = centerMS.x - tmp.x;
    double dyInMS = centerMS.y - tmp.y;
	
    // translate points from MS to PS
    double scale;
    if (byWidth)
        scale = scaleRelated / widthMS;
    else
        scale = scaleRelated;
    CDblPoint centerInPS2;
    centerInPS2.x = centerMS.x * scale;
    centerInPS2.y = centerMS.y * scale;
    double dxInPS = centerPS.x - centerInPS2.x;
    double dyInPS = centerPS.y - centerInPS2.y;
    for (auto& pt : pointsPS)
    {
        pt.x = (pt.x + dxInMS) * scale + dxInPS;
        pt.y = (pt.y + dyInMS) * scale + dyInPS;
    }
}

void LayoutData::AddPolygonalViewport(const DblPoints & polygonPS, const CDblPoint & centerMS, double heighMS, double twistAngle)
{
    std::shared_ptr<AcadLWPLine> polyline(new AcadLWPLine);
    CDblPoint centerPS;
    double widthPS, heightPS;
    GetCenterWH(polygonPS, centerPS, widthPS, heightPS);
    polyline->m_Vertices = std::move(polygonPS);
    polyline->m_Closed = true;
    m_Objects.push_back(polyline);
    std::shared_ptr<AcadViewport> viewport(new AcadViewport);
    viewport->m_PaperspaceCenter = centerPS;
    viewport->m_PaperspaceWidth = widthPS;
    viewport->m_PaperspaceHeight = heightPS;
    viewport->m_ModelSpaceCenter = centerMS;
    viewport->m_ModelSpaceHeight = heighMS;
    viewport->m_TwistAngle = twistAngle;//in degrees
    viewport->m_ClipEnt = polyline;
    m_Objects.push_back(viewport);
}

void LayoutData::AddPolygonalViewportByWidth(const DblPoints & polygonMS, const CDblPoint & centerPS, double widthPS, double twistAngle)
{
    DblPoints polygonPS;
    CDblPoint centerMS;
    double heightMS;
    CalPolygonPViewport(polygonMS, centerPS, widthPS, true, twistAngle, polygonPS, centerMS, heightMS);
    AddPolygonalViewport(polygonPS, centerMS, heightMS, twistAngle);
}

void LayoutData::AddPolygonalViewportByScale(const DblPoints & polygonMS, const CDblPoint & centerPS, double scale, double twistAngle)
{
    DblPoints polygonPS;
    CDblPoint centerMS;
    double heightMS;
    CalPolygonPViewport(polygonMS, centerPS, scale, false, twistAngle, polygonPS, centerMS, heightMS);
    AddPolygonalViewport(polygonPS, centerMS, heightMS, twistAngle);
}

// To be implemented. But it is very hard and trivial.
// For now, end user can select all entities by pressing ctrl+A,
// move all entities to some point, all dims will show.
// Move them back, it will be OK.
void AcadDimAln::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimAng3P::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimAng::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimDia::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimRad::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimRot::GenerateBlock(DxfData& graph, const char* blockName)
{
}

void AcadDimOrd::GenerateBlock(DxfData& graph, const char* blockName)
{
}

#pragma region Overrides
void AcadDim::OverrideDimensionLineColor(long value)
{
	m_DimStyleOverride.DimensionLineColor = value;
	m_ValidMembersFlag[DIMENSIONLINECOLOR] = true;
}

void AcadDim::OverrideDimensionLineWeight(long value)
{
	m_DimStyleOverride.DimensionLineWeight = value;
	m_ValidMembersFlag[DIMENSIONLINEWEIGHT] = true;
}

void AcadDim::OverrideDimLine1Suppress(bool value)
{
	m_DimStyleOverride.DimLine1Suppress = value;
	m_ValidMembersFlag[DIMLINE1SUPPRESS] = true;
}

void AcadDim::OverrideDimLine2Suppress(bool value)
{
	m_DimStyleOverride.DimLine2Suppress = value;
	m_ValidMembersFlag[DIMLINE2SUPPRESS] = true;
}

void AcadDim::OverrideForceLineInside(bool value)
{
	m_DimStyleOverride.ForceLineInside = value;
	m_ValidMembersFlag[FORCELINEINSIDE] = true;
}

void AcadDim::OverrideExtensionLineColor(long value)
{
	m_DimStyleOverride.ExtensionLineColor = value;
	m_ValidMembersFlag[EXTENSIONLINECOLOR] = true;
}

void AcadDim::OverrideExtensionLineWeight(long value)
{
	m_DimStyleOverride.ExtensionLineWeight = value;
	m_ValidMembersFlag[EXTENSIONLINEWEIGHT] = true;
}

void AcadDim::OverrideExtensionLineExtend(double value)
{
	m_DimStyleOverride.ExtensionLineExtend = value;
	m_ValidMembersFlag[EXTENSIONLINEEXTEND] = true;
}

void AcadDim::OverrideExtensionLineOffset(double value)
{
	m_DimStyleOverride.ExtensionLineOffset = value;
	m_ValidMembersFlag[EXTENSIONLINEOFFSET] = true;
}

void AcadDim::OverrideExtLine1Suppress(bool value)
{
	m_DimStyleOverride.ExtLine1Suppress = value;
	m_ValidMembersFlag[EXTLINE1SUPPRESS] = true;
}

void AcadDim::OverrideExtLine2Suppress(bool value)
{
	m_DimStyleOverride.ExtLine2Suppress = value;
	m_ValidMembersFlag[EXTLINE2SUPPRESS] = true;
}

void AcadDim::OverrideArrowHead1Type(long value)
{
	m_DimStyleOverride.ArrowHead1Type = value;
	m_ValidMembersFlag[ARROWHEAD1TYPE] = true;
}

void AcadDim::OverrideArrowHead2Type(long value)
{
	m_DimStyleOverride.ArrowHead2Type = value;
	m_ValidMembersFlag[ARROWHEAD2TYPE] = true;
}

void AcadDim::OverrideArrowHead1Block(std::string value)
{
	m_DimStyleOverride.ArrowHead1Type = acArrowUserDefined;
	m_DimStyleOverride.ArrowHead1Block = value;
	m_ValidMembersFlag[ARROWHEAD1TYPE] = true;
}

void AcadDim::OverrideArrowHead2Block(std::string value)
{
	m_DimStyleOverride.ArrowHead2Type = acArrowUserDefined;
	m_DimStyleOverride.ArrowHead2Block = value;
	m_ValidMembersFlag[ARROWHEAD2TYPE] = true;
}

void AcadDim::OverrideArrowHeadSize(double value)
{
	m_DimStyleOverride.ArrowHeadSize = value;
	m_ValidMembersFlag[ARROWHEADSIZE] = true;
}

void AcadDim::OverrideCenterType(long value)
{
	m_DimStyleOverride.CenterType = value;
	m_ValidMembersFlag[CENTERTYPE] = true;
}

void AcadDim::OverrideCenterMarkSize(double value)
{
	m_DimStyleOverride.CenterMarkSize = value;
	m_ValidMembersFlag[CENTERMARKSIZE] = true;
}

void AcadDim::OverrideTextStyle(std::string value)
{
	m_DimStyleOverride.TextStyle = value;
	m_ValidMembersFlag[TEXTSTYLE] = true;
}

void AcadDim::OverrideTextHeight(double value)
{
	m_DimStyleOverride.TextHeight = value;
	m_ValidMembersFlag[TEXTHEIGHT] = true;
}

void AcadDim::OverrideTextColor(long value)
{
	m_DimStyleOverride.TextColor = value;
	m_ValidMembersFlag[TEXTCOLOR] = true;
}

void AcadDim::OverrideVerticalTextPosition(long value)
{
	m_DimStyleOverride.VerticalTextPosition = value;
	m_ValidMembersFlag[VERTICALTEXTPOSITION] = true;
}

void AcadDim::OverrideHorizontalTextPosition(long value)
{
	m_DimStyleOverride.HorizontalTextPosition = value;
	m_ValidMembersFlag[HORIZONTALTEXTPOSITION] = true;
}

void AcadDim::OverrideTextAlign(bool value)
{
	m_DimStyleOverride.TextAlign = value;
	m_ValidMembersFlag[TEXTALIGN] = true;
}

void AcadDim::OverrideTextGap(double value)
{
	m_DimStyleOverride.TextGap = value;
	m_ValidMembersFlag[TEXTGAP] = true;
}

void AcadDim::OverrideText(std::string value)
{
	m_DimStyleOverride.Text = value;
	m_ValidMembersFlag[TEXT] = true;
}

void AcadDim::OverrideFit(long value)
{
	m_DimStyleOverride.Fit = value;
	m_ValidMembersFlag[FIT] = true;
}

void AcadDim::OverrideTextMovement(long value)
{
	m_DimStyleOverride.TextMovement = value;
	m_ValidMembersFlag[TEXTMOVEMENT] = true;
}

void AcadDim::OverrideTextInside(bool value)
{
	m_DimStyleOverride.TextInside = value;
	m_ValidMembersFlag[TEXTINSIDE] = true;
}

void AcadDim::OverrideUnitsFormat(long value)
{
	m_DimStyleOverride.UnitsFormat = value;
	m_ValidMembersFlag[UNITSFORMAT] = true;
}

void AcadDim::OverrideUnitsPrecision(long value)
{
	m_DimStyleOverride.UnitsPrecision = value;
	m_ValidMembersFlag[UNITSPRECISION] = true;
}

void AcadDim::OverrideSuppressLeadingZeros(bool value)
{
	m_DimStyleOverride.SuppressLeadingZeros = value;
	m_ValidMembersFlag[SUPPRESSLEADINGZEROS] = true;
}

void AcadDim::OverrideSuppressTrailingZeros(bool value)
{
	m_DimStyleOverride.SuppressTrailingZeros = value;
	m_ValidMembersFlag[SUPPRESSTRAILINGZEROS] = true;
}

void AcadDim::OverrideAngleSuppressLeadingZeros(bool value)
{
	m_DimStyleOverride.AngleSuppressLeadingZeros = value;
	m_ValidMembersFlag[ANGLESUPPRESSLEADINGZEROS] = true;
}

void AcadDim::OverrideAngleSuppressTrailingZeros(bool value)
{
	m_DimStyleOverride.AngleSuppressTrailingZeros = value;
	m_ValidMembersFlag[ANGLESUPPRESSTRAILINGZEROS] = true;
}

void AcadDim::OverrideAngleFormat(long value)
{
	m_DimStyleOverride.AngleFormat = value;
	m_ValidMembersFlag[ANGLEFORMAT] = true;
}

void AcadDim::OverrideAnglePrecision(long value)
{
	m_DimStyleOverride.AnglePrecision = value;
	m_ValidMembersFlag[ANGLEPRECISION] = true;
}

void AcadDim::OverrideLinearScaleFactor(double value)
{
	m_DimStyleOverride.LinearScaleFactor = value;
	m_ValidMembersFlag[LINEARSCALEFACTOR] = true;
}

void AcadDim::OverrideTextRotation(double textRotation)
{
	m_TextRotation = textRotation;
	m_ValidMembersFlag[TEXTROTATION] = true;
}
#pragma endregion

}
