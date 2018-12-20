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
#include <bitset>
#include <vector>
#include <map>
#include <list>
#include <memory>

#include "ImpExpMacro.h"
#include "ACADConst.h"
#include "utility.h"

namespace DXF {

using DblPoints = std::vector<CDblPoint>;

int GetArrowHeadType(const char* value);
const char* GetPredefinedArrowHeadBlockName(int predefinedArrowheadType);

class DxfData;
class DxfWriter;

struct NameHandle
{
	std::string name;
	int handle;
};

struct DXF_API LayerData
{
	LayerData();

	std::string m_Linetype;
	long m_Color;
	long m_LineWeight;
	bool m_Plottable;
};

struct DXF_API TextStyleData
{
	TextStyleData();

	std::string m_PrimaryFontFile;//.shx file used for single byte character.
	std::string m_BigFontFile;//.shx file used for multi-byte character.

	//"SimSun",                   "宋体",
	//"SimHei",                   "黑体",
	//"KaiTi_GB2312",             "楷体_GB2312",
	//"LiSu",                     "隶书",
	//"FangSong_GB2312",          "仿宋_GB2312",
	//"YouYuan",                  "幼圆",
	//"STCaiyun",                 "华文彩云",
	//"STFangsong",               "华文仿宋",
	//"STXihei",                  "华文细黑",
	//"STXingkai",                "华文行楷",
	//"STXinwei",                 "华文新魏",
	//"STZhongsong",              "华文中宋",
	//"FZShuTi",                  "方正舒体",
	//"FZYaoTi",                  "方正姚体",
	//"Simsun (Founder Extended)","宋体-方正超大字符集",
	//"NSimSun",                  "新宋体"
	// If you specify m_TrueType, then m_PrimaryFontFile and m_BigFontFile are ignored.
	std::string m_TrueType;// SimSun, SimHei, etc
	double m_Height;
	double m_WidthFactor;
	double m_ObliqueAngle;//in degrees within the range of -85 to +85 degrees.
};

//枚举值的顺序应该与DimStyleData的成员变量一致
enum
{
	DIMENSIONLINECOLOR
	, DIMENSIONLINEWEIGHT
	, DIMLINE1SUPPRESS
	, DIMLINE2SUPPRESS
	, FORCELINEINSIDE

	, EXTENSIONLINECOLOR
	, EXTENSIONLINEWEIGHT
	, EXTENSIONLINEEXTEND
	, EXTENSIONLINEOFFSET
	, EXTLINE1SUPPRESS
	, EXTLINE2SUPPRESS

	, ARROWHEAD1TYPE
	, ARROWHEAD2TYPE
	, ARROWHEADSIZE
	, CENTERTYPE
	, CENTERMARKSIZE

	, TEXTSTYLE
	, TEXTHEIGHT
	, TEXTCOLOR
	, VERTICALTEXTPOSITION
	, HORIZONTALTEXTPOSITION
	, TEXTALIGN
	, TEXTGAP
	, TEXT
	, FIT
	, TEXTMOVEMENT
	, TEXTINSIDE

	, UNITSFORMAT
	, UNITSPRECISION
	, SUPPRESSLEADINGZEROS
	, SUPPRESSTRAILINGZEROS
	, ANGLESUPPRESSLEADINGZEROS
	, ANGLESUPPRESSTRAILINGZEROS
	, ANGLEFORMAT
	, ANGLEPRECISION
	, LINEARSCALEFACTOR

	, TEXTROTATION
	, OBLIQUEANGLE

	, LASTBIT//必须位于最后
};

struct DXF_API DimStyleData
{
	DimStyleData();
	//创建ISO-25样式
	explicit DimStyleData(int/*unused*/);

	//尺寸线
	long DimensionLineColor;//初始值:acByBlock
	long DimensionLineWeight;//初始值:acLnWtByBlock
	bool DimLine1Suppress;//初始值:false
	bool DimLine2Suppress;//初始值:false
	//true: 即使当文字绘制在尺寸界线之外，也要把尺寸线置于尺寸界线之内。 
	//false: 当文字绘制在尺寸界线之外时，不强制把尺寸线置于尺寸界线之内。
	//初始值:true
	bool ForceLineInside;

	//尺寸界线
	long ExtensionLineColor;//初始值:acByBlock
	long ExtensionLineWeight;//初始值:acLnWtByBlock
	double ExtensionLineExtend;//指定尺寸界线超出尺寸线的长度//初始值:0
	double ExtensionLineOffset;//指定尺寸界线从尺寸界线根点处的偏移//初始值:0
	bool ExtLine1Suppress;//true--不绘制第一条尺寸界线//初始值:false
	bool ExtLine2Suppress;//true--不绘制第二条尺寸界线//初始值:false

	//箭头
	long ArrowHead1Type;//取值范围：enum AcDimArrowheadType//初始值:acArrowDefault
	long ArrowHead2Type;//取值范围：enum AcDimArrowheadType//初始值:acArrowDefault
	std::string ArrowHead1Block;//当Arrowhead1Type = acArrowUserDefined时起作用
	std::string ArrowHead2Block;//当Arrowhead2Type = acArrowUserDefined时起作用
	double ArrowHeadSize;//初始值:0.18
	long CenterType;//取值范围：enum AcDimCenterType//初始值:acCenterMark
	double CenterMarkSize;//初始值:0.09

	//文字
	std::string TextStyle;//尺寸文字和公差文字的样式
	double TextHeight;//当TextStyle有固定字高时，这个属性被忽略。//初始值:0.18
	long TextColor;//初始值:acByBlock
	//取值范围：enum AcDimVerticalJustification//初始值:acVertCentered
	long VerticalTextPosition;
	//取值范围：enum AcDimHorizontalJustification//初始值:acHorzCentered
	long HorizontalTextPosition;
	bool TextAlign;//指定文字方向。true--水平,false--与尺寸线对齐//初始值:false
	//当为了容纳标注文字而断开尺寸线时，指定标注文字和尺寸线间的距离。//初始值:0.045
	//当文字位于尺寸线上方时，指定了标注文字和尺寸线间的距离。
	double TextGap;
	//如果为空或为“<>”，标注测量将绘制为文字；如果为“ ”（一个空格），文字将被抑制。
	//例如"Ф<>mm"。
	std::string Text;
	//调整
	//取值范围：enum AcDimFit，指定当尺寸界线内空间不足时采取的调整措施
	long Fit;//初始值:acBestFit
	//指定移动文字时如何调整尺寸线，对程序出图没有影响，但影响对所出图编辑，应采用缺省值。
	//取值范围：enum AcDimTextMovement
	long TextMovement;//初始值:acMoveTextNoLeader
	//true: 强制标注文字位于尺寸界线内。 
	//false: 仅当有足够空间时把标注文字置于尺寸界线内。
	//仅对线性和角度标注起作用，对半径和直径标注不起作用。
	//初始值:false
	bool TextInside;

	//主单位
	//取值范围：enum AcDimLUnits,指定除角度外所有标注的单位格式//初始值:acDimLDecimal
	long UnitsFormat;
	//取值范围：enum AcDimPrecision,指定除角度外所有标注的小数位数
	long UnitsPrecision;//初始值:acDimPrecisionTwo
	bool SuppressLeadingZeros;//抑制标注值的前导0//初始值:false
	bool SuppressTrailingZeros;//抑制标注值的后缀0//初始值:true
	bool AngleSuppressLeadingZeros;//抑制标注值的前导0//初始值:false
	bool AngleSuppressTrailingZeros;//抑制标注值的后缀0//初始值:true
	long AngleFormat;//取值范围：AcAngleUnits//指定角度标注的单位格式//初始值:acDegrees
	long AnglePrecision;//指定角度标注文本的精度//初始值:acDimPrecisionOne
	double LinearScaleFactor;//除角度标注外所有标注度量的比例因子//初始值:1
};

struct DXF_API PlotSettings
{
	PlotSettings();

	/*系统打印机或打印配置文件的名称*/
	std::string m_PlotConfigFile;

	//图纸名称
	std::string m_PaperName;

	/*图纸尺寸*/	//in millimeter
	double m_LeftMargin;
	double m_BottomMargin;
	double m_RightMargin;
	double m_TopMargin;
	double m_Width;
	double m_Height;

	/*! 打印旋转：
	0 = 无旋转
	1 = 逆时针旋转 90 度
	2 = 颠倒
	3 = 顺时针旋转 90 度
	*/
	int m_PlotRotation;

	CDblPoint m_PlotOrigin;
};

struct DXF_API EntAttribute
{
	EntAttribute();
	EntAttribute(const EntAttribute&) = default;
	virtual ~EntAttribute() = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) {};
	virtual void AssignHandle(/*in, out*/int& handle);
	void WriteAttribute(DxfWriter& writer, bool bInPaperSpace) const;

	mutable int m_Handle; // m_Handle will be assigned value on writing.
	bool m_IsInPaperspace;
	AcColor m_Color;
	AcLineWeight m_Lineweight;
	double m_LinetypeScale;
	std::string m_Layer;
	std::string m_Linetype;
	std::vector<std::weak_ptr<EntAttribute>> m_Reactors;
};

class DXF_API EntityList : public std::vector<std::shared_ptr<EntAttribute>>
{
public:
	EntityList() = default;
	// Because EntityList owns the entities, so it can not be shallow-copied.
	EntityList(const EntityList& src) = delete;
	~EntityList() = default;
	iterator FindEntity(int handle);
};

using VectorOfEntityList = std::vector<std::shared_ptr<EntityList>>;

struct DXF_API AcadCircle : public EntAttribute
{
	AcadCircle();
	AcadCircle(const AcadCircle&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_Center;
	double m_Radius;
};

struct DXF_API AcadArc : public AcadCircle
{
	AcadArc();
	AcadArc(const AcadArc&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	double m_StartAngle;//in degrees
	double m_EndAngle;//in degrees
};

struct DXF_API AcadAttDef : public EntAttribute
{
	AcadAttDef();
	AcadAttDef(const AcadAttDef&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	// See AcadText for explanation.
	CDblPoint m_BaseLeftPoint; /*10*/
	CDblPoint m_InsertionPoint; /*11*/
	double m_TextHeight; /*40*/
	std::string m_Text; /*1*/
	std::string m_Tag; /*2, cannot contain spaces*/
	AcAttributeMode m_Flags; /*70*/
	double m_RotationAngle;/*50, in degrees*/
	std::string m_TextStyle; /*7*/
	short m_HorAlign; /*72*/
	short m_VerAlign; /*74*/
	int m_DuplicateFlag; // 1 = Keep existing
	std::string m_Prompt; /*3*/
};

struct DXF_API AcadAttrib : public EntAttribute
{
	AcadAttrib();
	AcadAttrib(const AcadAttrib&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;
	void WriteDxf(DxfWriter& writer, bool bInPaperSpace, int parentHandle);

	// See AcadText for explanation.
	CDblPoint m_BaseLeftPoint; /*10*/
	CDblPoint m_InsertionPoint; /*11*/
	double m_TextHeight; /*40*/
	std::string m_Text; /*1*/
	std::string m_Tag; /*2, cannot contain spaces*/
	AcAttributeMode m_Flags; /*70*/
	double m_RotationAngle;/*50, in degrees*/
	std::string m_TextStyle; /*7*/
	short m_HorAlign; /*72*/
	short m_VerAlign; /*74*/
	int m_DuplicateFlag; // 1 = Keep existing
};


struct DXF_API AcadBlockInstance : public EntAttribute
{
	AcadBlockInstance();
	AcadBlockInstance(const AcadBlockInstance&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;
	virtual void AssignHandle(/*in, out*/int& handle);

	std::string m_Name;
	CDblPoint m_InsertionPoint;
	double m_Xscale, m_Yscale, m_Zscale;
	double m_RotationAngle;//in degrees
	std::vector<AcadAttrib> m_Attribs;
};

struct DXF_API AcadDim : public EntAttribute
{
	AcadDim();
	AcadDim(const AcadDim&) = default;
	void WriteHeader(DxfWriter& writer, bool bInPaperSpace);
	void WriteOverride(DxfWriter& writer);

	//如果m_BlockName为空，DxfWriter需要生成标注图块，否则说明是从DXF文件读取的，已经读入显示标注图块(*D<n>)。
	std::string m_BlockName;
	//定义点（在 WCS 中），定义点含义与标注类型相关。
	CDblPoint m_DefPoint;
	//标注文字的中点（在 OCS 中）
	CDblPoint m_TextPosition;
	//值 0 - 6 是表示标注类型的整数值。值 32、64 和 128 是添加到整数值中的位值（在 R13 及以后的版本中始终设置值 32）
	int m_DimType;
	AcAttachmentPoint m_Attachment;

	std::string m_DimStyleName;
	//用来指示m_DimStyleOverride中某个成员变量(Name除外)/m_TextRotation的值是否有效。
	std::bitset<LASTBIT> m_ValidMembersFlag;
	//m_DimStyleOverride.Name没有使用。
	DimStyleData m_DimStyleOverride;
	//标注文字与其默认方向所成的旋转角度（尺寸线方向）（可选）
	double m_TextRotation;//in degrees

#pragma region Overrides
	void OverrideDimensionLineColor(long value);
	void OverrideDimensionLineWeight(long value);
	void OverrideDimLine1Suppress(bool value);
	void OverrideDimLine2Suppress(bool value);
	void OverrideForceLineInside(bool value);
	void OverrideExtensionLineColor(long value);
	void OverrideExtensionLineWeight(long value);
	void OverrideExtensionLineExtend(double value);
	void OverrideExtensionLineOffset(double value);
	void OverrideExtLine1Suppress(bool value);
	void OverrideExtLine2Suppress(bool value);
	void OverrideArrowHead1Type(long value);
	void OverrideArrowHead2Type(long value);
	void OverrideArrowHead1Block(std::string value);
	void OverrideArrowHead2Block(std::string value);
	void OverrideArrowHeadSize(double value);
	void OverrideCenterType(long value);
	void OverrideCenterMarkSize(double value);
	void OverrideTextStyle(std::string value);
	void OverrideTextHeight(double value);
	void OverrideTextColor(long value);
	void OverrideVerticalTextPosition(long value);
	void OverrideHorizontalTextPosition(long value);
	void OverrideTextAlign(bool value);
	void OverrideTextGap(double value);
	void OverrideText(std::string value);
	void OverrideFit(long value);
	void OverrideTextMovement(long value);
	void OverrideTextInside(bool value);
	void OverrideUnitsFormat(long value);
	void OverrideUnitsPrecision(long value);
	void OverrideSuppressLeadingZeros(bool value);
	void OverrideSuppressTrailingZeros(bool value);
	void OverrideAngleSuppressLeadingZeros(bool value);
	void OverrideAngleSuppressTrailingZeros(bool value);
	void OverrideAngleFormat(long value);
	void OverrideAnglePrecision(long value);
	void OverrideLinearScaleFactor(double value);
	void OverrideTextRotation(double textRotation);
#pragma endregion

#pragma region Gets
	long GetDimensionLineColor(const DxfData& graph) const;
	long GetDimensionLineWeight(const DxfData& graph) const;
	bool GetDimLine1Suppress(const DxfData& graph) const;
	bool GetDimLine2Suppress(const DxfData& graph) const;
	bool GetForceLineInside(const DxfData& graph) const;
	long GetExtensionLineColor(const DxfData& graph) const;
	long GetExtensionLineWeight(const DxfData& graph) const;
	double GetExtensionLineExtend(const DxfData& graph) const;
	double GetExtensionLineOffset(const DxfData& graph) const;
	bool GetExtLine1Suppress(const DxfData& graph) const;
	bool GetExtLine2Suppress(const DxfData& graph) const;
	long GetArrowHead1Type(const DxfData& graph) const;
	long GetArrowHead2Type(const DxfData& graph) const;
	std::string GetArrowHead1Block(const DxfData& graph) const;
	std::string GetArrowHead2Block(const DxfData& graph) const;
	double GetArrowHeadSize(const DxfData& graph) const;
	long GetCenterType(const DxfData& graph) const;
	double GetCenterMarkSize(const DxfData& graph) const;
	std::string GetTextStyle(const DxfData& graph) const;
	double GetTextHeight(const DxfData& graph) const;
	long GetTextColor(const DxfData& graph) const;
	long GetVerticalTextPosition(const DxfData& graph) const;
	long GetHorizontalTextPosition(const DxfData& graph) const;
	bool GetTextAlign(const DxfData& graph) const;
	double GetTextGap(const DxfData& graph) const;
	std::string GetText(const DxfData& graph) const;
	long GetFit(const DxfData& graph) const;
	long GetTextMovement(const DxfData& graph) const;
	bool GetTextInside(const DxfData& graph) const;
	long GetUnitsFormat(const DxfData& graph) const;
	long GetUnitsPrecision(const DxfData& graph) const;
	bool GetSuppressLeadingZeros(const DxfData& graph) const;
	bool GetSuppressTrailingZeros(const DxfData& graph) const;
	bool GetAngleSuppressLeadingZeros(const DxfData& graph) const;
	bool GetAngleSuppressTrailingZeros(const DxfData& graph) const;
	long GetAngleFormat(const DxfData& graph) const;
	long GetAnglePrecision(const DxfData& graph) const;
	double GetLinearScaleFactor(const DxfData& graph) const;
#pragma endregion

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) = 0;
};

struct AcadDimInternal : public AcadDim
{
	AcadDimInternal();

	std::vector<int> m_ReactorHandles;

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override {};
};

struct DXF_API AcadDimAln : public AcadDim
{
	AcadDimAln();
	AcadDimAln(const AcadDimAln&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_ExtLine1Point, m_ExtLine2Point;
	double m_RotationAngle;//in degrees

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimAng3P : public AcadDim
{
	AcadDimAng3P();
	AcadDimAng3P(const AcadDimAng3P&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_FirstEnd, m_SecondEnd; // the endpoints of the extension lines
	CDblPoint m_AngleVertex; // the vertex of the angle

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimAng : public AcadDim
{
	AcadDimAng();
	AcadDimAng(const AcadDimAng&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_FirstStart, m_FirstEnd; // the first extension line
	CDblPoint m_SecondStart/*, m_SecondEnd*/; // the second extension line

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimDia : public AcadDim
{
	AcadDimDia();
	AcadDimDia(const AcadDimDia&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_ChordPoint/*, m_FarChordPoint*/;
	double m_LeaderLength;

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimRad : public AcadDim
{
	AcadDimRad();
	AcadDimRad(const AcadDimRad&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_ChordPoint/*, m_Center*/;
	double m_LeaderLength;

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimRot : public AcadDimAln
{
	AcadDimRot();
	AcadDimRot(const AcadDimRot&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadDimOrd : public AcadDim
{
	AcadDimOrd();
	AcadDimOrd(const AcadDimOrd&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_OrdPoint, m_LeaderPoint;

public: // Used internally
	virtual void GenerateBlock(DxfData& graph, const char* blockName) override;
};

struct DXF_API AcadEllipse : public EntAttribute
{
	AcadEllipse();
	AcadEllipse(const AcadEllipse&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_Center;
	CDblPoint m_MajorAxisPoint;
	double m_MinorAxisRatio;
	double m_StartAngle;//in radians
	double m_EndAngle;//in radians
};

enum BoundaryPathFlag { Outer = 0x1, LWPline = 0x2, Export = 0x4, Text = 0x8, OuterMost = 0x10 };
struct DXF_API AcadHatch : public EntAttribute
{
	AcadHatch();
	AcadHatch(const AcadHatch&) = default;

	enum FillFlag { PatternFill, SolidFill }; // 图案填充, 实体填充
	FillFlag m_FillFlag;
	AcHatchStyle m_HatchStyle;

	// These fields are only valid when m_FillFlag is PatternFill
	std::string m_PatternName;
	AcPatternType m_PatternType;
	double m_PatternScale;
	double m_PatternAngle;//in degrees
	// DXF code 47
	double m_PixelSize;

	// AcadHatch owns these entities. They are just boundary data, not standalone entities.
	// When you write DXF, don't operate these 2 variables, use AddEntity method instead.
	EntityList m_OuterLoop;
	VectorOfEntityList m_InnerLoops;

	// AcadHatch doesn't own these entities.
	// When you write DXF, don't operate these 2 variables, use AddAssociatedEntity method instead.
	std::vector<std::weak_ptr<EntAttribute>> m_OuterAssociatedEntities;
	std::vector<std::shared_ptr<std::vector<std::weak_ptr<EntAttribute>>>> m_InnerAssociatedEntities;

	BoundaryPathFlag m_OuterFlag;
	std::vector<BoundaryPathFlag> m_InnerFlags;

	// if loopIndex is -1, add pEnt to outer loop;
	// if loopIndex >= 0, add pEnt to the specified inner loop;
	// You can only add one closed AcadLWPLine/AcadCircle into a loop;
	// Or you can add many AcadLine, AcadArc, AcadEllipseArc, AcadSpline into a loop;
	// It is client programmer's responsibility to ensure that the loop is closed. 
	bool AddEntity(const std::shared_ptr<EntAttribute>& pEnt, int loopIndex = -1);
	// Here, pEnt can be any entities, such as AcadText.
	bool AddAssociatedEntity(const std::shared_ptr<EntAttribute>& pEnt, int loopIndex = -1);
	bool HasAssociatedEntity() const;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

private:
	void WriteLoop(DxfWriter& writer, int loopIndex);
};

struct DXF_API AcadLeader : public EntAttribute
{
	AcadLeader();
	AcadLeader(const AcadLeader&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	std::string m_StyleName;
	AcLeaderType m_Type;
	double m_ArrowSize;
	bool m_HasBaseLine;
	bool m_BaseLineDirection;
	DblPoints m_Vertices;
	double m_CharHeight, m_TextWidth;

	// It is a AcadMText or AcadBlockInstance, you should add it to the same block with the block  AcadLeader.
	std::weak_ptr<EntAttribute> m_Annotation;
};

enum ContentType { BLOCK_TYPE = 1, MTEXT_TYPE = 2 };

struct DXF_API StyleContent
{
	virtual ~StyleContent() = 0 {}
	virtual void WriteDxf(DxfWriter& writer) = 0;
	virtual ContentType GetContentType() const = 0;
};

struct DXF_API StyleMTextPart : StyleContent
{
	StyleMTextPart();
	StyleMTextPart(const StyleMTextPart&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual ContentType GetContentType() const override;
	std::string m_TextStyle; /*342*/
	AcTextAngleType m_TextAngleType; /*175*/
	AcColor m_TextColor; /*93*/
	double m_TextHeight; /* 45*/
	double m_AlignSpace; /*46*/
};

struct DXF_API StyleBlockPart : StyleContent
{
	StyleBlockPart();
	StyleBlockPart(const StyleBlockPart&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual ContentType GetContentType() const override;
	std::string  m_BlockName; /*343*/
	AcBlockConnectionType m_BlockConnectionType; /*177, Center Extens, Insertion point*/
	AcColor m_BlockColor; /*94*/
	CDblPoint m_BlockScale; /*47, 49*/
};

struct DXF_API MLeaderStyle /*in OBJECTS section*/
{
	MLeaderStyle();
	MLeaderStyle(const MLeaderStyle&) = default;
	void WriteDxf(DxfWriter& writer, const std::string& name, int handle);
	AcMLeaderType m_LeaderType; /*173*/
	AcColor m_LineColor; /*91*/
	std::string m_LineType; /*340*/
	AcLineWeight m_LineWeight; /*92*/
	std::string m_ArrowHead; /*341, arrow block, can be empty*/
	double m_ArrowSize; /*44*/

	int m_MaxLeaderPoints; /*90*/
	double m_FirstSegAngleConstraint; /*40*/
	double m_SecondSegAngleConstraint; /*41*/
	bool m_EnableLanding; /*290*/
	bool m_EnableDogleg; /*291*/
	bool m_IsAnnotative; /*296*/
	double m_LandingGap; /*42*/

	double m_DoglegLength; /*43*/
	double m_BreakGapSize; /*143*/

	// ContentType m_ContentType; /*170*/
	std::shared_ptr<StyleContent> m_Content;
};

struct DXF_API LeaderLine
{
	DblPoints m_Points; /*10*/
};

struct DXF_API LeaderPart
{
	LeaderPart();
	LeaderPart(const LeaderPart&) = default;
	void WriteDxf(DxfWriter& writer);
	CDblPoint m_CommonPoint; /*10*/
	std::vector<LeaderLine> m_Lines;
	double m_DoglegLength; /*40, required*/
};

struct DXF_API Content
{
	virtual void WriteDxf(DxfWriter& writer) = 0;
	virtual ContentType GetContentType() const = 0;
	virtual ~Content() = 0 {}
};

struct DXF_API MTextPart : Content
{
	MTextPart();
	MTextPart(const MTextPart&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual ContentType GetContentType() const override;
	std::string m_Text; /* 304, no default value, use \P as new line. such as hello\Pworld*/
	double m_TextWidth; /*43, no default value*/
	std::string m_TextStyle; /*340*/
	AcColor m_TextColor; /*90*/
	CDblPoint m_TextLocation; /*12, no default value*/
	double m_TextRotation; /*42, in radians, 13 = cos, 23 = sin*/
	AcAttachmentPoint m_AttachmentPoint;; /*171*/
};

struct DXF_API BlockPart : Content
{
	BlockPart();
	BlockPart(const BlockPart&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual ContentType GetContentType() const override;
	std::string  m_BlockName; /*341, no default value*/
	AcColor m_BlockColor; /*93*/
	CDblPoint m_BlockPosition; /*15, no default value, sixteen 47 for 4x4 matrix is derived from this*/
	CDblPoint m_BlockScale; /*16*/
	double m_BlockRotation; /*46, in radians, 13 = cos, 23 = sin*/
};

struct DXF_API CONTEXT_DATA
{
	CONTEXT_DATA();
	CONTEXT_DATA(const CONTEXT_DATA&) = default;
	void WriteDxf(DxfWriter& writer);
	CDblPoint m_LandingPosition; /* 10, no default value*/
	double m_TextHeight; /* 41*/
	double m_ArrowSize; /* 140*/
	double m_LandingGap; /* 145*/
	/* 290 hasMText, 296 hasBlock*/
	std::shared_ptr<Content> m_Content;
	/*110, Mleader Plane Origin Point is first leader arrow point*/
	LeaderPart m_Leader;
};

// http://help.autodesk.com/view/ACD/2016/ENU/?guid=GUID-72D20B8C-0F5E-4993-BEB7-0FCF94F32BE0
// http://docs.autodesk.com/ACD/2010/ENU/AutoCAD%202010%20User%20Documentation/index.html?url=WS73099cc142f487551d92abb10dc573c45d-7bf1.htm,topicNumber=d0e117972
// A multileader object typically consists of an arrowhead, a horizontal landing, a leader line or curve, and either a multiline text object or a block.
struct DXF_API AcadMLeader : public EntAttribute
{
	AcadMLeader();
	AcadMLeader(const AcadMLeader&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CONTEXT_DATA m_ContextData;
	std::string m_LeaderStyle; /*340*/
	AcMLeaderType m_LeaderType; /*170*/
	std::string m_LineType; /*341*/
	AcLineWeight m_LineWeight; /*171*/
	bool m_EnableLanding; /*290*/
	bool m_EnableDogleg; /*291*/
	//double m_DoglegLength; /*41, this is ignored by ACAD, only LEADER::m_DoglegLength matters*/
	//double m_ArrowSize; /*42, this is ignored by ACAD, only CONTEXT_DATA::m_ArrowSize matters*/
	// ContentType m_ContentType; /*172*/
	//std::string m_TextStyle; /*343, this is ignored by ACAD, only MTEXT::m_TextStyle matters*/
	std::vector<std::shared_ptr<AcadAttDef>> m_AttrDefs; /*330*/
	//Text string value for an attribute definition, repeated once per attribute definition and applicable only for a block - type cell
	std::vector<std::string> m_AttrValues;/*302, ""*/
};

enum CellType { TEXT_CELL = 1, BLOCK_CELL = 2 };

struct DXF_API CellInTableStyle // Not CELLSTYLE, CELLSTYLE isn't implemented now
{
	CellInTableStyle();
	CellInTableStyle(const CellInTableStyle&) = default;
	void WriteDxf(DxfWriter& writer);

	double m_TextHeight; /*140, 0.18*/
	AcCellAlignment m_Alignment; /*170, acMiddleCenter*/
	AcColor m_TextColor; /*62, acByBlock*/
	AcColor m_BgColor; /*63, acWhite*/
	bool m_BgColorEnabled; /*283, false*/
	AcValueDataType m_CellDataType; /*90, acGeneral*/
	AcValueUnitType m_CellUnitType; /*91, acUnitless*/
									/*1, "", ???*/
	AcLineWeight m_BorderLineWeight[6]; /*274-279, acLnWtByBlock*/
	bool m_BorderVisible[6]; /*284-289, true*/
	AcColor m_BorderColor[6]; /*64-69, acByBlock*/
};

//http://docs.autodesk.com/ACD/2014/CSY/index.html?url=files/GUID-0DBCA057-9F6C-4DEB-A66F-8A9B3C62FB1A.htm,topicNumber=d30e702776
struct DXF_API TableStyle /*in OBJECTS section*/
{
	TableStyle();
	TableStyle(const TableStyle&) = default;
	void WriteDxf(DxfWriter& writer, const std::string& name, int handle);

	double m_HorCellMargin; /*40, 0.06*/
	double m_VerCellMargin; /*41, 0.06*/
	bool m_HasNoTitle; /*280, false*/
	bool m_HasNoColumnHeading; /*281, false*/
	std::string m_TextStyle; /*7, "Standard*/

	// Don't understand why there are many CellInTableStyle(s)
	std::vector<CellInTableStyle> m_Cells;
};

struct CellContent
{
	virtual ~CellContent() = 0 {}
	virtual void WriteDxf(DxfWriter& writer) = 0;
	virtual CellType GetContentType() const = 0;
};

struct DXF_API CellText : public CellContent
{
	CellText();
	CellText(const CellText&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual CellType GetContentType() const override;

	std::string m_Text;/*many 2 and one 1, such as {\fSimSun|b0|i0|c134|p2;王五}*/
					   /*many 303 and one 302, same as 2 and 1*/
	std::string m_TextStyle; /*7, ""*/
};

struct DXF_API CellBlock : public CellContent
{
	CellBlock();
	CellBlock(const CellBlock&) = default;
	virtual void WriteDxf(DxfWriter& writer) override;
	virtual CellType GetContentType() const override;

	std::string m_BlockName; /*340*/
	double m_BlockScale; /*144*/
	/*179, number of attribute definitions in the block*/
	std::vector<std::shared_ptr<AcadAttDef>> m_AttrDefs; /*331*/
	//Text string value for an attribute definition, repeated once per attribute definition and applicable only for a block - type cell
	std::vector<std::string> m_AttrValues;/*300, ""*/
};

struct DXF_API Cell
{
	Cell(CellType);
	Cell(const Cell&) = default;
	void WriteDxf(DxfWriter& writer);

	//Type m_Type; /*171*/
	int m_Flag; /*172, 0*/
	bool m_Merged; /*173, false*/
	bool m_AutoFit; /*174, false*/
	// These 2 fields is applicable for merging cells 
	int m_ColSpan; /*175*/
	int m_RowSpan; /*176*/
	int m_OverrideFlag1; /*91, 0*/ /*or values in AcCellProperty*/
	// A virtual edge is used when a grid line is shared by two cells.For example, if a table contains one row and two columns and it contains cell A and cell B, the central grid line contains the right edge of cell A and the left edge of cell B.One edge is real, and the other edge is virtual.The virtual edge points to the real edge; both edges have the same set of properties, including color, lineweight, and visibility.
	int m_VirtualEdgeFlag; /*178, 0*/
	double m_Rotation; /*145, 0.0, in radians, applicable for TEXT and BLOCK*/
	AcCellAlignment m_Alignment; /*170, 0*/
	AcColor m_BgColor; /*63*/
	AcColor m_TextColor; /*64*/
	bool m_BgColorEnabled; /*283, false*/
	double m_TextHeight; /*140, 0.0*/
	// 92, 0, Extended cell flags

	/*301, CELL_VALUE*/
	int m_Flag93;/*93, 6, ???*/
	int m_Flag90;/*90, 4, ???*/
	int m_OverrideFlag2;/*91, 0, ???*/
	std::shared_ptr<CellContent> m_Content;
	bool m_LeftBorderVisible; /*288, true*/
	bool m_RightBorderVisible; /*285, true*/
	bool m_TopBorderVisible; /*289, true*/
	bool m_BottomBorderVisible; /*286, true*/
	/*304, ACVALUE_END*/
};

//http://help.autodesk.com/view/ACD/2016/ENU/?guid=GUID-D8CCD2F0-18A3-42BB-A64D-539114A07DA0
struct DXF_API AcadTable : public EntAttribute
{
	AcadTable();
	AcadTable(const AcadTable&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	// These are convinient functions to construct a table object.
	void InitializeCells(int rowCount, int colCount, CellType ct);
	void SetRowHeight(double height);
	void SetColWidth(double width);
	// It is programmer's responsibilty to ensure merged cells are not overlapped.
	void MergeCells(int row, int col, int rowSpan, int colSpan);
	Cell& GetCell(int row, int col);
	CellText& GetCellText(int row, int col);
	CellBlock& GetCellBlock(int row, int col);

	/*??? 160, 3512*/
	/*100, AcDbBlockReference*/
	std::string m_TableBlockName;/*2, optional anonymous block begins with a *T value to render the whole table*/
	CDblPoint m_InsertionPoint; /*10*/
	/*100, AcDbTable*/
	/*280, 0, 2010*/
	std::string m_TableStyle; /*342*/
	/*343, *T Block handle*/
	/*11, 1.0, 0.0, Horizontal direction vector*/
	/*90, 22, Flag for table value (unsigned integer)*/
	int m_RowCount; /*91*/
	int m_ColCount; /*92*/
	/*93, 0, Flag for an override*/
	/*94, 0, Flag for an override of border color*/
	/*95, 0, Flag for an override of border lineweight*/
	/*96, 0, Flag for an override of border visibility*/
	std::vector<double> m_RowHeights; /*141*/
	std::vector<double> m_ColWidths; /*142*/
	// There must be m_RowCount * m_ColCount Cell(s), for merged cells which m_Merged is true.*/
	std::vector<Cell> m_Cells;
	double m_HorCellMargin; /*40, 0.0*/
	double m_VerCellMargin; /*41, 0.0*/
	bool m_LeftBorderVisible; /*288, true*/
	bool m_RightBorderVisible; /*285, true*/
	bool m_TopBorderVisible; /*289, true*/
	bool m_BottomBorderVisible; /*286, true*/
};

struct DXF_API AcadLine : public EntAttribute
{
	AcadLine();
	AcadLine(const AcadLine&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_StartPoint;
	CDblPoint m_EndPoint;
};

struct DXF_API AcadLWPLine : public EntAttribute
{
	AcadLWPLine();
	AcadLWPLine(const AcadLWPLine&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	DblPoints m_Vertices;
	bool m_Closed;

	void SetConstWidth(double width)
	{
		m_Width = width;
		m_IsWidthValid = true;
	}
	void SetConstWidth()
	{
		m_Width = 0;
		m_IsWidthValid = false;
	}
	bool IsConstWidth() { return m_IsWidthValid; }
	double GetConstWdith() { return m_IsWidthValid ? m_Width : 0.0; }
	void SetBulge(size_t Index, double Bulge);
	void PushBulge(double bulge);
	double GetBulge(size_t Index) const;
	void SetBulges(std::vector<double> bulges);
	bool HasBulges() const { return !m_Bulges.empty(); }

private:
	double m_Width;
	bool   m_IsWidthValid;
	std::vector<double> m_Bulges;
};

struct DXF_API AcadMText : public EntAttribute
{
	AcadMText();
	AcadMText(const AcadMText&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_InsertionPoint;//对齐点的坐标
	double m_Width;
	double m_CharHeight;//值为0.0时表示使用文字样式中的高度
	std::string m_Text;// such as hello\Pworld, \P stands for new line.
	double m_LineSpacingFactor;//行距系数为０时表示使用至少行距风格acLineSpacingStyleAtLeast
	double m_RotationAngle;//in degrees
	AcAttachmentPoint m_AttachmentPoint;
	AcDrawingDirection m_DrawingDirection;
	std::string m_StyleName;
};

struct DXF_API AcadPoint : public EntAttribute
{
	AcadPoint();
	AcadPoint(const AcadPoint&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_Point;
};

struct DXF_API AcadSolid : public EntAttribute
{
	AcadSolid();
	AcadSolid(const AcadSolid&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_Point1, m_Point2, m_Point3, m_Point4;
};

struct DXF_API AcadSpline : public EntAttribute
{
	AcadSpline();
	AcadSpline(const AcadSpline&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	//样条曲线标志（按位编码）：
	//	1 = 闭合样条曲线
	//	2 = 周期性样条曲线
	//	4 = 有理样条曲线
	//	8 = 平面
	//	16 = 线性（同时还设置平面位）
	int m_Flag;
	int m_Degree;
	CDblPoint m_StartTangent;
	CDblPoint m_EndTangent;
	std::vector<double> m_Knots;
	std::vector<double> m_Weights;
	std::vector<CDblPoint> m_ControlPoints;
	std::vector<CDblPoint> m_FitPoints;
};

struct DXF_API AcadText : public EntAttribute
{
	AcadText();
	AcadText(const AcadText&) = default;
	void SetObliqueAngle(double a)
	{
		m_ObliqueAngle = a;
		m_IsObliqueAngleValid = true;
	}
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	// m_BaseLeftPoint可以从m_InsertionPoint, m_HorAlign, m_VerAlign等信息计算出。
	CDblPoint m_BaseLeftPoint;//文字左下角点
	CDblPoint m_InsertionPoint;//是文字对齐点，而非文字左下角点
	std::string m_Text;
	double m_RotationAngle;//in degrees
	//文字水平对正类型（可选；默认值 = 0）整数代码（非按位编码）
	//	0 = 左对正；1 = 居中对正；2 = 右对正
	//	3 = 对齐（如果垂直对齐 = 0）
	//	4 = 中间（如果垂直对齐 = 0）
	//	5 = 两端对齐（如果垂直对齐 = 0）
	short m_HorAlign;
	//文字垂直对正类型（可选；默认值 = 0）整数代码（不是按位编码）
	//	0 = 基线对正；1 = 底端对正；2 = 居中对正；3 = 顶端对正
	short m_VerAlign;
	//VerAlign		HorAlign
	//				0		1		2		3		4		5
	//0-基线对正		左		中		右		对齐		中间		两端对齐
	//1-底端对正		左下		中下		右下
	//2-居中对正		左中		正中		右中
	//3-顶端对正		左上		中上		右上

	std::string m_StyleName;
	double m_Height;//值为0.0时表示使用文字样式中的高度
	double m_WidthFactor;//值为0.0时表示使用文字样式中的宽度系数

private:
	bool m_IsObliqueAngleValid;//值为false时表示使用文字样式中的文字倾角
	double m_ObliqueAngle;//in degrees
};

struct DXF_API AcadViewport : public EntAttribute
{
	AcadViewport();
	AcadViewport(const AcadViewport&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_PaperspaceCenter;
	double m_PaperspaceWidth;
	double m_PaperspaceHeight;
	CDblPoint m_ModelSpaceCenter;
	double m_ModelSpaceHeight;
	double m_TwistAngle;//in degrees
    bool m_locked;

	// if m_ClipEnt is valid, viewport will be clipped by an entity, usually a polygon.
	std::weak_ptr<EntAttribute> m_ClipEnt;
};

struct DXF_API AcadXLine : public EntAttribute
{
	AcadXLine();
	AcadXLine(const AcadXLine&) = default;
	virtual void WriteDxf(DxfWriter& writer, bool bInPaperSpace) override;

	CDblPoint m_First;
	CDblPoint m_Second;
};

struct DXF_API LayoutData
{
	LayoutData();
	void WriteDxf(DxfWriter& writer);
    // convenient functions to add polygonal AcadViewport
    void AddPolygonalViewport(const DblPoints & polygonPS, const CDblPoint & centerMS, double heighMS, double twistAngle/*degrees*/);
    void AddPolygonalViewportByWidth(const DblPoints & polygonMS, const CDblPoint & centerPS, double widthPS, double twistAngle/*degrees*/);
    void AddPolygonalViewportByScale(const DblPoints & polygonMS, const CDblPoint & centerPS, double scale, double twistAngle/*degrees*/);

	// Must be *Paper_Space, *Paper_Space0, *Paper_Space1, ...
	// Don't set it if you create DxfData manually. DxfWriter will set it.
	std::string m_BlockName;
	PlotSettings m_PlotSettings;
	// m_MinLim and m_MaxLim defines the limits in paperspace.
	CDblPoint m_MinLim; // in millimeters
	CDblPoint m_MaxLim; // in millimeters
	// m_MinExt and m_MaxExt defines the default viewport size in paperspace.
	CDblPoint m_MinExt; // in millimeters
	CDblPoint m_MaxExt; // in millimeters
	// Line, arc, dims, ... and AcadViewport objects
	EntityList m_Objects;
	// Don't set it if you create DxfData manually. DxfWriter will set it.
	int m_LayoutOrder;
};

struct DXF_API BlockDef
{
	BlockDef() = default;

	CDblPoint m_InsertPoint;
	EntityList m_Objects;
};

}