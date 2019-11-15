/***************************************************************************
* Copyright (C) 2017, Deping Chen, cdp97531@sina.com
*
* All rights reserved.
* For permission requests, write to the publisher.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
***************************************************************************/
using System.Collections.Generic;
using System.Collections;
using System;

namespace DXF
{

    using DblPoints = List<CDblPoint>;
    using VectorOfEntityList = List<EntityList>;
    enum BoundaryEntType
    {
        Line = 1, Arc, EllipseArc, Spline
    };

    public partial class Global
    {
        private static string[] PredefinedArrowheadBlock = { "_Solid", "_ClosedBlank", "_Closed", "_Dot", "_ArchTick", "_Oblique", "_Open", "_Origin", "_Origin2", "_Open90", "_Open30", "_DotSmall", "_DotBlank", "_Small", "_BoxBlank", "_BoxFilled", "_DatumBlank", "_DatumFilled", "_Integral", "_None" };

        public static string GetPredefinedArrowHeadBlockName(AcDimArrowheadType predefinedArrowheadType)
        {
            //Global.ASSERT_DEBUG_INFO("predefinedArrowheadType >= 0 && predefinedArrowheadType < PredefinedArrowheadBlock.Length", predefinedArrowheadType >= 0 && predefinedArrowheadType < PredefinedArrowheadBlock.Length);
            return PredefinedArrowheadBlock[(int)predefinedArrowheadType];
        }

        public static AcDimArrowheadType GetArrowHeadType(string value)
        {
            for (int i = 0; i < PredefinedArrowheadBlock.Length; ++i)
            {
                if (string.Compare(PredefinedArrowheadBlock[i], value) == 0)
                {
                    return (AcDimArrowheadType)i;
                }
            }
            return AcDimArrowheadType.acArrowUserDefined;
        }

    }

    public class NameHandle
    {
        public string name;
        public int handle;
    }

    public class LayerData
    {
        public LayerData()
        {
            this.m_Linetype = "Continuous";
            this.m_Color = AcColor.acWhite;
            this.m_LineWeight = AcLineWeight.acLnWtByLwDefault;
            this.m_Plottable = false;
        }
        public string m_Linetype;
        public AcColor m_Color;
        public AcLineWeight m_LineWeight;
        public bool m_Plottable;
    }

    public class TextStyleData
    {
        public TextStyleData()
        {
            this.m_PrimaryFontFile = "txt.shx";
            this.m_BigFontFile = "";
            this.m_Height = 2.5;
            this.m_WidthFactor = 1.0;
            this.m_ObliqueAngle = 0;
        }

        public string m_PrimaryFontFile; //.shx file used for single byte character.
        public string m_BigFontFile; //.shx file used for multi-byte character.

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
        public string m_TrueType = string.Empty; // SimSun, SimHei, etc
        public double m_Height;
        public double m_WidthFactor;
        public double m_ObliqueAngle; //in degrees within the range of -85 to +85 degrees.
    }

    //枚举值的顺序应该与DimStyleData的成员变量一致
    enum MemIndex
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

    public class DimStyleData
    {
        public DimStyleData()
        {
            this.DimensionLineColor = AcColor.acByBlock;
            this.DimensionLineWeight = AcLineWeight.acLnWtByBlock;
            this.ExtensionLineColor = AcColor.acByBlock;
            this.ExtensionLineWeight = AcLineWeight.acLnWtByBlock;
            this.ExtensionLineExtend = 0;
            this.ExtensionLineOffset = 0;
            this.ExtLine1Suppress = false;
            this.ExtLine2Suppress = false;
            this.DimLine1Suppress = false;
            this.DimLine2Suppress = false;
            this.ArrowHead1Type = AcDimArrowheadType.acArrowDefault;
            this.ArrowHead2Type = AcDimArrowheadType.acArrowDefault;
            this.ArrowHeadSize = 0.18;
            this.CenterType = AcDimCenterType.acCenterMark;
            this.CenterMarkSize = 0.18;
            this.TextHeight = 0.18;
            this.TextColor = AcColor.acByBlock;
            this.VerticalTextPosition = AcDimVerticalJustification.acVertCentered;
            this.HorizontalTextPosition = AcDimHorizontalJustification.acHorzCentered;
            this.TextAlign = false;
            this.TextGap = 0.045;
            this.Fit = AcDimFit.acBestFit;
            this.TextMovement = AcDimTextMovement.acMoveTextNoLeader;
            this.TextInside = false;
            this.ForceLineInside = true;
            this.UnitsFormat = AcDimLUnits.acDimLDecimal;
            this.UnitsPrecision = AcDimPrecision.acDimPrecisionTwo;
            this.SuppressLeadingZeros = false;
            this.SuppressTrailingZeros = true;
            this.AngleSuppressLeadingZeros = false;
            this.AngleSuppressTrailingZeros = true;
            this.AngleFormat = AcAngleUnits.acDegrees;
            this.AnglePrecision = AcDimPrecision.acDimPrecisionOne;
            this.LinearScaleFactor = 1;
        }

        public DimStyleData(int UnnamedParameter)
        {
            this.DimensionLineColor = AcColor.acByBlock;
            this.DimensionLineWeight = AcLineWeight.acLnWtByBlock;
            this.ExtensionLineColor = AcColor.acByBlock;
            this.ExtensionLineWeight = AcLineWeight.acLnWtByBlock;
            this.ExtensionLineExtend = 1.25;
            this.ExtensionLineOffset = 0.625;
            this.ExtLine1Suppress = false;
            this.ExtLine2Suppress = false;
            this.DimLine1Suppress = false;
            this.DimLine2Suppress = false;
            this.ArrowHead1Type = AcDimArrowheadType.acArrowDefault;
            this.ArrowHead2Type = AcDimArrowheadType.acArrowDefault;
            this.ArrowHeadSize = 2.5;
            this.CenterType = AcDimCenterType.acCenterMark;
            this.CenterMarkSize = 2.5;
            this.TextStyle = "Standard";
            this.TextHeight = 2.5;
            this.TextColor = AcColor.acWhite;
            this.VerticalTextPosition = AcDimVerticalJustification.acAbove;
            this.HorizontalTextPosition = AcDimHorizontalJustification.acHorzCentered;
            this.TextAlign = false;
            this.TextGap = 0.625;
            this.Fit = AcDimFit.acBestFit;
            this.TextMovement = AcDimTextMovement.acDimLineWithText;
            this.TextInside = false;
            this.ForceLineInside = true;
            this.UnitsFormat = AcDimLUnits.acDimLDecimal;
            this.UnitsPrecision = AcDimPrecision.acDimPrecisionTwo;
            this.SuppressLeadingZeros = false;
            this.SuppressTrailingZeros = true;
            this.AngleSuppressLeadingZeros = false;
            this.AngleSuppressTrailingZeros = false;
            this.AngleFormat = AcAngleUnits.acDegrees;
            this.AnglePrecision = AcDimPrecision.acDimPrecisionOne;
            this.LinearScaleFactor = 1.0;
        }

        //尺寸线
        public AcColor DimensionLineColor; //初始值:AcColor.acByBlock
        public AcLineWeight DimensionLineWeight; //初始值:AcLineWeight.acLnWtByBlock
        public bool DimLine1Suppress; //初始值:false
        public bool DimLine2Suppress; //初始值:false
                                      //true: 即使当文字绘制在尺寸界线之外，也要把尺寸线置于尺寸界线之内。 
                                      //false: 当文字绘制在尺寸界线之外时，不强制把尺寸线置于尺寸界线之内。
                                      //初始值:true
        public bool ForceLineInside;

        //尺寸界线
        public AcColor ExtensionLineColor; //初始值:AcColor.acByBlock
        public AcLineWeight ExtensionLineWeight; //初始值:AcLineWeight.acLnWtByBlock
        public double ExtensionLineExtend; //指定尺寸界线超出尺寸线的长度//初始值:0
        public double ExtensionLineOffset; //指定尺寸界线从尺寸界线根点处的偏移//初始值:0
        public bool ExtLine1Suppress; //true--不绘制第一条尺寸界线//初始值:false
        public bool ExtLine2Suppress; //true--不绘制第二条尺寸界线//初始值:false

        //箭头
        public AcDimArrowheadType ArrowHead1Type; //取值范围：enum AcDimArrowheadType//初始值:AcDimArrowheadType.acArrowDefault
        public AcDimArrowheadType ArrowHead2Type; //取值范围：enum AcDimArrowheadType//初始值:AcDimArrowheadType.acArrowDefault
        public string ArrowHead1Block; //当Arrowhead1Type = acArrowUserDefined时起作用
        public string ArrowHead2Block; //当Arrowhead2Type = acArrowUserDefined时起作用
        public double ArrowHeadSize; //初始值:0.18
        public AcDimCenterType CenterType; //取值范围：enum AcDimCenterType//初始值:AcDimCenterType.acCenterMark
        public double CenterMarkSize; //初始值:0.09

        //文字
        public string TextStyle; //尺寸文字和公差文字的样式
        public double TextHeight; //当TextStyle有固定字高时，这个属性被忽略。//初始值:0.18
        public AcColor TextColor; //初始值:AcColor.acByBlock
                              //取值范围：enum AcDimVerticalJustification//初始值:AcDimVerticalJustification.acVertCentered
        public AcDimVerticalJustification VerticalTextPosition;
        //取值范围：enum AcDimHorizontalJustification//初始值:AcDimHorizontalJustification.acHorzCentered
        public AcDimHorizontalJustification HorizontalTextPosition;
        public bool TextAlign; //指定文字方向。true--水平,false--与尺寸线对齐//初始值:false
                               //当为了容纳标注文字而断开尺寸线时，指定标注文字和尺寸线间的距离。//初始值:0.045
                               //当文字位于尺寸线上方时，指定了标注文字和尺寸线间的距离。
        public double TextGap;
        //如果为空或为"<>"，标注测量将绘制为文字；如果为" "（一个空格），文字将被抑制。
        //例如"Ф<>mm"。
        public string Text = string.Empty;
        //调整
        //取值范围：enum AcDimFit，指定当尺寸界线内空间不足时采取的调整措施
        public AcDimFit Fit; //初始值:AcDimFit.acBestFit
                        //指定移动文字时如何调整尺寸线，对程序出图没有影响，但影响对所出图编辑，应采用缺省值。
                        //取值范围：enum AcDimTextMovement
        public AcDimTextMovement TextMovement; //初始值:AcDimTextMovement.acMoveTextNoLeader
                                 //true: 强制标注文字位于尺寸界线内。 
                                 //false: 仅当有足够空间时把标注文字置于尺寸界线内。
                                 //仅对线性和角度标注起作用，对半径和直径标注不起作用。
                                 //初始值:false
        public bool TextInside;

        //主单位
        //取值范围：enum AcDimLUnits,指定除角度外所有标注的单位格式//初始值:AcDimLUnits.acDimLDecimal
        public AcDimLUnits UnitsFormat;
        //取值范围：enum AcDimPrecision,指定除角度外所有标注的小数位数
        public AcDimPrecision UnitsPrecision; //初始值:AcDimPrecision.acDimPrecisionTwo
        public bool SuppressLeadingZeros; //抑制标注值的前导0//初始值:false
        public bool SuppressTrailingZeros; //抑制标注值的后缀0//初始值:true
        public bool AngleSuppressLeadingZeros; //抑制标注值的前导0//初始值:false
        public bool AngleSuppressTrailingZeros; //抑制标注值的后缀0//初始值:true
        public AcAngleUnits AngleFormat; //取值范围：AcAngleUnits//指定角度标注的单位格式//初始值:AcAngleUnits.acDegrees
        public AcDimPrecision AnglePrecision; //指定角度标注文本的精度//初始值:AcDimPrecision.acDimPrecisionOne
        public double LinearScaleFactor; //除角度标注外所有标注度量的比例因子//初始值:1
    }

    public class PlotSettings
    {
        public PlotSettings()
        {
            this.m_PlotConfigFile = "none_device";
            this.m_PaperName = "ISO_A4_(210.00_x_297.00_MM)";
            this.m_LeftMargin = 7.5;
            this.m_BottomMargin = 20;
            this.m_RightMargin = 7.5;
            this.m_TopMargin = 20;
            this.m_Width = 210;
            this.m_Height = 297;
            this.m_PlotOrigin = new CDblPoint(0, 0);
            this.m_PlotRotation = 1;
        }

        /*系统打印机或打印配置文件的名称*/
        public string m_PlotConfigFile;

        //图纸名称
        public string m_PaperName;

        /*图纸尺寸*/    //in millimeter
        public double m_LeftMargin;
        public double m_BottomMargin;
        public double m_RightMargin;
        public double m_TopMargin;
        public double m_Width;
        public double m_Height;

        /*! 打印旋转：
        0 = 无旋转
        1 = 逆时针旋转 90 度
        2 = 颠倒
        3 = 顺时针旋转 90 度
        */
        public int m_PlotRotation;

        public CDblPoint m_PlotOrigin = new CDblPoint();
    }

    public class EntAttribute
    {
        public EntAttribute()
        {
            this.m_Handle = 0;
            this.m_IsInPaperspace = false;
            this.m_Color = AcColor.acByLayer;
            this.m_Lineweight = AcLineWeight.acLnWtByLayer;
            this.m_LinetypeScale = 1.0;
            this.m_Layer = "0";
            this.m_Linetype = "ByLayer";
        }

        internal virtual void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
        }

        public virtual void AssignHandle(ref int handle)
        {
            m_Handle = ++handle;
        }

        public void WriteAttribute(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfHex(5, m_Handle);
            List<EntAttribute> reactors = new List<EntAttribute>();
            foreach (var reactorWeakPtr in m_Reactors)
            {
                var reactor = reactorWeakPtr.Target as EntAttribute;
                if (reactor != null)
                {
                    reactors.Add(reactor);
                }
            }
            if (reactors.Count > 0)
            {
                writer.dxfString(102, "{ACAD_REACTORS");
                foreach (var reactor in reactors)
                {
                    writer.dxfHex(330, reactor.m_Handle);
                }
                writer.dxfString(102, "}");
            }
            writer.dxfHex(330, writer.m_CurrentBlockEntryHandle);
            writer.dxfString(100, "AcDbEntity");
            if (bInPaperSpace)
            {
                writer.dxfInt(67, 1);
            }
            writer.dxfString(8, m_Layer);
            if (m_Linetype != "ByLayer" && !string.IsNullOrEmpty(m_Linetype))
            {
                writer.dxfString(6, m_Linetype);
            }
            if (m_Color != AcColor.acByLayer)
            {
                writer.dxfInt(62, (int)m_Color);
            }
            if (m_Lineweight != AcLineWeight.acLnWtByLayer)
            {
                writer.dxfInt(370, (int)m_Lineweight);
            }
            if (m_LinetypeScale != 1.0)
            {
                writer.dxfReal(48, m_LinetypeScale);
            }
        }

        public int m_Handle; // m_Handle will be assigned value on writing.
        public bool m_IsInPaperspace;
        public AcColor m_Color;
        public AcLineWeight m_Lineweight;
        public double m_LinetypeScale;
        public string m_Layer;
        public string m_Linetype;
        public List<WeakReference> m_Reactors = new List<WeakReference>();
    }

    public class EntityList : List<EntAttribute>
    {
        public int FindEntity(int handle)
        {
            return FindIndex((EntAttribute pEnt) => { return pEnt.m_Handle == handle; });
        }
    }

    public class AcadCircle : EntAttribute
    {
        public AcadCircle()
        {
            this.m_Radius = 0;
        }
        public AcadCircle(AcadCircle c)
        {
            this.m_Center.SetPoint(c.m_Center);
            this.m_Radius = c.m_Radius;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "CIRCLE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbCircle");
            writer.coord(10, m_Center.x, m_Center.y);
            writer.dxfReal(40, m_Radius);
        }

        public CDblPoint m_Center = new CDblPoint();
        public double m_Radius;
    }

    public class AcadArc : AcadCircle
    {
        public AcadArc()
        {
            this.m_StartAngle = 0;
            this.m_EndAngle = 360;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
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

        public double m_StartAngle; //in degrees
        public double m_EndAngle; //in degrees
    }

    public class AcadAttDef : EntAttribute
    {
        public AcadAttDef()
        {
            this.m_TextHeight = 0.25;
            this.m_Flags = AcAttributeMode.acAttributeModeNormal;
            this.m_RotationAngle = 0.0;
            this.m_HorAlign = 0;
            this.m_VerAlign = 0;
            this.m_DuplicateFlag = 1;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "ATTDEF");

            WriteAttribute(writer, bInPaperSpace);

            writer.dxfString(100, "AcDbText");
            if (m_HorAlign == 0 && m_VerAlign == 0)
            {
                writer.coord(10, m_BaseLeftPoint.x, m_BaseLeftPoint.y);
            }
            else
            {
                writer.coord(11, m_InsertionPoint.x, m_InsertionPoint.y);
            }
            writer.dxfReal(40, m_TextHeight);
            writer.dxfString(1, m_Text);
            if (m_RotationAngle != 0.0)
            {
                writer.dxfReal(50, m_RotationAngle);
            }
            if (!string.IsNullOrEmpty(m_TextStyle))
            {
                writer.dxfString(7, m_TextStyle);
            }
            if (m_HorAlign != 0)
            {
                writer.dxfInt(72, m_HorAlign);
            }
            writer.dxfString(100, "AcDbAttributeDefinition");
            writer.dxfInt(280, 0);
            writer.dxfString(3, m_Prompt);
            writer.dxfString(2, m_Tag);
            writer.dxfInt(70, (int)m_Flags);
            if (m_VerAlign != 0)
            {
                writer.dxfInt(74, m_VerAlign);
            }
            writer.dxfInt(280, m_DuplicateFlag);
        }

        // See AcadText for explanation.
        public CDblPoint m_BaseLeftPoint = new CDblPoint(); //10
        public CDblPoint m_InsertionPoint = new CDblPoint(); //11
        public double m_TextHeight; //40
        public string m_Text; //1
        public string m_Tag; //2, cannot contain spaces
        public AcAttributeMode m_Flags; //70
        public double m_RotationAngle; //50, in degrees
        public string m_TextStyle; //7
        public short m_HorAlign; //72
        public short m_VerAlign; //74
        public int m_DuplicateFlag; // 1 = Keep existing
        public string m_Prompt; //3
    }

    public class AcadAttrib : EntAttribute
    {
        public AcadAttrib()
        {
            this.m_TextHeight = 0.25;
            this.m_Flags = AcAttributeMode.acAttributeModeNormal;
            this.m_RotationAngle = 0.0;
            this.m_HorAlign = 0;
            this.m_VerAlign = 0;
            this.m_DuplicateFlag = 1;
        }

        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            // Not used.
            Global.ASSERT_DEBUG_INFO("Not used.", false);
        }

        internal void WriteDxf(DxfWriter writer, bool bInPaperSpace, int parentHandle)
        {
            writer.dxfString(0, "ATTRIB");

            //WriteAttribute(writer, bInPaperSpace);
            writer.dxfHex(5, m_Handle);
            writer.dxfHex(330, parentHandle);
            writer.dxfString(100, "AcDbEntity");
            if (bInPaperSpace)
            {
                writer.dxfInt(67, 1);
            }
            writer.dxfString(8, m_Layer);

            writer.dxfString(100, "AcDbText");
            if (m_HorAlign == 0 && m_VerAlign == 0)
            {
                writer.coord(10, m_BaseLeftPoint.x, m_BaseLeftPoint.y);
            }
            else
            {
                writer.coord(11, m_InsertionPoint.x, m_InsertionPoint.y);
            }
            writer.dxfReal(40, m_TextHeight);
            writer.dxfString(1, m_Text);
            if (m_RotationAngle != 0.0)
            {
                writer.dxfReal(50, m_RotationAngle);
            }
            if (!string.IsNullOrEmpty(m_TextStyle))
            {
                writer.dxfString(7, m_TextStyle);
            }
            if (m_HorAlign != 0)
            {
                writer.dxfInt(72, m_HorAlign);
            }
            writer.dxfString(100, "AcDbAttribute");
            writer.dxfInt(280, 0);
            writer.dxfString(2, m_Tag);
            writer.dxfInt(70, (int)m_Flags);
            if (m_VerAlign != 0)
            {
                writer.dxfInt(74, m_VerAlign);
            }
            writer.dxfInt(280, m_DuplicateFlag);
        }


        // See AcadText for explanation.
        public CDblPoint m_BaseLeftPoint = new CDblPoint(); //10
        public CDblPoint m_InsertionPoint = new CDblPoint(); //11
        public double m_TextHeight; //40
        public string m_Text; //1
        public string m_Tag; //2, cannot contain spaces
        public AcAttributeMode m_Flags; //70
        public double m_RotationAngle; //50, in degrees
        public string m_TextStyle; //7
        public short m_HorAlign; //72
        public short m_VerAlign; //74
        public int m_DuplicateFlag; // 1 = Keep existing
    }


    public class AcadBlockInstance : EntAttribute
    {
        public AcadBlockInstance()
        {
            this.m_Xscale = 1;
            this.m_Yscale = 1;
            this.m_Zscale = 1;
            this.m_RotationAngle = 0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "INSERT");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbBlockReference");
            if (m_Attribs.Count > 0)
            {
                writer.dxfInt(66, 1);
            }
            writer.dxfString(2, m_Name);
            writer.coord(10, m_InsertionPoint.x, m_InsertionPoint.y);
            writer.dxfReal(41, m_Xscale);
            writer.dxfReal(42, m_Yscale);
            writer.dxfReal(43, m_Zscale);
            if (m_RotationAngle != 0.0)
            {
                writer.dxfReal(50, m_RotationAngle);
            }

            if (m_Attribs.Count == 0)
            {
                return;
            }
            foreach (var attrib in m_Attribs)
            {
                attrib.WriteDxf(writer, bInPaperSpace, m_Handle);
            }

            // SEQEND
            writer.dxfString(0, "SEQEND");
            writer.dxfHex(5, m_Attribs[m_Attribs.Count - 1].m_Handle + 1);
            writer.dxfHex(330, m_Handle);
            writer.dxfString(100, "AcDbEntity");
            writer.dxfString(8, m_Layer);
        }
        public override void AssignHandle(ref int handle)
        {
            m_Handle = ++handle;
            if (m_Attribs.Count == 0)
            {
                return;
            }
            foreach (var attrib in m_Attribs)
            {
                attrib.m_Handle = ++handle;
            }
            // Handle for SEQEND
            ++handle;
        }

        public string m_Name;
        public CDblPoint m_InsertionPoint = new CDblPoint();
        public double m_Xscale;
        public double m_Yscale;
        public double m_Zscale;
        public double m_RotationAngle; //in degrees
        public List<AcadAttrib> m_Attribs = new List<AcadAttrib>();
    }

    public abstract class AcadDim : EntAttribute
    {
        public AcadDim()
        {
            this.m_DimStyleName = "ISO-25";
        }

        public void WriteHeader(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "DIMENSION");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbDimension");
            writer.dxfInt(280, 0); // 版本号：0 = 2010
            if (!string.IsNullOrEmpty(m_BlockName))
            {
                writer.dxfString(2, m_BlockName);
            }
            writer.coord(10, m_DefPoint.x, m_DefPoint.y);
            writer.coord(11, m_TextPosition.x, m_TextPosition.y);
        }

        public void WriteOverride(DxfWriter writer)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXT])
            {
                writer.dxfString(1, m_DimStyleOverride.Text);
            }
            writer.dxfString(3, m_DimStyleName);
            if (m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEOFFSET])
            {
                writer.dxfInt(1070, 42);
                writer.dxfReal(1040, m_DimStyleOverride.ExtensionLineOffset);
            }
            if (m_ValidMembersFlag[(int)MemIndex.HORIZONTALTEXTPOSITION])
            {
                writer.dxfInt(1070, 280);
                writer.dxfInt(1070, (int)m_DimStyleOverride.HorizontalTextPosition);
            }
            if (m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINECOLOR])
            {
                writer.dxfInt(1070, 176);
                writer.dxfInt(1070, (int)m_DimStyleOverride.DimensionLineColor);
            }
            if (m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINEWEIGHT])
            {
                writer.dxfInt(1070, 371);
                writer.dxfInt(1070, (int)m_DimStyleOverride.DimensionLineWeight);
            }
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEADSIZE])
            {
                writer.dxfInt(1070, 41);
                writer.dxfReal(1040, m_DimStyleOverride.ArrowHeadSize);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTHEIGHT])
            {
                writer.dxfInt(1070, 140);
                writer.dxfReal(1040, m_DimStyleOverride.TextHeight);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTSTYLE])
            {
                writer.dxfInt(1070, 340);
                writer.dxfHex(1005, (int)writer.LookupTextStyleHandle(m_DimStyleOverride.TextStyle));
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTCOLOR])
            {
                writer.dxfInt(1070, 178);
                writer.dxfInt(1070, (int)m_DimStyleOverride.TextColor);
            }
            if (m_ValidMembersFlag[(int)MemIndex.VERTICALTEXTPOSITION])
            {
                writer.dxfInt(1070, 145);
                writer.dxfInt(1070, (int)m_DimStyleOverride.VerticalTextPosition);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTALIGN])
            {
                writer.dxfInt(1070, 73);
                writer.dxfInt(1070, m_DimStyleOverride.TextAlign ? 1 : 0);
                writer.dxfInt(1070, 74);
                writer.dxfInt(1070, m_DimStyleOverride.TextAlign ? 1 : 0);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTGAP])
            {
                writer.dxfInt(1070, 147);
                writer.dxfReal(1040, m_DimStyleOverride.TextGap);
            }
            if (m_ValidMembersFlag[(int)MemIndex.FIT])
            {
                writer.dxfInt(1070, 289);
                writer.dxfInt(1070, (int)m_DimStyleOverride.Fit);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTINSIDE])
            {
                writer.dxfInt(1070, 174);
                writer.dxfInt(1070, m_DimStyleOverride.TextInside ? 1 : 0);
            }
            if (m_ValidMembersFlag[(int)MemIndex.FORCELINEINSIDE])
            {
                writer.dxfInt(1070, 172);
                writer.dxfInt(1070, m_DimStyleOverride.ForceLineInside ? 1 : 0);
            }
            if (m_ValidMembersFlag[(int)MemIndex.SUPPRESSLEADINGZEROS] || m_ValidMembersFlag[(int)MemIndex.SUPPRESSTRAILINGZEROS])
            {
                int flag = 0;
                if (m_DimStyleOverride.SuppressLeadingZeros)
                {
                    flag |= 1;
                }
                if (m_DimStyleOverride.SuppressTrailingZeros)
                {
                    flag |= 2;
                }
                if (this as AcadDimAng != null)

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
            if (m_ValidMembersFlag[(int)MemIndex.DIMLINE1SUPPRESS])
            {
                writer.dxfInt(1070, 281);
                writer.dxfInt(1070, m_DimStyleOverride.DimLine1Suppress ? 1 : 0);
            }
            if (m_ValidMembersFlag[(int)MemIndex.DIMLINE2SUPPRESS])
            {
                writer.dxfInt(1070, 282);
                writer.dxfInt(1070, m_DimStyleOverride.DimLine2Suppress ? 1 : 0);
            }
            if (m_ValidMembersFlag[(int)MemIndex.TEXTROTATION])
            {
                writer.dxfReal(53, m_TextRotation);
            }
        }

        //如果m_BlockName为空，DxfWriter需要生成标注图块，否则说明是从DXF文件读取的，已经读入显示标注图块(*D<n>)。
        public string m_BlockName;
        //定义点（在 WCS 中），定义点含义与标注类型相关。
        public CDblPoint m_DefPoint = new CDblPoint();
        //标注文字的中点（在 OCS 中）
        public CDblPoint m_TextPosition = new CDblPoint();
        //值 0 - 6 是表示标注类型的整数值。值 32、64 和 128 是添加到整数值中的位值（在 R13 及以后的版本中始终设置值 32）
        public int m_DimType;
        public AcAttachmentPoint m_Attachment;

        public string m_DimStyleName;
        //用来指示m_DimStyleOverride中某个成员变量(Name除外)/m_TextRotation的值是否有效。
        public BitArray m_ValidMembersFlag = new BitArray((int)MemIndex.LASTBIT);
        //m_DimStyleOverride.Name没有使用。
        public DimStyleData m_DimStyleOverride = new DimStyleData();
        //标注文字与其默认方向所成的旋转角度（尺寸线方向）（可选）
        public double m_TextRotation; //in degrees

        public double m_GlobalFactor = 1.0; // xdata of ACAD
        internal void WriteGlobalFactor(DxfWriter writer)
        {
            if (m_GlobalFactor != 1.0)
            {
                writer.dxfString(1001, "ACAD");
                writer.dxfString(1000, "DSTYLE");
                writer.dxfString(1002, "{");
                writer.dxfInt(1070, 40);
                writer.dxfReal(1040, m_GlobalFactor);
                writer.dxfString(1002, "}");
            }
        }

        #region Overrides
        public void OverrideDimensionLineColor(AcColor value)
        {
            m_DimStyleOverride.DimensionLineColor = value;
            m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINECOLOR] = true;
        }

        public void OverrideDimensionLineWeight(AcLineWeight value)
        {
            m_DimStyleOverride.DimensionLineWeight = value;
            m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINEWEIGHT] = true;
        }

        public void OverrideDimLine1Suppress(bool value)
        {
            m_DimStyleOverride.DimLine1Suppress = value;
            m_ValidMembersFlag[(int)MemIndex.DIMLINE1SUPPRESS] = true;
        }

        public void OverrideDimLine2Suppress(bool value)
        {
            m_DimStyleOverride.DimLine2Suppress = value;
            m_ValidMembersFlag[(int)MemIndex.DIMLINE2SUPPRESS] = true;
        }

        public void OverrideForceLineInside(bool value)
        {
            m_DimStyleOverride.ForceLineInside = value;
            m_ValidMembersFlag[(int)MemIndex.FORCELINEINSIDE] = true;
        }

        public void OverrideExtensionLineColor(AcColor value)
        {
            m_DimStyleOverride.ExtensionLineColor = value;
            m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINECOLOR] = true;
        }

        public void OverrideExtensionLineWeight(AcLineWeight value)
        {
            m_DimStyleOverride.ExtensionLineWeight = value;
            m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEWEIGHT] = true;
        }

        public void OverrideExtensionLineExtend(double value)
        {
            m_DimStyleOverride.ExtensionLineExtend = value;
            m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEEXTEND] = true;
        }

        public void OverrideExtensionLineOffset(double value)
        {
            m_DimStyleOverride.ExtensionLineOffset = value;
            m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEOFFSET] = true;
        }

        public void OverrideExtLine1Suppress(bool value)
        {
            m_DimStyleOverride.ExtLine1Suppress = value;
            m_ValidMembersFlag[(int)MemIndex.EXTLINE1SUPPRESS] = true;
        }

        public void OverrideExtLine2Suppress(bool value)
        {
            m_DimStyleOverride.ExtLine2Suppress = value;
            m_ValidMembersFlag[(int)MemIndex.EXTLINE2SUPPRESS] = true;
        }

        public void OverrideArrowHead1Type(AcDimArrowheadType value)
        {
            m_DimStyleOverride.ArrowHead1Type = value;
            m_ValidMembersFlag[(int)MemIndex.ARROWHEAD1TYPE] = true;
        }

        public void OverrideArrowHead2Type(AcDimArrowheadType value)
        {
            m_DimStyleOverride.ArrowHead2Type = value;
            m_ValidMembersFlag[(int)MemIndex.ARROWHEAD2TYPE] = true;
        }

        public void OverrideArrowHead1Block(string value)
        {
            m_DimStyleOverride.ArrowHead1Type = AcDimArrowheadType.acArrowUserDefined;
            m_DimStyleOverride.ArrowHead1Block = value;
            m_ValidMembersFlag[(int)MemIndex.ARROWHEAD1TYPE] = true;
        }

        public void OverrideArrowHead2Block(string value)
        {
            m_DimStyleOverride.ArrowHead2Type = AcDimArrowheadType.acArrowUserDefined;
            m_DimStyleOverride.ArrowHead2Block = value;
            m_ValidMembersFlag[(int)MemIndex.ARROWHEAD2TYPE] = true;
        }

        public void OverrideArrowHeadSize(double value)
        {
            m_DimStyleOverride.ArrowHeadSize = value;
            m_ValidMembersFlag[(int)MemIndex.ARROWHEADSIZE] = true;
        }

        public void OverrideCenterType(AcDimCenterType value)
        {
            m_DimStyleOverride.CenterType = value;
            m_ValidMembersFlag[(int)MemIndex.CENTERTYPE] = true;
        }

        public void OverrideCenterMarkSize(double value)
        {
            m_DimStyleOverride.CenterMarkSize = value;
            m_ValidMembersFlag[(int)MemIndex.CENTERMARKSIZE] = true;
        }

        public void OverrideTextStyle(string value)
        {
            m_DimStyleOverride.TextStyle = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTSTYLE] = true;
        }

        public void OverrideTextHeight(double value)
        {
            m_DimStyleOverride.TextHeight = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTHEIGHT] = true;
        }

        public void OverrideTextColor(AcColor value)
        {
            m_DimStyleOverride.TextColor = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTCOLOR] = true;
        }

        public void OverrideVerticalTextPosition(AcDimVerticalJustification value)
        {
            m_DimStyleOverride.VerticalTextPosition = value;
            m_ValidMembersFlag[(int)MemIndex.VERTICALTEXTPOSITION] = true;
        }

        public void OverrideHorizontalTextPosition(AcDimHorizontalJustification value)
        {
            m_DimStyleOverride.HorizontalTextPosition = value;
            m_ValidMembersFlag[(int)MemIndex.HORIZONTALTEXTPOSITION] = true;
        }

        public void OverrideTextAlign(bool value)
        {
            m_DimStyleOverride.TextAlign = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTALIGN] = true;
        }

        public void OverrideTextGap(double value)
        {
            m_DimStyleOverride.TextGap = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTGAP] = true;
        }

        public void OverrideText(string value)
        {
            m_DimStyleOverride.Text = value;
            m_ValidMembersFlag[(int)MemIndex.TEXT] = true;
        }

        public void OverrideFit(AcDimFit value)
        {
            m_DimStyleOverride.Fit = value;
            m_ValidMembersFlag[(int)MemIndex.FIT] = true;
        }

        public void OverrideTextMovement(AcDimTextMovement value)
        {
            m_DimStyleOverride.TextMovement = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTMOVEMENT] = true;
        }

        public void OverrideTextInside(bool value)
        {
            m_DimStyleOverride.TextInside = value;
            m_ValidMembersFlag[(int)MemIndex.TEXTINSIDE] = true;
        }

        public void OverrideUnitsFormat(AcDimLUnits value)
        {
            m_DimStyleOverride.UnitsFormat = value;
            m_ValidMembersFlag[(int)MemIndex.UNITSFORMAT] = true;
        }

        public void OverrideUnitsPrecision(AcDimPrecision value)
        {
            m_DimStyleOverride.UnitsPrecision = value;
            m_ValidMembersFlag[(int)MemIndex.UNITSPRECISION] = true;
        }

        public void OverrideSuppressLeadingZeros(bool value)
        {
            m_DimStyleOverride.SuppressLeadingZeros = value;
            m_ValidMembersFlag[(int)MemIndex.SUPPRESSLEADINGZEROS] = true;
        }

        public void OverrideSuppressTrailingZeros(bool value)
        {
            m_DimStyleOverride.SuppressTrailingZeros = value;
            m_ValidMembersFlag[(int)MemIndex.SUPPRESSTRAILINGZEROS] = true;
        }
        public void OverrideAngleSuppressLeadingZeros(bool value)
        {
            m_DimStyleOverride.AngleSuppressLeadingZeros = value;
            m_ValidMembersFlag[(int)MemIndex.ANGLESUPPRESSLEADINGZEROS] = true;
        }

        public void OverrideAngleSuppressTrailingZeros(bool value)
        {
            m_DimStyleOverride.AngleSuppressTrailingZeros = value;
            m_ValidMembersFlag[(int)MemIndex.ANGLESUPPRESSTRAILINGZEROS] = true;
        }

        public void OverrideAngleFormat(AcAngleUnits value)
        {
            m_DimStyleOverride.AngleFormat = value;
            m_ValidMembersFlag[(int)MemIndex.ANGLEFORMAT] = true;
        }

        public void OverrideAnglePrecision(AcDimPrecision value)
        {
            m_DimStyleOverride.AnglePrecision = value;
            m_ValidMembersFlag[(int)MemIndex.ANGLEPRECISION] = true;
        }

        public void OverrideLinearScaleFactor(double value)
        {
            m_DimStyleOverride.LinearScaleFactor = value;
            m_ValidMembersFlag[(int)MemIndex.LINEARSCALEFACTOR] = true;
        }

        public void OverrideTextRotation(double textRotation)
        {
            m_TextRotation = textRotation;
            m_ValidMembersFlag[(int)MemIndex.TEXTROTATION] = true;
        }
        #endregion

        #region Gets
        public AcColor GetDimensionLineColor(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINECOLOR])
            {
                return m_DimStyleOverride.DimensionLineColor;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.DimensionLineColor;
            }
            return AcColor.acByBlock;
        }

        public AcLineWeight GetDimensionLineWeight(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.DIMENSIONLINEWEIGHT])
            {
                return m_DimStyleOverride.DimensionLineWeight;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.DimensionLineWeight;
            }
            return AcLineWeight.acLnWtByBlock;
        }

        public bool GetDimLine1Suppress(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.DIMLINE1SUPPRESS])
            {
                return m_DimStyleOverride.DimLine1Suppress;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.DimLine1Suppress;
            }
            return false;
        }

        public bool GetDimLine2Suppress(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.DIMLINE2SUPPRESS])
            {
                return m_DimStyleOverride.DimLine2Suppress;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.DimLine2Suppress;
            }
            return false;
        }

        public bool GetForceLineInside(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.FORCELINEINSIDE])
            {
                return m_DimStyleOverride.ForceLineInside;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ForceLineInside;
            }
            return false;
        }

        public AcColor GetExtensionLineColor(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINECOLOR])
            {
                return m_DimStyleOverride.ExtensionLineColor;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtensionLineColor;
            }
            return AcColor.acByBlock;
        }

        public AcLineWeight GetExtensionLineWeight(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEWEIGHT])
            {
                return m_DimStyleOverride.ExtensionLineWeight;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtensionLineWeight;
            }
            return AcLineWeight.acLnWtByBlock;
        }

        public double GetExtensionLineExtend(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEEXTEND])
            {
                return m_DimStyleOverride.ExtensionLineExtend;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtensionLineExtend;
            }
            return 1.25;
        }

        public double GetExtensionLineOffset(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTENSIONLINEOFFSET])
            {
                return m_DimStyleOverride.ExtensionLineOffset;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtensionLineOffset;
            }
            return 0.625;
        }

        public bool GetExtLine1Suppress(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTLINE1SUPPRESS])
            {
                return m_DimStyleOverride.ExtLine1Suppress;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtLine1Suppress;
            }
            return false;
        }
        public bool GetExtLine2Suppress(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.EXTLINE2SUPPRESS])
            {
                return m_DimStyleOverride.ExtLine2Suppress;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ExtLine2Suppress;
            }
            return false;
        }

        public AcDimArrowheadType GetArrowHead1Type(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEAD1TYPE])
            {
                return m_DimStyleOverride.ArrowHead1Type;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ArrowHead1Type;
            }
            return AcDimArrowheadType.acArrowDefault;
        }

        public AcDimArrowheadType GetArrowHead2Type(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEAD2TYPE])
            {
                return m_DimStyleOverride.ArrowHead2Type;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ArrowHead2Type;
            }
            return AcDimArrowheadType.acArrowDefault;
        }

        public string GetArrowHead1Block(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEAD1TYPE])
            {
                if (m_DimStyleOverride.ArrowHead1Type == AcDimArrowheadType.acArrowUserDefined)
                {
                    return m_DimStyleOverride.ArrowHead1Block;
                }
                else
                {
                    return Global.GetPredefinedArrowHeadBlockName(m_DimStyleOverride.ArrowHead1Type);
                }
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                if (dsd.ArrowHead1Type == AcDimArrowheadType.acArrowUserDefined)
                {
                    return dsd.ArrowHead1Block;
                }
                else
                {
                    return Global.GetPredefinedArrowHeadBlockName(dsd.ArrowHead1Type);
                }
            }
            return Global.GetPredefinedArrowHeadBlockName(AcDimArrowheadType.acArrowDefault);
        }

        public string GetArrowHead2Block(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEAD2TYPE])
            {
                if (m_DimStyleOverride.ArrowHead2Type == AcDimArrowheadType.acArrowUserDefined)
                {
                    return m_DimStyleOverride.ArrowHead2Block;
                }
                else
                {
                    return Global.GetPredefinedArrowHeadBlockName(m_DimStyleOverride.ArrowHead2Type);
                }
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                if (dsd.ArrowHead2Type == AcDimArrowheadType.acArrowUserDefined)
                {
                    return dsd.ArrowHead2Block;
                }
                else
                {
                    return Global.GetPredefinedArrowHeadBlockName(dsd.ArrowHead2Type);
                }
            }
            return Global.GetPredefinedArrowHeadBlockName(AcDimArrowheadType.acArrowDefault);
        }

        public double GetArrowHeadSize(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ARROWHEADSIZE])
            {
                return m_DimStyleOverride.ArrowHeadSize;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.ArrowHeadSize;
            }
            return 2.5;
        }

        public AcDimCenterType GetCenterType(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.CENTERTYPE])
            {
                return m_DimStyleOverride.CenterType;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.CenterType;
            }
            return AcDimCenterType.acCenterMark;
        }

        public double GetCenterMarkSize(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.CENTERMARKSIZE])
            {
                return m_DimStyleOverride.CenterMarkSize;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.CenterMarkSize;
            }
            return 2.5;
        }
        public string GetTextStyle(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTSTYLE])
            {
                return m_DimStyleOverride.TextStyle;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextStyle;
            }
            return "Standard";
        }

        public double GetTextHeight(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTHEIGHT])
            {
                return m_DimStyleOverride.TextHeight;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextHeight;
            }
            return 2.5;
        }

        public AcColor GetTextColor(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTCOLOR])
            {
                return m_DimStyleOverride.TextColor;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextColor;
            }
            return AcColor.acByBlock;
        }

        public AcDimVerticalJustification GetVerticalTextPosition(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.VERTICALTEXTPOSITION])
            {
                return m_DimStyleOverride.VerticalTextPosition;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.VerticalTextPosition;
            }
            return AcDimVerticalJustification.acAbove;
        }

        public AcDimHorizontalJustification GetHorizontalTextPosition(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.HORIZONTALTEXTPOSITION])
            {
                return m_DimStyleOverride.HorizontalTextPosition;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.HorizontalTextPosition;
            }
            return AcDimHorizontalJustification.acHorzCentered;
        }

        public bool GetTextAlign(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTALIGN])
            {
                return m_DimStyleOverride.TextAlign;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextAlign;
            }
            return false;
        }

        public double GetTextGap(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTGAP])
            {
                return m_DimStyleOverride.TextGap;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextGap;
            }
            return 0.625;
        }

        public string GetText(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXT])
            {
                return m_DimStyleOverride.Text;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.Text;
            }
            return "";
        }

        public AcDimFit GetFit(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.FIT])
            {
                return m_DimStyleOverride.Fit;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.Fit;
            }
            return AcDimFit.acBestFit;
        }

        public AcDimTextMovement GetTextMovement(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTMOVEMENT])
            {
                return m_DimStyleOverride.TextMovement;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextMovement;
            }
            return AcDimTextMovement.acMoveTextNoLeader;
        }
        public bool GetTextInside(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.TEXTINSIDE])
            {
                return m_DimStyleOverride.TextInside;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.TextInside;
            }
            return false;
        }

        public AcDimLUnits GetUnitsFormat(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.UNITSFORMAT])
            {
                return m_DimStyleOverride.UnitsFormat;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.UnitsFormat;
            }
            return AcDimLUnits.acDimLDecimal;
        }

        public AcDimPrecision GetUnitsPrecision(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.UNITSPRECISION])
            {
                return m_DimStyleOverride.UnitsPrecision;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.UnitsPrecision;
            }
            return AcDimPrecision.acDimPrecisionTwo;
        }

        public bool GetSuppressLeadingZeros(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.SUPPRESSLEADINGZEROS])
            {
                return m_DimStyleOverride.SuppressLeadingZeros;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.SuppressLeadingZeros;
            }
            return false;
        }

        public bool GetSuppressTrailingZeros(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.SUPPRESSTRAILINGZEROS])
            {
                return m_DimStyleOverride.SuppressTrailingZeros;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.SuppressTrailingZeros;
            }
            return true;
        }

        public bool GetAngleSuppressLeadingZeros(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ANGLESUPPRESSLEADINGZEROS])
            {
                return m_DimStyleOverride.AngleSuppressLeadingZeros;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.AngleSuppressLeadingZeros;
            }
            return false;
        }

        public bool GetAngleSuppressTrailingZeros(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ANGLESUPPRESSTRAILINGZEROS])
            {
                return m_DimStyleOverride.AngleSuppressTrailingZeros;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.AngleSuppressTrailingZeros;
            }
            return true;
        }

        public AcAngleUnits GetAngleFormat(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ANGLEFORMAT])
            {
                return m_DimStyleOverride.AngleFormat;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.AngleFormat;
            }
            return AcAngleUnits.acDegrees;
        }

        public AcDimPrecision GetAnglePrecision(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.ANGLEPRECISION])
            {
                return m_DimStyleOverride.AnglePrecision;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.AnglePrecision;
            }
            return AcDimPrecision.acDimPrecisionOne;
        }

        public double GetLinearScaleFactor(DxfData graph)
        {
            if (m_ValidMembersFlag[(int)MemIndex.LINEARSCALEFACTOR])
            {
                return m_DimStyleOverride.LinearScaleFactor;
            }
            DimStyleData dsd;
            bool hasIt = graph.m_DimStyles.TryGetValue(m_DimStyleName, out dsd);
            if (hasIt)
            {
                return dsd.LinearScaleFactor;
            }
            return 1.0;
        }
        #endregion

        public abstract void GenerateBlock(DxfData graph, string blockName);
    }

    public class AcadDimInternal : AcadDim
    {
        public AcadDimInternal()
        {
            this.m_ReactorHandles = new List<int>();
        }

        public List<int> m_ReactorHandles;

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimAln : AcadDim
    {
        public AcadDimAln()
        {
            this.m_RotationAngle = 0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 1 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDbAlignedDimension");
            writer.coord(13, m_ExtLine1Point.x, m_ExtLine1Point.y);
            writer.coord(14, m_ExtLine2Point.x, m_ExtLine2Point.y);
            if (m_RotationAngle != 0.0)
            {
                writer.dxfReal(50, m_RotationAngle);
            }
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_ExtLine1Point = new CDblPoint();
        public CDblPoint m_ExtLine2Point = new CDblPoint();
        public double m_RotationAngle; //in degrees

        // To be implemented. But it is very hard and trivial.
        // For now, end user can select all entities by pressing ctrl+A,
        // move all entities to some point, all dims will show.
        // Move them back, it will be OK.
        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }


    public class AcadDimAng3P : AcadDim
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 5 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDb3PointAngularDimension");
            writer.coord(13, m_FirstEnd.x, m_FirstEnd.y);
            writer.coord(14, m_SecondEnd.x, m_SecondEnd.y);
            writer.coord(15, m_AngleVertex.x, m_AngleVertex.y);
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_FirstEnd = new CDblPoint(); // the endpoints of the extension lines
        public CDblPoint m_SecondEnd = new CDblPoint();
        public CDblPoint m_AngleVertex = new CDblPoint(); // the vertex of the angle

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimAng : AcadDim
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 2 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDb2LineAngularDimension");
            writer.coord(13, m_FirstStart.x, m_FirstStart.y);
            writer.coord(14, m_FirstEnd.x, m_FirstEnd.y);
            writer.coord(15, m_SecondStart.x, m_SecondStart.y);
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_FirstStart = new CDblPoint(); // the first extension line
        public CDblPoint m_FirstEnd = new CDblPoint();
        public CDblPoint m_SecondStart = new CDblPoint(); // the second extension line

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimDia : AcadDim
    {
        public AcadDimDia()
        {
            this.m_LeaderLength = 0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 3 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDbDiametricDimension");
            writer.coord(15, m_ChordPoint.x, m_ChordPoint.y);
            writer.dxfReal(40, m_LeaderLength);
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_ChordPoint = new CDblPoint();
        public double m_LeaderLength;

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimRad : AcadDim
    {
        public AcadDimRad()
        {
            this.m_LeaderLength = 0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 4 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDbRadialDimension");
            writer.coord(15, m_ChordPoint.x, m_ChordPoint.y);
            writer.dxfReal(40, m_LeaderLength);
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_ChordPoint = new CDblPoint();
        public double m_LeaderLength;

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimRot : AcadDimAln
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 0 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDbAlignedDimension");
            writer.coord(13, m_ExtLine1Point.x, m_ExtLine1Point.y);
            writer.coord(14, m_ExtLine2Point.x, m_ExtLine2Point.y);
            writer.dxfReal(50, m_RotationAngle);
            writer.dxfString(100, "AcDbRotatedDimension");
            WriteGlobalFactor(writer);
        }

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadDimOrd : AcadDim
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            WriteHeader(writer, bInPaperSpace);
            WriteOverride(writer);
            int type = 6 | 0x20;
            writer.dxfInt(70, type);
            writer.dxfInt(71, 5); //文字正中对齐

            writer.dxfString(100, "AcDbOrdinateDimension");
            writer.coord(13, m_OrdPoint.x, m_OrdPoint.y);
            writer.coord(14, m_LeaderPoint.x, m_LeaderPoint.y);
            WriteGlobalFactor(writer);
        }

        public CDblPoint m_OrdPoint = new CDblPoint();
        public CDblPoint m_LeaderPoint = new CDblPoint();

        public override void GenerateBlock(DxfData graph, string blockName)
        {
        }
    }

    public class AcadEllipse : EntAttribute
    {
        public AcadEllipse()
        {
            this.m_MinorAxisRatio = 0;
            this.m_StartAngle = 0;
            this.m_EndAngle = 2 * Math.PI;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
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

        public CDblPoint m_Center = new CDblPoint();
        public CDblPoint m_MajorAxisPoint = new CDblPoint();
        public double m_MinorAxisRatio;
        public double m_StartAngle; //in radians
        public double m_EndAngle; //in radians
    }

    public enum BoundaryPathFlag { Outer = 0x1, LWPline = 0x2, Export = 0x4, Text = 0x8, OuterMost = 0x10 };
    public class AcadHatch : EntAttribute
    {
        public AcadHatch()
        {
            this.m_FillFlag = AcadHatch.FillFlag.PatternFill;
            this.m_HatchStyle = AcHatchStyle.acHatchStyleNormal;
            this.m_PatternName = "ANSI31";
            this.m_PatternType = AcPatternType.acHatchPatternTypePreDefined;
            this.m_PatternScale = 1;
            this.m_PatternAngle = 0;
            this.m_PixelSize = 2.5;
            this.m_OuterFlag = BoundaryPathFlag.Outer | BoundaryPathFlag.Export;
        }

        public enum FillFlag
        {
            PatternFill,
            SolidFill
        } // 图案填充, 实体填充
        public FillFlag m_FillFlag;
        public AcHatchStyle m_HatchStyle;

        // These fields are only valid when m_FillFlag is PatternFill
        public string m_PatternName;
        public AcPatternType m_PatternType;
        public double m_PatternScale;
        public double m_PatternAngle; //in degrees
                                      // DXF code 47
        public double m_PixelSize;

        // AcadHatch owns these entities. They are just boundary data, not standalone entities.
        // When you write DXF, don't operate these 2 variables, use AddEntity method instead.
        public EntityList m_OuterLoop = new EntityList();
        public VectorOfEntityList m_InnerLoops = new VectorOfEntityList();

        // AcadHatch doesn't own these entities.
        // When you write DXF, don't operate these 2 variables, use AddAssociatedEntity method instead.
        public List<WeakReference> m_OuterAssociatedEntities = new List<WeakReference>(); // WeakReference of EntAttribute
        public List<List<WeakReference>> m_InnerAssociatedEntities = new List<List<WeakReference>>(); // WeakReference of EntAttribute

        public BoundaryPathFlag m_OuterFlag = new BoundaryPathFlag();
        public List<BoundaryPathFlag> m_InnerFlags = new List<BoundaryPathFlag>();

        // Here, pEnt can be any entities, such as AcadText.
        public bool AddAssociatedEntity(EntAttribute pEnt, int loopIndex = -1)
        {
            //Global.ASSERT_DEBUG_INFO("pEnt != null", pEnt != null);
            //Global.ASSERT_DEBUG_INFO("loopIndex >= -1", loopIndex >= -1);

            if (loopIndex == -1)
            {
                m_OuterAssociatedEntities.Add(new WeakReference(pEnt));
            }
            else
            {
                int origSize = m_InnerAssociatedEntities.Count;
                if (origSize <= (uint)loopIndex)
                {
                    for (int i = origSize; i <= loopIndex; ++i)
                    {
                        m_InnerAssociatedEntities.Add(new List<WeakReference>());
                    }
                }
                m_InnerAssociatedEntities[loopIndex].Add(new WeakReference(pEnt));
            }
            return true;
        }
        public bool HasAssociatedEntity()
        {
            foreach (var weakPtr in m_OuterAssociatedEntities)
            {
                if (weakPtr != null && weakPtr.IsAlive)
                {
                    return true;
                }
            }
            foreach (var innerLoop in m_InnerAssociatedEntities)
            {
                foreach (var weakPtr in innerLoop)
                {
                    if (weakPtr != null && weakPtr.IsAlive)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        private void WriteLoop(DxfWriter writer, int loopIndex)
        {
            EntityList pBoundaryData = null;
            List<WeakReference> pSourceHandles = null;
            if (loopIndex == -1)
            {
                pBoundaryData = m_OuterLoop;
                pSourceHandles = m_OuterAssociatedEntities;
            }
            else
            {
                pBoundaryData = m_InnerLoops[loopIndex];
                pSourceHandles = m_InnerAssociatedEntities[loopIndex];
            }
            Global.ASSERT_DEBUG_INFO("pBoundaryData != null && pBoundaryData.Count != 0", pBoundaryData != null && pBoundaryData.Count != 0);

            do
            {
                BoundaryPathFlag flag = 0;
                if (loopIndex == -1)
                {
                    flag = m_OuterFlag;
                }
                else
                {
                    flag = m_InnerFlags[loopIndex];
                }
                // pBoundaryData->Count == 1 && pBoundaryData[0] is closed AcadLWPLine
                if (pBoundaryData.Count == 1)
                {
                    var pEnt = pBoundaryData[0];
                    var pLWPLine = pEnt as AcadLWPLine;
                    if (pLWPLine != null && pLWPLine.m_Closed)
                    {
                        writer.dxfInt(92, (int)(flag | BoundaryPathFlag.LWPline));
                        writer.dxfInt(72, pLWPLine.HasBulges() ? 1 : 0);
                        writer.dxfInt(73, 1);
                        writer.dxfInt(93, (int)pLWPLine.m_Vertices.Count);
                        for (int i = 0; i < pLWPLine.m_Vertices.Count; ++i)
                        {
                            writer.dxfReal(10, pLWPLine.m_Vertices[i].x);
                            writer.dxfReal(20, pLWPLine.m_Vertices[i].y);
                            if (pLWPLine.HasBulges())
                            {
                                writer.dxfReal(42, pLWPLine.GetBulge(i));
                            }
                        }
                        break;
                    }
                }
                {
                    // pBoundaryData->Count > 1 or pBoundaryData[0] is not closed AcadLWPLine
                    int edgeNum = pBoundaryData.Count;
                    writer.dxfInt(92, (int)flag);
                    writer.dxfInt(93, edgeNum);
                    foreach (var pEnt in pBoundaryData)
                    {
                        var pLine = pEnt as AcadLine;
                        if (pLine != null)
                        {
                            writer.dxfInt(72, 1);
                            writer.dxfReal(10, pLine.m_StartPoint.x);
                            writer.dxfReal(20, pLine.m_StartPoint.y);
                            writer.dxfReal(11, pLine.m_EndPoint.x);
                            writer.dxfReal(21, pLine.m_EndPoint.y);
                            continue;
                        }

                        var pCircle = pEnt as AcadCircle;
                        if (pCircle != null)
                        {
                            Global.PRINT_DEBUG_INFO("Error: Circle isn't supported by DXF. It should be convert to LWPline. See convert function.");
                            throw new System.Exception("Program error!");
                            // Circle isn't supported by DXF. It should be convert to LWPline.
                            //writer.dxfInt(72, 2);
                            //writer.dxfReal(10, pCircle->m_Center.x);
                            //writer.dxfReal(20, pCircle->m_Center.y);
                            //writer.dxfReal(40, pCircle->m_Radius);
                            //auto pArc = dynamic_cast<AcadArc*>(pEnt.get());
                            //if (pArc)
                            //{
                            //  writer.dxfReal(50, pArc->m_StartAngle);
                            //  writer.dxfReal(51, pArc->m_EndAngle);
                            //}
                            //else
                            //{
                            //  writer.dxfReal(50, 0);
                            //  writer.dxfReal(51, 360);
                            //}
                            //writer.dxfInt(73, 1);
                            //continue;
                        }

                        var pEllipse = pEnt as AcadEllipse;
                        if (pEllipse != null)
                        {
                            writer.dxfInt(72, 3);
                            writer.dxfReal(10, pEllipse.m_Center.x);
                            writer.dxfReal(20, pEllipse.m_Center.y);
                            writer.dxfReal(11, pEllipse.m_MajorAxisPoint.x);
                            writer.dxfReal(21, pEllipse.m_MajorAxisPoint.x);
                            writer.dxfReal(40, pEllipse.m_MinorAxisRatio);
                            writer.dxfReal(50, pEllipse.m_StartAngle);
                            writer.dxfReal(51, pEllipse.m_EndAngle);
                            writer.dxfInt(73, 1);
                            continue;
                        }

                        var pSpline = pEnt as AcadSpline;
                        if (pSpline != null)
                        {
                            writer.dxfInt(72, 4);
                            writer.dxfInt(94, pSpline.m_Degree);
                            writer.dxfInt(73, (pSpline.m_Flag & 4) != 0 ? 1 : 0);
                            writer.dxfInt(74, (pSpline.m_Flag & 2) != 0 ? 1 : 0);

                            writer.dxfInt(95, (int)pSpline.m_Knots.Count);
                            writer.dxfInt(96, (int)pSpline.m_ControlPoints.Count);
                            foreach (double knot in pSpline.m_Knots)
                            {
                                writer.dxfReal(40, knot);
                            }

                            foreach (var cp in pSpline.m_ControlPoints)
                            {
                                writer.dxfReal(10, cp.x);
                                writer.dxfReal(20, cp.y);
                            }

                            // m_Weights ?

                            writer.dxfInt(97, (int)pSpline.m_FitPoints.Count);
                            foreach (var fp in pSpline.m_FitPoints)
                            {
                                writer.dxfReal(11, fp.x);
                                writer.dxfReal(21, fp.y);
                            }

                            writer.dxfReal(12, pSpline.m_StartTangent.x);
                            writer.dxfReal(22, pSpline.m_StartTangent.y);
                            writer.dxfReal(13, pSpline.m_EndTangent.x);
                            writer.dxfReal(23, pSpline.m_EndTangent.y);
                            continue;
                        }

                        // If execution reaches here, it is client programmer's error.
                        Global.ASSERT_DEBUG_INFO("Program error!", false);
                    }
                }
            } while (false);

            List<EntAttribute> validSourceHandles = new List<EntAttribute>();
            foreach (var sourceHandle in pSourceHandles)
            {
                var validSourceHandle = sourceHandle.Target as EntAttribute;
                if (validSourceHandle != null)
                {
                    validSourceHandles.Add(validSourceHandle);
                }
            }
            writer.dxfInt(97, (int)validSourceHandles.Count);
            foreach (var validSourceHandle in validSourceHandles)
            {
                writer.dxfHex(330, validSourceHandle.m_Handle);
            }
        }

        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            if (m_OuterLoop.Count == 0)
            {
                return;
            }

            writer.dxfString(0, "HATCH");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbHatch");
            writer.coord(10, 0, 0);
            writer.dxfReal(210, 0);
            writer.dxfReal(220, 0);
            writer.dxfReal(230, 1);
            string PatternName = m_PatternName;
            if (m_HatchStyle == AcHatchStyle.acHatchStyleOuter)
            {
                PatternName += ",_O";
            }
            else if (m_HatchStyle == AcHatchStyle.acHatchStyleIgnore)
            {
                PatternName += ",_I";
            }
            writer.dxfString(2, PatternName);
            bool bSolid = AcadHatch.FillFlag.SolidFill == m_FillFlag || string.Compare(m_PatternName, "SOLID", true) == 0;
            writer.dxfInt(70, bSolid ? 1 : 0);
            bool associativity = HasAssociatedEntity();
            writer.dxfInt(71, associativity ? 1 : 0);
            //环数
            writer.dxfInt(91, (int)(m_InnerLoops.Count + 1));
            //边界路径数据
            WriteLoop(writer, -1);
            for (uint i = 0; i < m_InnerLoops.Count; ++i)
            {
                WriteLoop(writer, (int)i);
            }
            writer.dxfInt(75, (int)m_HatchStyle);
            writer.dxfInt(76, (int)m_PatternType);
            if (!bSolid)
            {
                writer.dxfReal(52, m_PatternAngle);
                writer.dxfReal(41, m_PatternScale);
                writer.dxfInt(77, 0);
                Pattern pPat = writer.FindPattern(m_PatternName);
                if (pPat != null)
                {
                    writer.dxfInt(78, (int)pPat.dashlines.Count);
                    for (int i = 0; i < pPat.dashlines.Count; ++i)
                    {
                        writer.dxfReal(53, pPat.dashlines[i].angle + m_PatternAngle);
                        CDblPoint origin = new CDblPoint(pPat.dashlines[i].x_origin, pPat.dashlines[i].y_origin);
                        origin.x *= m_PatternScale;
                        origin.y *= m_PatternScale;
                        CDblPoint zero = new CDblPoint();
                        zero.Rotate(ref origin, m_PatternAngle * Math.PI / 180.0);
                        writer.dxfReal(43, origin.x);
                        writer.dxfReal(44, origin.y);
                        double angle = (pPat.dashlines[i].angle + m_PatternAngle) * Math.PI / 180.0;
                        CDblPoint point = new CDblPoint(pPat.dashlines[i].delta_x, pPat.dashlines[i].delta_y);
                        point.x *= m_PatternScale;
                        point.y *= m_PatternScale;
                        zero.Rotate(ref point, angle);
                        writer.dxfReal(45, point.x);
                        writer.dxfReal(46, point.y);
                        writer.dxfInt(79, (int)(pPat.dashlines[i].dashes.Count));
                        for (int j = 0; j < pPat.dashlines[i].dashes.Count; ++j)
                        {
                            writer.dxfReal(49, pPat.dashlines[i].dashes[j] * m_PatternScale);
                        }
                    }
                }
                else
                {
                    Global.PRINT_DEBUG_INFO("Can not find pattern {0}", m_PatternName);
                }
            }
            writer.dxfReal(47, m_PixelSize);
            writer.dxfInt(98, 1);
            writer.dxfReal(10, 0);
            writer.dxfReal(20, 0);
        }

        private AcadLWPLine convert(AcadCircle c)
        {
            var lwp = new AcadLWPLine();
            lwp.m_Vertices.Add(new CDblPoint(c.m_Center.x - c.m_Radius, c.m_Center.y));
            lwp.m_Vertices.Add(new CDblPoint(c.m_Center.x + c.m_Radius, c.m_Center.y));
            lwp.PushBulge(1.0);
            lwp.PushBulge(1.0);
            lwp.m_Closed = true;
            return lwp;
        }

        // if loopIndex is -1, add pEnt to outer loop;
        // if loopIndex >= 0, add pEnt to the specified inner loop;
        // You can only add one closed AcadLWPLine/AcadCircle into a loop;
        // Or you can add many AcadLine, AcadArc, AcadEllipseArc, AcadSpline into a loop;
        // It is client programmer's responsibility to ensure that the loop is closed. 
        public bool AddEntity(EntAttribute pEnt, int loopIndex = -1)
        {
            Global.ASSERT_DEBUG_INFO("pEnt != null", pEnt != null);
            Global.ASSERT_DEBUG_INFO("loopIndex >= -1", loopIndex >= -1);
            var pC = (pEnt) as AcadCircle;
            EntAttribute pEnt2;
            if (pC != null)
            {
                pEnt2 = convert(pC);
                if (loopIndex == -1)
                {
                    m_OuterFlag = (BoundaryPathFlag)(BoundaryPathFlag.Outer | BoundaryPathFlag.LWPline | BoundaryPathFlag.Export);
                }
                else
                {
                    if ((int)m_InnerFlags.Count <= loopIndex)
                    {
                        //m_InnerFlags.Resize(loopIndex + 1);
                        m_InnerFlags.Capacity = loopIndex + 1;
                        for (int i = m_InnerFlags.Count; i <= loopIndex; ++i)
                            m_InnerFlags.Add((BoundaryPathFlag)0);
                    }
                    m_InnerFlags[loopIndex] = (BoundaryPathFlag.LWPline);
                }
            }
            else
            {
                pEnt2 = pEnt;
            }

            if (loopIndex == -1)
            {
                m_OuterLoop.Add(pEnt2);
            }
            else
            {
                int origSize = m_InnerLoops.Count;
                if (origSize <= (int)loopIndex)
                {
                    //m_InnerLoops.Resize(loopIndex + 1);
                    m_InnerLoops.Capacity = loopIndex + 1;
                    for (int i = origSize; i <= loopIndex; ++i)
                        m_InnerLoops.Add(new EntityList());
                    //m_InnerAssociatedEntities.Resize(loopIndex + 1);
                    m_InnerAssociatedEntities.Capacity = loopIndex + 1;
                    for (int i = origSize; i <= loopIndex; ++i)
                        m_InnerAssociatedEntities.Add(new List<WeakReference>());
                    //for (int i = origSize; i <= (int)loopIndex; ++i)
                    //{
                    //    if (m_InnerAssociatedEntities[i] == null)
                    //    {
                    //        m_InnerAssociatedEntities[i] = new List<WeakReference>();
                    //    }
                    //    m_InnerLoops[i] = new EntityList();
                    //}
                }
                m_InnerLoops[loopIndex].Add(pEnt2);
            }
            return true;
        }
    }

    public class AcadLeader : EntAttribute
    {
        public AcadLeader()
        {
            this.m_StyleName = "ISO-25";
            this.m_Type = AcLeaderType.acLineWithArrow;
            this.m_ArrowSize = 10;
            this.m_HasBaseLine = true;
            this.m_BaseLineDirection = false;
            this.m_CharHeight = 10;
            this.m_TextWidth = 100;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "LEADER");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbLeader");
            writer.dxfString(3, m_StyleName);
            writer.dxfInt(71, (int)m_Type & 0x2);
            writer.dxfInt(72, (int)m_Type & 0x1);
            var annotation = m_Annotation.Target as EntAttribute;
            int val73 = 3;
            if ((annotation) as AcadMText != null)
            {
                val73 = 0;
            }
            else if ((annotation) as AcadBlockInstance != null)
            {
                val73 = 2;
            }
            writer.dxfInt(73, val73);
            writer.dxfInt(74, m_BaseLineDirection ? 1 : 0);
            writer.dxfInt(75, m_HasBaseLine ? 1 : 0);
            writer.dxfReal(40, m_CharHeight);
            writer.dxfReal(41, m_TextWidth);
            writer.dxfInt(76, (int)m_Vertices.Count);
            foreach (var vp in m_Vertices)
            {
                writer.dxfReal(10, vp.x);
                writer.dxfReal(20, vp.y);
            }
            if (annotation != null && annotation.m_Handle != 0)
            {
                writer.dxfHex(340, annotation.m_Handle);
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

        public string m_StyleName;
        public AcLeaderType m_Type = new AcLeaderType();
        public double m_ArrowSize;
        public bool m_HasBaseLine;
        public bool m_BaseLineDirection;
        public DblPoints m_Vertices = new DblPoints();
        public double m_CharHeight;
        public double m_TextWidth;

        // It is a AcadMText or AcadBlockInstance, you should add it to the same block with the block  AcadLeader.
        public WeakReference m_Annotation = new WeakReference(null);
    }

    public enum ContentType { BLOCK_TYPE = 1, MTEXT_TYPE = 2 };

    public interface StyleContent
    {
        void WriteDxf(DxfWriter writer);
        ContentType GetContentType();
    }

    public class StyleMTextPart : StyleContent
    {
        public StyleMTextPart()
        {
            this.m_TextStyle = "Standard";
            this.m_TextAngleType = AcTextAngleType.acHorizontalAngle;
            this.m_TextColor = AcColor.acByBlock;
            this.m_TextHeight = 0.18;
            this.m_AlignSpace = 4.0;
        }
        public void WriteDxf(DxfWriter writer)
        {
            writer.dxfHex(342, writer.LookupTextStyleHandle(m_TextStyle));
            writer.dxfInt(174, 1); // Text Left Attachment Type
            writer.dxfInt(178, 1); // ext Right Attachment Type
            writer.dxfInt(175, (int)m_TextAngleType);
            writer.dxfInt(176, 0); // Text Alignment Type
            writer.dxfInt(93, (int)m_TextColor);
            writer.dxfReal(45, m_TextHeight);
            writer.dxfInt(292, 0); // Enable Frame Text
            writer.dxfInt(297, 0); // Text Align Always Left
            writer.dxfReal(46, m_AlignSpace); // Align Space
        }
        public ContentType GetContentType()
        {
            return ContentType.MTEXT_TYPE;
        }
        public string m_TextStyle; //342
        public AcTextAngleType m_TextAngleType; //175
        public AcColor m_TextColor; //93
        public double m_TextHeight; // 45
        public double m_AlignSpace; //46
    }

    public class StyleBlockPart : StyleContent
    {
        public StyleBlockPart()
        {
            this.m_BlockName = "_DetailCallout";
            this.m_BlockConnectionType = AcBlockConnectionType.acConnectExtents;
            this.m_BlockColor = AcColor.acByLayer;
            this.m_BlockScale = new CDblPoint(1.0, 1.0);
        }
        public void WriteDxf(DxfWriter writer)
        {
            // CDblPoint m_BlockScale; /*142*/
            writer.dxfHex(343, writer.LookupBlockEntryHandle(m_BlockName));
            writer.dxfInt(94, (int)m_BlockColor);
            writer.dxfReal(47, m_BlockScale.x);
            writer.dxfReal(49, m_BlockScale.y);
            writer.dxfReal(140, 1.0); // Block Content Scale on Z-axis
            writer.dxfInt(293, 1); // Enable Block Content Scale
            writer.dxfReal(141, 0.0); // Block Content Rotation
            writer.dxfInt(294, 1); // Enable Block Content Rotation
            writer.dxfInt(177, (int)m_BlockConnectionType);
            writer.dxfReal(142, 1.0); // Scale
            writer.dxfInt(295, 1); // Overwrite Property Value
        }

        public ContentType GetContentType()
        {
            return ContentType.BLOCK_TYPE;
        }
        public string m_BlockName; //343
        public AcBlockConnectionType m_BlockConnectionType; //177, Center Extens, Insertion point
        public AcColor m_BlockColor; //94
        public CDblPoint m_BlockScale; //47, 49
    }

    public class MLeaderStyle //in OBJECTS section
    {
        public MLeaderStyle() // AcDimArrowheadType.acArrowDefault
        {
            this.m_LeaderType = AcMLeaderType.acStraightLeader;
            this.m_LineColor = AcColor.acByBlock;
            this.m_LineType = "ByBlock";
            this.m_LineWeight = AcLineWeight.acLnWtByBlock;
            this.m_ArrowHead = string.Empty;
            this.m_ArrowSize = 0.18;
            this.m_MaxLeaderPoints = 2;
            this.m_FirstSegAngleConstraint = 0.0f;
            this.m_SecondSegAngleConstraint = 0.0f;
            this.m_EnableLanding = true;
            this.m_IsAnnotative = false;
            this.m_LandingGap = 0.09;
            this.m_DoglegLength = 8.0;
            this.m_BreakGapSize = 3.75;
            this.m_Content = new StyleMTextPart();
        }
        internal void WriteDxf(DxfWriter writer, string name, int handle)
        {
            Global.ASSERT_DEBUG_INFO("!string.IsNullOrEmpty(name", !string.IsNullOrEmpty(name) && m_Content != null);
            writer.dxfString(0, "MLEADERSTYLE");
            writer.dxfHex(5, handle);
            writer.dxfString(102, "{ACAD_REACTORS");
            writer.dxfHex(330, 0x12D); // ACAD_MLEADERSTYLE dictionary handle
            var mls = writer.GetMLeaders(name);
            foreach (var ml in mls)
            {
                writer.dxfHex(330, ml.m_Handle);
            }
            writer.dxfString(102, "}");
            writer.dxfHex(330, 0x12D);
            writer.dxfString(100, "AcDbMLeaderStyle");

            writer.dxfInt(179, 2); // ???
            writer.dxfInt(170, (int)m_Content.GetContentType());
            writer.dxfInt(171, 1); // DrawMLeaderOrder Type
            writer.dxfInt(172, 0); // DrawLeaderOrder Type
            writer.dxfInt(90, m_MaxLeaderPoints);
            writer.dxfReal(40, m_FirstSegAngleConstraint);
            writer.dxfReal(41, m_SecondSegAngleConstraint);
            writer.dxfInt(173, (int)m_LeaderType);
            writer.dxfInt(91, (int)m_LineColor);
            writer.dxfHex(340, writer.LookupLinetypeHandle(m_LineType));
            writer.dxfInt(92, (int)m_LineWeight);
            writer.dxfInt(290, m_EnableLanding ? 1 : 0);
            writer.dxfReal(42, m_LandingGap);
            writer.dxfInt(291, m_EnableDogleg ? 1 : 0);
            writer.dxfReal(43, m_DoglegLength);
            writer.dxfString(3, name); // Mleader Style Description
            if (!string.IsNullOrEmpty(m_ArrowHead))
            {
                writer.dxfHex(341, writer.LookupBlockEntryHandle(m_ArrowHead));
            }
            writer.dxfReal(44, m_ArrowSize);
            writer.dxfString(300, ""); // Default MText Contents

            m_Content.WriteDxf(writer);

            writer.dxfInt(295, 0);
            writer.dxfInt(296, m_IsAnnotative ? 1 : 0);
            writer.dxfReal(143, m_BreakGapSize);
            writer.dxfInt(271, 0); // Text attachment direction for MText contents: 0 = Horizontal, 1 = Vertical
            writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
            writer.dxfInt(273, 9); // Top text attachment direction:    9 = Center, 10 = Overline and Center
        }
        public AcMLeaderType m_LeaderType; //173
        public AcColor m_LineColor; //91
        public string m_LineType; //340
        public AcLineWeight m_LineWeight; //92
        public string m_ArrowHead; //341, arrow block, can be empty
        public double m_ArrowSize; //44

        public int m_MaxLeaderPoints; //90
        public double m_FirstSegAngleConstraint; //40
        public double m_SecondSegAngleConstraint; //41
        public bool m_EnableLanding; //290
        public bool m_EnableDogleg; //291
        public bool m_IsAnnotative; //296
        public double m_LandingGap; //42

        public double m_DoglegLength; //43
        public double m_BreakGapSize; //143

        // ContentType m_ContentType; /*170*/
        public StyleContent m_Content;
    }

    public class LeaderLine
    {
        public DblPoints m_Points = new DblPoints(); /*10*/
    }

    public class LeaderPart
    {
        public LeaderPart()
        {
            this.m_DoglegLength = 8.0;
        }
        internal void WriteDxf(DxfWriter writer)
        {
            writer.dxfString(302, "LEADER{");
            writer.dxfInt(290, 1); // Has Set Last Leader Line Point
            writer.dxfInt(291, 1); // Has Set Dogleg Vector
            writer.coord(10, m_CommonPoint.x, m_CommonPoint.y);
            writer.coord(11, 1.0, 0.0); // Dogleg Vector
            writer.dxfInt(90, 0); // Leader Branch Index
            writer.dxfReal(40, m_DoglegLength); // Dogleg Length
            int i = -1;
            foreach (var ll in m_Lines)
            {
                writer.dxfString(304, "LEADER_LINE{");
                foreach (var p in ll.m_Points)
                {
                    writer.coord(10, p.x, p.y);
                }
                writer.dxfInt(91, ++i);
                writer.dxfString(305, "}");
            }
            writer.dxfInt(271, 0); // ???
            writer.dxfString(303, "}");
        }
        public CDblPoint m_CommonPoint = new CDblPoint(); //10
        public List<LeaderLine> m_Lines = new List<LeaderLine>();
        public double m_DoglegLength; //40, required
    }

    public interface Content
    {
        void WriteDxf(DxfWriter writer);
        ContentType GetContentType();
    }

    public class MTextPart : Content
    {
        public MTextPart()
        {
            this.m_TextWidth = 0.0;
            this.m_TextStyle = "Standard";
            this.m_TextColor = AcColor.acByBlock;
            this.m_TextRotation = 0.0;
            this.m_AttachmentPoint = AcAttachmentPoint.acAttachmentPointTopLeft;
        }
        public void WriteDxf(DxfWriter writer)
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
            writer.coord(13, Math.Cos(m_TextRotation), Math.Sin(m_TextRotation)); // Text Direction
            writer.dxfReal(42, m_TextRotation);
            writer.dxfReal(43, m_TextWidth);
            writer.dxfReal(44, 0.0); // Text Height, same as 41, don't know why.
            writer.dxfReal(45, 1.0); // Text Line Spacing Factor
            writer.dxfInt(170, 1); // Text Line Spacing Style
            writer.dxfInt(90, (int)m_TextColor);
            writer.dxfInt(171, (int)m_AttachmentPoint);
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

        public ContentType GetContentType()
        {
            return ContentType.MTEXT_TYPE;
        }
        public string m_Text; // 304, no default value, use \P as new line. such as hello\Pworld
        public double m_TextWidth; //43, no default value
        public string m_TextStyle; //340
        public AcColor m_TextColor; //90
        public CDblPoint m_TextLocation = new CDblPoint(); //12, no default value
        public double m_TextRotation; //42, in radians, 13 = cos, 23 = sin
        public AcAttachmentPoint m_AttachmentPoint; //171
    }

    public class BlockPart : Content
    {
        public BlockPart()
        {
            this.m_BlockColor = AcColor.acByLayer;
            this.m_BlockScale = new CDblPoint(1.0, 1.0);
            this.m_BlockRotation = 0.0;
        }
        public void WriteDxf(DxfWriter writer)
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
            writer.dxfInt(93, (int)m_BlockColor);
            double[] matrix = { 1, 0, 0, m_BlockPosition.x, 0, 1, 0, m_BlockPosition.y, 0, 0, 1, 0, 0, 0, 0, 1 };
            for (int i = 0; i < 16; ++i)
            {
                writer.dxfReal(47, matrix[i]);
            }
        }

        public ContentType GetContentType()
        {
            return ContentType.BLOCK_TYPE;
        }
        public string m_BlockName; //341, no default value
        public AcColor m_BlockColor; //93
        public CDblPoint m_BlockPosition = new CDblPoint(); //15, no default value, sixteen 47 for 4x4 matrix is derived from this
        public CDblPoint m_BlockScale = new CDblPoint(); //16
        public double m_BlockRotation; //46, in radians, 13 = cos, 23 = sin
    }

    public class CONTEXT_DATA
    {
        public CONTEXT_DATA()
        {
            this.m_LandingGap = 0.09;
            this.m_TextHeight = 4.0;
            this.m_ArrowSize = 4.0;
            this.m_Content = new MTextPart();
        }
        internal void WriteDxf(DxfWriter writer)
        {
            writer.dxfString(300, "CONTEXT_DATA{");
            writer.dxfReal(40, 1.0);
            writer.coord(10, m_LandingPosition.x, m_LandingPosition.y);
            writer.dxfReal(41, m_TextHeight);
            writer.dxfReal(140, m_ArrowSize);
            writer.dxfReal(145, m_LandingGap);
            m_Content.WriteDxf(writer);
            // Already checked validity.
            writer.coord(110, m_Leader.m_Lines[0].m_Points[0].x, m_Leader.m_Lines[0].m_Points[0].y); // Mleader Plane Origin Point
            writer.coord(111, 1.0, 0.0); // MLeader Plane X-Axis Direction
            writer.coord(112, 0.0, 1.0); // MLeader Plane Y-Axis Direction
            writer.dxfInt(297, 0); // MLeader Plane Normal Reversed
            m_Leader.WriteDxf(writer);
            writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
            writer.dxfInt(273, 9); // Top text attachment direction:    9 = Center, 10 = Overline and Center
            writer.dxfString(301, "}");
        }
        public CDblPoint m_LandingPosition = new CDblPoint(); // 10, no default value
        public double m_TextHeight; // 41
        public double m_ArrowSize; // 140
        public double m_LandingGap; // 145
                                    /* 290 hasMText, 296 hasBlock*/
        public Content m_Content;
        /*110, Mleader Plane Origin Point is first leader arrow point*/
        public LeaderPart m_Leader = new LeaderPart();
    }

    // http://help.autodesk.com/view/ACD/2016/ENU/?guid=GUID-72D20B8C-0F5E-4993-BEB7-0FCF94F32BE0
    // http://docs.autodesk.com/ACD/2010/ENU/AutoCAD%202010%20User%20Documentation/index.html?url=WS73099cc142f487551d92abb10dc573c45d-7bf1.htm,topicNumber=d0e117972
    // A multileader object typically consists of an arrowhead, a horizontal landing, a leader line or curve, and either a multiline text object or a block.
    public class AcadMLeader : EntAttribute
    {
        public AcadMLeader()
        //, m_ArrowSize(0.18)
        //, m_TextStyle("Standard")
        //, m_DoglegLength(4.0)
        {
            this.m_LeaderStyle = "Standard";
            this.m_LeaderType = AcMLeaderType.acStraightLeader;
            this.m_LineType = "ByBlock";
            this.m_LineWeight = AcLineWeight.acLnWtByBlock;
            this.m_EnableLanding = true;
            this.m_EnableDogleg = true;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            if (m_ContextData.m_Content == null || (m_ContextData.m_Leader.m_Lines.Count == 0 || m_ContextData.m_Leader.m_Lines[0].m_Points.Count == 0) || m_AttrDefs.Count != m_AttrValues.Count)
            {
                Global.PRINT_DEBUG_INFO("Invalid MultiLeader.");
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
            writer.dxfInt(170, (int)m_LeaderType);
            writer.dxfInt(91, -1056964608); // Leade LineColor
            writer.dxfHex(341, writer.LookupLinetypeHandle(m_LineType));
            writer.dxfInt(171, (int)m_LineWeight);
            writer.dxfInt(290, m_EnableLanding ? 1 : 0);
            writer.dxfInt(291, m_EnableDogleg ? 1 : 0);
            //writer.dxfReal(41, m_DoglegLength);
            //writer.dxfReal(42, m_ArrowSize);
            writer.dxfInt(172, (int)m_ContextData.m_Content.GetContentType());

            //writer.dxfHex(343, writer.LookupTextStyleHandle(m_TextStyle));
            writer.dxfInt(173, 1); // Text Left Attachment Type
            writer.dxfInt(95, 1); // Text Right Attachement Type
            writer.dxfInt(174, 1); // Text Angle Type
            writer.dxfInt(175, 0); // Text Alignment Type
            writer.dxfInt(92, -1056964608); // Text Color
            writer.dxfInt(292, 0); // Enable Frame Text
            if (m_ContextData.m_Content.GetContentType() == ContentType.BLOCK_TYPE)

            {
                BlockPart pBlock = (BlockPart)m_ContextData.m_Content;
                writer.dxfHex(344, writer.LookupBlockEntryHandle(pBlock.m_BlockName));
            }
            writer.dxfInt(93, -1056964608); // Block Content Color
            writer.coord(10, 1.0, 1.0); // Block Content Scale
            writer.dxfReal(43, 0.0); // Block Content Rotation
            writer.dxfInt(176, 0); // Block Content Connection Type
            writer.dxfInt(293, 0); // Enable Annotation Scale
            for (int i = 0; i < m_AttrValues.Count; ++i)

            {
                writer.dxfHex(330, m_AttrDefs[i].m_Handle);
                writer.dxfInt(177, (int)(i + 1)); // Block Attribute Index
                writer.dxfReal(44, 0.0); // Block Attribute Width
                writer.dxfString(302, m_AttrValues[i]);
            }
            writer.dxfInt(294, 0); // Text Direction Negative
            writer.dxfInt(178, 0); // Text Align in IPE
            writer.dxfInt(179, 1); // Text Attachment Point
            writer.dxfReal(45, 1.0);
            writer.dxfInt(271, 0); // Text attachment direction for MText contents: 0 = Horizontal, 1 = Vertical
            writer.dxfInt(272, 9); // Bottom text attachment direction: 9 = Center, 10 = Underline and Center
            writer.dxfInt(273, 9); // Top text attachment direction:    9 = Center, 10 = Overline and Center
        }

        public CONTEXT_DATA m_ContextData = new CONTEXT_DATA();
        public string m_LeaderStyle; //340
        public AcMLeaderType m_LeaderType; //170
        public string m_LineType; //341
        public AcLineWeight m_LineWeight; //171
        public bool m_EnableLanding; //290
        public bool m_EnableDogleg; //291
                                    //double m_DoglegLength; /*41, this is ignored by ACAD, only LEADER::m_DoglegLength matters*/
                                    //double m_ArrowSize; /*42, this is ignored by ACAD, only CONTEXT_DATA::m_ArrowSize matters*/
                                    // ContentType m_ContentType; /*172*/
                                    //std::string m_TextStyle; /*343, this is ignored by ACAD, only MTEXT::m_TextStyle matters*/
        public List<AcadAttDef> m_AttrDefs = new List<AcadAttDef>(); //330
                                                                     //Text string value for an attribute definition, repeated once per attribute definition and applicable only for a block - type cell
        public List<string> m_AttrValues = new List<string>(); //302, ""
    }

    public enum CellType { TEXT_CELL = 1, BLOCK_CELL = 2 };

    public class CellInTableStyle // Not CELLSTYLE, CELLSTYLE isn't implemented now
    {
        public CellInTableStyle()
        {
            this.m_TextHeight = 0.18;
            this.m_Alignment = AcCellAlignment.acMiddleCenter;
            this.m_TextColor = AcColor.acByBlock;
            this.m_BgColor = AcColor.acWhite;
            this.m_BgColorEnabled = false;
            this.m_CellDataType = AcValueDataType.acGeneral;
            this.m_CellUnitType = AcValueUnitType.acUnitless;
            for (int i = 0; i < 6; ++i)
            {
                m_BorderLineWeight[i] = AcLineWeight.acLnWtByBlock;
                m_BorderVisible[i] = true;
                m_BorderColor[i] = AcColor.acByBlock;
            }
        }
        internal void WriteDxf(DxfWriter writer)
        {
            writer.dxfReal(140, m_TextHeight);
            writer.dxfInt(170, (int)m_Alignment);
            writer.dxfInt(62, (int)m_TextColor);
            writer.dxfInt(63, (int)m_BgColor);
            writer.dxfInt(283, m_BgColorEnabled ? 1 : 0);
            writer.dxfInt(90, (int)m_CellDataType);
            writer.dxfInt(91, (int)m_CellUnitType);
            writer.dxfString(1, ""); // ???
            for (int i = 0; i < 6; ++i)
            {
                writer.dxfInt(274 + i, (int)m_BorderLineWeight[i]);
                writer.dxfInt(284 + i, m_BorderVisible[i] ? 1 : 0);
                writer.dxfInt(64 + i, (int)m_BorderColor[i]);
            }
        }

        public double m_TextHeight; //140, 0.18
        public AcCellAlignment m_Alignment; //170, AcCellAlignment.acMiddleCenter
        public AcColor m_TextColor; //62, AcColor.acByBlock
        public AcColor m_BgColor; //63, AcColor.acWhite
        public bool m_BgColorEnabled; //283, false
        public AcValueDataType m_CellDataType; //90, AcValueDataType.acGeneral
        public AcValueUnitType m_CellUnitType; //91, AcValueUnitType.acUnitless
                                                                       /*1, "", ???*/
        public AcLineWeight[] m_BorderLineWeight = new AcLineWeight[6]; //274-279, AcLineWeight.acLnWtByBlock
        public bool[] m_BorderVisible = new bool[6]; //284-289, true
        public AcColor[] m_BorderColor = new AcColor[6]; //64-69, AcColor.acByBlock
    }

    //http://docs.autodesk.com/ACD/2014/CSY/index.html?url=files/GUID-0DBCA057-9F6C-4DEB-A66F-8A9B3C62FB1A.htm,topicNumber=d30e702776

    public class TableStyle //in OBJECTS section
    {
        public TableStyle()
        {
            this.m_HorCellMargin = 0.06;
            this.m_VerCellMargin = 0.06;
            this.m_HasNoTitle = false;
            this.m_HasNoColumnHeading = false;
            this.m_TextStyle = "Standard";
            // List has no resize!!!
            for (int i = 0; i < 3; ++i)
                m_Cells.Add(new CellInTableStyle());
        }
        internal void WriteDxf(DxfWriter writer, string name, int handle)
        {
            Global.ASSERT_DEBUG_INFO("!string.IsNullOrEmpty(name)", !string.IsNullOrEmpty(name));
            writer.dxfString(0, "TABLESTYLE");
            writer.dxfHex(5, handle);
            writer.dxfString(102, "{ACAD_REACTORS");
            writer.dxfHex(330, 0x7E); // ACAD_TABLESTYLE dictionary handle
            var tbls = writer.GetTables(name);
            foreach (var tbl in tbls)
            {
                writer.dxfHex(330, tbl.m_Handle);
            }
            writer.dxfString(102, "}");
            writer.dxfHex(330, 0x7E);
            writer.dxfString(100, "AcDbTableStyle");
            writer.dxfInt(280, 0);
            writer.dxfString(3, name); // Table style description
            writer.dxfInt(70, 0); // FlowDirection (integer): 0 = Down 1 = Up
            writer.dxfInt(71, 0); // Flags (bit-coded)
            writer.dxfReal(40, m_HorCellMargin);
            writer.dxfReal(41, m_VerCellMargin);
            writer.dxfInt(280, m_HasNoTitle ? 1 : 0);
            writer.dxfInt(281, m_HasNoColumnHeading ? 1 : 0);
            writer.dxfString(7, m_TextStyle);
            foreach (var cell in m_Cells)
            {
                cell.WriteDxf(writer);
            }
        }

        public double m_HorCellMargin; //40, 0.06
        public double m_VerCellMargin; //41, 0.06
        public bool m_HasNoTitle; //280, false
        public bool m_HasNoColumnHeading; //281, false
        public string m_TextStyle; //7, "Standard

        // Don't understand why there are many CellInTableStyle(s)
        public List<CellInTableStyle> m_Cells = new List<CellInTableStyle>(3);
    }

    public interface CellContent
    {
        void WriteDxf(DxfWriter writer);
        CellType GetContentType();
    }

    public class CellText : CellContent
    {
        public void WriteDxf(DxfWriter writer)
        {
            if (!string.IsNullOrEmpty(m_Text))
            {
                writer.dxfText(2, 1, m_Text);
            }
            if (!string.IsNullOrEmpty(m_TextStyle))
            {
                writer.dxfString(7, m_TextStyle);
            }
            writer.dxfInt(94, 0); // ???
            writer.dxfString(300, "");
            writer.dxfText(303, 302, m_Text);
        }
        public CellType GetContentType()
        {
            return CellType.TEXT_CELL;
        }

        public string m_Text = string.Empty; //many 2 and one 1, such as {\fSimSun|b0|i0|c134|p2;王五}
                              /*many 303 and one 302, same as 2 and 1*/
        public string m_TextStyle = string.Empty; //7, ""
    }

    public class CellBlock : CellContent
    {
        public CellBlock()
        {
            this.m_BlockScale = 1.0;
        }
        public void WriteDxf(DxfWriter writer)
        {
            Global.ASSERT_DEBUG_INFO("m_AttrDefs.Count == m_AttrValues.Count", m_AttrDefs.Count == m_AttrValues.Count);
            writer.dxfHex(340, writer.LookupBlockEntryHandle(m_BlockName));
            writer.dxfReal(144, m_BlockScale);
            writer.dxfInt(179, (int)m_AttrDefs.Count);

            if (m_AttrValues.Count == 0)
            {
                return;
            }
            for (int i = 0; i < m_AttrDefs.Count; ++i)
            {
                writer.dxfHex(331, m_AttrDefs[i].m_Handle);
                writer.dxfString(300, m_AttrValues[i]);
            }
        }
        public CellType GetContentType()
        {
            return CellType.BLOCK_CELL;
        }

        public string m_BlockName; //340
        public double m_BlockScale; //144
                                    /*179, number of attribute definitions in the block*/
        public List<AcadAttDef> m_AttrDefs = new List<AcadAttDef>(); //331
                                                                     //Text string value for an attribute definition, repeated once per attribute definition and applicable only for a block - type cell
        public List<string> m_AttrValues = new List<string>(); //300, ""
    }

    public class Cell
    {
        public Cell(CellType ct)
        {
            this.m_Flag = 0;
            this.m_Merged = false;
            this.m_AutoFit = false;
            this.m_ColSpan = 1;
            this.m_RowSpan = 1;
            this.m_OverrideFlag1 = (int)AcCellProperty.acAlignmentProperty | (int)AcCellProperty.acContentLayout | (int)AcCellProperty.acTextHeight;
            this.m_VirtualEdgeFlag = 0;
            this.m_Rotation = 0.0;
            this.m_Alignment = AcCellAlignment.acInvalidAlignment;
            this.m_BgColor = AcColor.acByBlock;
            this.m_TextColor = AcColor.acByBlock;
            this.m_BgColorEnabled = false;
            this.m_TextHeight = 0.0;
            this.m_Flag93 = 6;
            this.m_Flag90 = 4;
            this.m_OverrideFlag2 = 0;
            this.m_LeftBorderVisible = true;
            this.m_RightBorderVisible = true;
            this.m_TopBorderVisible = true;
            this.m_BottomBorderVisible = true;
            if (ct == CellType.TEXT_CELL)
            {
                m_Content = new CellText();
            }
            else
            {
                m_Content = new CellBlock();
            }
        }
        internal void WriteDxf(DxfWriter writer)
        {
            Global.ASSERT_DEBUG_INFO("m_Content != null", m_Content != null);
            writer.dxfInt(171, (int)m_Content.GetContentType());
            writer.dxfInt(172, m_Flag);
            writer.dxfInt(173, m_Merged ? 1 : 0);
            writer.dxfInt(174, m_AutoFit ? 1 : 0);
            writer.dxfInt(175, m_ColSpan);
            writer.dxfInt(176, m_RowSpan);
            writer.dxfInt(91, m_OverrideFlag1);
            writer.dxfInt(178, m_VirtualEdgeFlag);
            writer.dxfReal(145, m_Rotation);
            if (m_Content as CellBlock != null)
            {
                m_Content.WriteDxf(writer);
            }
            if (m_Alignment != AcCellAlignment.acInvalidAlignment)
            {
                writer.dxfInt(170, (int)m_Alignment);
            }
            if (m_BgColor != AcColor.acByBlock)
            {
                writer.dxfInt(63, (int)m_BgColor);
            }
            if (m_TextColor != AcColor.acByBlock)
            {
                writer.dxfInt(64, (int)m_TextColor);
            }
            if (m_BgColor != AcColor.acByBlock)
            {
                writer.dxfInt(283, m_BgColorEnabled ? 1 : 0);
            }
            if (m_TextHeight != 0.0)
            {
                writer.dxfReal(140, m_TextHeight);
            }
            writer.dxfInt(92, 0); //Extended cell flags
            writer.dxfString(301, "CELL_VALUE");
            writer.dxfInt(93, m_Flag93); // ???
            writer.dxfInt(90, m_Flag90); // ???
            if (m_OverrideFlag2 != 0)
            {
                writer.dxfInt(91, m_OverrideFlag2);
            }
            if (m_Content as CellText != null)
            {
                m_Content.WriteDxf(writer);
            }
            else
            {
                new CellText().WriteDxf(writer);
            }
            if (!m_LeftBorderVisible)
            {
                writer.dxfReal(288, 0);
            }
            if (!m_RightBorderVisible)
            {
                writer.dxfReal(285, 0);
            }
            if (!m_TopBorderVisible)
            {
                writer.dxfReal(289, 0);
            }
            if (!m_BottomBorderVisible)
            {
                writer.dxfReal(286, 0);
            }
            writer.dxfString(304, "ACVALUE_END");
        }

        //Type m_Type; /*171*/
        public int m_Flag; //172, 0
        public bool m_Merged; //173, false
        public bool m_AutoFit; //174, false
                               // These 2 fields is applicable for merging cells 
        public int m_ColSpan; //175
        public int m_RowSpan; //176
        public int m_OverrideFlag1; //or values in AcCellProperty
                                    // A virtual edge is used when a grid line is shared by two cells.For example, if a table contains one row and two columns and it contains cell A and cell B, the central grid line contains the right edge of cell A and the left edge of cell B.One edge is real, and the other edge is virtual.The virtual edge points to the real edge; both edges have the same set of properties, including color, lineweight, and visibility.
        public int m_VirtualEdgeFlag; //178, 0
        public double m_Rotation; //145, 0.0, in radians, applicable for TEXT and BLOCK
        public AcCellAlignment m_Alignment; //170, 0
        public AcColor m_BgColor; //63
        public AcColor m_TextColor; //64
        public bool m_BgColorEnabled; //283, false
        public double m_TextHeight; //140, 0.0
                                    // 92, 0, Extended cell flags

        /*301, CELL_VALUE*/
        public int m_Flag93; //93, 6, ???
        public int m_Flag90; //90, 4, ???
        public int m_OverrideFlag2; //91, 0, ???
        public CellContent m_Content;
        public bool m_LeftBorderVisible; //288, true
        public bool m_RightBorderVisible; //285, true
        public bool m_TopBorderVisible; //289, true
        public bool m_BottomBorderVisible; //286, true
                                           /*304, ACVALUE_END*/
    }

    public class AcadTable : EntAttribute
    {
        public AcadTable()
        {
            this.m_TableStyle = "Standard";
            this.m_RowCount = 0;
            this.m_ColCount = 0;
            this.m_HorCellMargin = 0.0;
            this.m_VerCellMargin = 0.0;
            this.m_LeftBorderVisible = true;
            this.m_RightBorderVisible = true;
            this.m_TopBorderVisible = true;
            this.m_BottomBorderVisible = true;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            if (m_RowCount <= 0 || m_ColCount <= 0 || m_RowHeights.Count != m_RowCount || m_ColWidths.Count != m_ColCount || m_Cells.Count != m_RowCount * m_ColCount)
            {
                Global.PRINT_DEBUG_INFO("Invalid Table.");
                return;
            }
            writer.AddTableToItsStyle(m_TableStyle, this);
            writer.dxfString(0, "ACAD_TABLE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfInt(160, 3512); // ???
            writer.dxfString(100, "AcDbBlockReference");
            if (!string.IsNullOrEmpty(m_TableBlockName))
            {
                writer.dxfString(2, m_TableBlockName);
            }
            writer.coord(10, m_InsertionPoint.x, m_InsertionPoint.y);
            writer.dxfString(100, "AcDbTable");
            writer.dxfInt(280, 0);
            writer.dxfHex(342, writer.LookupTableStyleHandle(m_TableStyle));
            if (!string.IsNullOrEmpty(m_TableBlockName))
            {
                writer.dxfHex(343, writer.LookupBlockEntryHandle(m_TableBlockName));
            }
            writer.coord(11, 1.0, 0.0);
            writer.dxfInt(90, 22);
            writer.dxfInt(91, m_RowCount);
            writer.dxfInt(92, m_ColCount);
            writer.dxfInt(93, 0);
            writer.dxfInt(94, 0);
            writer.dxfInt(95, 0);
            writer.dxfInt(96, 0);
            foreach (var h in m_RowHeights)
            {
                writer.dxfReal(141, h);
            }
            foreach (var w in m_ColWidths)
            {
                writer.dxfReal(142, w);
            }
            foreach (var cell in m_Cells)
            {
                cell.WriteDxf(writer);
            }
            if (m_HorCellMargin != 0.0)
            {
                writer.dxfReal(40, m_HorCellMargin);
            }
            if (m_VerCellMargin != 0.0)
            {
                writer.dxfReal(41, m_VerCellMargin);
            }
            if (!m_LeftBorderVisible)
            {
                writer.dxfReal(288, 0);
            }
            if (!m_RightBorderVisible)
            {
                writer.dxfReal(285, 0);
            }
            if (!m_TopBorderVisible)
            {
                writer.dxfReal(289, 0);
            }
            if (!m_BottomBorderVisible)
            {
                writer.dxfReal(286, 0);
            }
        }

        // These are convinient functions to construct a table object.
        public void InitializeCells(int rowCount, int colCount, CellType ct)
        {
            if (rowCount <= 0 || colCount <= 0)
            {
                Global.PRINT_DEBUG_INFO("Invalid argument: rowCount = {0}, colCount = {1}.", rowCount, colCount);
                return;
            }
            m_RowCount = rowCount;
            m_ColCount = colCount;
            // List has no resize!
            for (int i=0; i<rowCount * colCount; ++i)
                m_Cells.Add(new Cell(ct));
        }

        public void SetRowHeight(double height)
        {
            // List has no resize!
            for (int i = 0; i < m_RowCount; ++i)
                m_RowHeights.Add(height);
        }

        public void SetColWidth(double width)
        {
            // List has no resize!
            for (int i = 0; i < m_ColCount; ++i)
                m_ColWidths.Add(width);
        }

        // It is programmer's responsibilty to ensure merged cells are not overlapped.
        public void MergeCells(int row, int col, int rowSpan, int colSpan)
        {
#if DEBUG
            Global.ASSERT_DEBUG_INFO("row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount", row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
            Global.ASSERT_DEBUG_INFO("rowSpan >= 1 && row + rowSpan <= m_RowCount && colSpan >= 1 && col + colSpan <= m_ColCount", rowSpan >= 1 && row + rowSpan <= m_RowCount && colSpan >= 1 && col + colSpan <= m_ColCount);
            Global.ASSERT_DEBUG_INFO("m_Cells.Count == m_RowCount * m_ColCount", m_Cells.Count == m_RowCount * m_ColCount);
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

        public Cell GetCell(int row, int col)
        {
#if DEBUG
            Global.ASSERT_DEBUG_INFO("row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount", row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
            Global.ASSERT_DEBUG_INFO("m_Cells.Count == m_RowCount * m_ColCount", m_Cells.Count == m_RowCount * m_ColCount);
#endif
            return m_Cells[row * m_ColCount + col];
        }

        public CellText GetCellText(int row, int col)
        {
#if DEBUG
            Global.ASSERT_DEBUG_INFO("row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount", row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
            Global.ASSERT_DEBUG_INFO("m_Cells.Count == m_RowCount * m_ColCount", m_Cells.Count == m_RowCount * m_ColCount);
#endif
            var cell = m_Cells[row * m_ColCount + col];
            if (cell.m_Content == null || (cell.m_Content as CellBlock) != null)
            {
                cell.m_Content = new CellText();
            }
            CellText pText = (CellText)cell.m_Content;
            return pText;
        }

        public CellBlock GetCellBlock(int row, int col)
        {
#if DEBUG
            Global.ASSERT_DEBUG_INFO("row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount", row >= 0 && row < m_RowCount && col >= 0 && col < m_ColCount);
            Global.ASSERT_DEBUG_INFO("m_Cells.Count == m_RowCount * m_ColCount", m_Cells.Count == m_RowCount * m_ColCount);
#endif
            var cell = m_Cells[row * m_ColCount + col];
            if (cell.m_Content as CellText != null)
            {
                cell.m_Content = new CellBlock();
            }
            CellBlock pBlock = (CellBlock)cell.m_Content;
            return pBlock;
        }

        /*??? 160, 3512*/
        /*100, AcDbBlockReference*/
        public string m_TableBlockName; //2, optional anonymous block begins with a *T value to render the whole table
        public CDblPoint m_InsertionPoint = new CDblPoint(); //10
                                                             /*100, AcDbTable*/
                                                             /*280, 0, 2010*/
        public string m_TableStyle; //342
                                    /*343, *T Block handle*/
                                    /*11, 1.0, 0.0, Horizontal direction vector*/
                                    /*90, 22, Flag for table value (unsigned integer)*/
        public int m_RowCount; //91
        public int m_ColCount; //92
                               /*93, 0, Flag for an override*/
                               /*94, 0, Flag for an override of border color*/
                               /*95, 0, Flag for an override of border lineweight*/
                               /*96, 0, Flag for an override of border visibility*/
        public List<double> m_RowHeights = new List<double>(); //141
        public List<double> m_ColWidths = new List<double>(); //142
                                                              // There must be m_RowCount * m_ColCount Cell(s), for merged cells which m_Merged is true.*/
        public List<Cell> m_Cells = new List<Cell>();
        public double m_HorCellMargin; //40, 0.0
        public double m_VerCellMargin; //41, 0.0
        public bool m_LeftBorderVisible; //288, true
        public bool m_RightBorderVisible; //285, true
        public bool m_TopBorderVisible; //289, true
        public bool m_BottomBorderVisible; //286, true
    }
    public class AcadLine : EntAttribute
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "LINE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbLine");
            writer.dxfReal(10, m_StartPoint.x);
            writer.dxfReal(20, m_StartPoint.y);
            writer.dxfReal(11, m_EndPoint.x);
            writer.dxfReal(21, m_EndPoint.y);
        }

        public CDblPoint m_StartPoint = new CDblPoint();
        public CDblPoint m_EndPoint = new CDblPoint();
    }
    public class AcadLWPLine : EntAttribute
    {
        public AcadLWPLine()
        {
            this.m_Closed = false;
            this.m_Width = 1.0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "LWPOLYLINE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbPolyline");
            writer.dxfInt(90, (int)m_Vertices.Count);
            writer.dxfInt(70, (m_Closed ? 1 : 0) | 128); //128 = 启用线型生成
            if (IsConstWidth())
            {
                writer.dxfReal(43, m_Width);
            }
            if (!IsConstWidth())
            {
                int count = m_Vertices.Count;
                int start = m_startWidths.Count;
                for (int i = start; i < count; ++i)
                    m_startWidths.Add(1.0);
                int start2 = m_endWidths.Count;
                for (int i = start2; i < count; ++i)
                    m_endWidths.Add(1.0);
            }
            for (int i = 0; i < m_Vertices.Count; ++i)
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
                {
                    writer.dxfReal(42, bulge);
                }
            }
        }

        public DblPoints m_Vertices = new DblPoints();
        public bool m_Closed;

        public void SetConstWidth(double width)
        {
            m_Width = width;
        }
        public bool IsConstWidth()
        {
            return m_startWidths.Count == 0;
        }
        public double GetConstWdith()
        {
            return m_Width;
        }

        public void SetWidth(int Index, double startWidth, double endWidth)
        {
            SetStartWidth(Index, startWidth);
            SetEndWidth(Index, endWidth);
        }

        public void SetStartWidth(int Index, double startWidth)
        {
            Global.ASSERT_DEBUG_INFO("Index < m_Vertices.Count", Index < m_Vertices.Count);
            int count = m_startWidths.Count;
            if (count < Index + 1)
            {
                for (int i = count; i < Index + 1; ++i)
                    m_startWidths.Add(0.0);
            }
            m_startWidths[Index] = startWidth;
        }

        public void SetEndWidth(int Index, double endWidth)
        {
            Global.ASSERT_DEBUG_INFO("Index < m_Vertices.Count", Index < m_Vertices.Count);
            int count = m_endWidths.Count;
            if (count < Index + 1)
            {
                for (int i = count; i < Index + 1; ++i)
                    m_endWidths.Add(0.0);
            }
            m_endWidths[Index] = endWidth;
        }
        public List<double> startWidths() { return m_startWidths; }
        public List<double> endWidths() { return m_endWidths; }

        public void SetBulge(int Index, double Bulge)
        {
            Global.ASSERT_DEBUG_INFO("Index < m_Vertices.Count", Index < m_Vertices.Count);
            int count = m_Bulges.Count;
            if (count < Index + 1)
            {
                for(int i=count; i<Index+1; ++i)
                    m_Bulges.Add(0.0);
            }
            m_Bulges[Index] = Bulge;
        }

        public void PushBulge(double bulge)
        {
            m_Bulges.Add(bulge);
        }

        //C++ TO C# CONVERTER WARNING: 'const' methods are not available in C#:
        //ORIGINAL LINE: double GetBulge(uint Index) const
        public double GetBulge(int Index)
        {
            int count = m_Vertices.Count;
            Global.ASSERT_DEBUG_INFO("Index < count", Index < count);
            if (Index < m_Bulges.Count)
            {
                return m_Bulges[Index];
            }
            return 0.0;
        }
        public void SetBulges(List<double> bulges)
        {
            m_Bulges = bulges;
        }
        //C++ TO C# CONVERTER WARNING: 'const' methods are not available in C#:
        //ORIGINAL LINE: bool HasBulges() const
        public bool HasBulges()
        {
            return m_Bulges.Count > 0;
        }

        private double m_Width;
        private List<double> m_Bulges = new List<double>();
        private List<double> m_startWidths = new List<double>();
        private List<double> m_endWidths = new List<double>();
    }
    public class AcadMText : EntAttribute
    {
        public AcadMText()
        {
            this.m_Width = 100;
            this.m_CharHeight = 0;
            this.m_LineSpacingFactor = 0;
            this.m_RotationAngle = 0;
            this.m_AttachmentPoint = AcAttachmentPoint.acAttachmentPointTopLeft;
            this.m_DrawingDirection = AcDrawingDirection.acLeftToRight;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "MTEXT");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbMText");
            writer.coord(10, m_InsertionPoint.x, m_InsertionPoint.y);
            if (m_CharHeight != 0.0)
            {
                writer.dxfReal(40, m_CharHeight);
            }
            writer.dxfReal(41, m_Width);
            writer.dxfInt(71, (int)m_AttachmentPoint);
            writer.dxfInt(72, (int)m_DrawingDirection);
            writer.dxfText(3, 1, m_Text);
            if (!string.IsNullOrEmpty(m_StyleName))
            {
                writer.dxfString(7, m_StyleName);
            }
            double rotAngle = m_RotationAngle * Math.PI / 180.0;
            writer.dxfReal(11, Math.Cos(rotAngle));
            writer.dxfReal(21, Math.Sin(rotAngle));
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

        public CDblPoint m_InsertionPoint = new CDblPoint(); //对齐点的坐标
        public double m_Width;
        public double m_CharHeight; //值为0.0时表示使用文字样式中的高度
        public string m_Text; // such as hello\Pworld, \P stands for new line.
        public double m_LineSpacingFactor; //行距系数为０时表示使用至少行距风格acLineSpacingStyleAtLeast
        public double m_RotationAngle; //in degrees
        public AcAttachmentPoint m_AttachmentPoint;
        public AcDrawingDirection m_DrawingDirection;
        public string m_StyleName;
    }
    public class AcadPoint : EntAttribute
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "POINT");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbPoint");
            writer.dxfReal(10, m_Point.x);
            writer.dxfReal(20, m_Point.y);
        }

        public CDblPoint m_Point = new CDblPoint();
    }
    public class AcadSolid : EntAttribute
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "SOLID");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbTrace");
            writer.coord(10, m_Point1.x, m_Point1.y);
            writer.coord(11, m_Point2.x, m_Point2.y);
            writer.coord(12, m_Point3.x, m_Point3.y);
            writer.coord(13, m_Point4.x, m_Point4.y);
        }

        public CDblPoint m_Point1 = new CDblPoint();
        public CDblPoint m_Point2 = new CDblPoint();
        public CDblPoint m_Point3 = new CDblPoint();
        public CDblPoint m_Point4 = new CDblPoint();
    }

    public class AcadSpline : EntAttribute
    {
        public AcadSpline()
        {
            this.m_Flag = 8;
            this.m_Degree = 3;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "SPLINE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbSpline");
            writer.dxfInt(70, m_Flag);
            writer.dxfInt(71, m_Degree);
            writer.dxfInt(72, (int)m_Knots.Count);
            writer.dxfInt(73, (int)m_ControlPoints.Count);
            writer.dxfInt(74, (int)m_FitPoints.Count);
            writer.coord(12, m_StartTangent.x, m_StartTangent.y);
            writer.coord(13, m_EndTangent.x, m_EndTangent.y);
            foreach (var knot in m_Knots)
            {
                writer.dxfReal(40, knot);
            }
            foreach (var cp in m_ControlPoints)
            {
                writer.coord(10, cp.x, cp.y);
            }
            foreach (var fp in m_FitPoints)
            {
                writer.coord(11, fp.x, fp.y);
            }
        }

        //样条曲线标志（按位编码）：
        //  1 = 闭合样条曲线
        //  2 = 周期性样条曲线
        //  4 = 有理样条曲线
        //  8 = 平面
        //  16 = 线性（同时还设置平面位）
        public int m_Flag;
        public int m_Degree;
        public CDblPoint m_StartTangent = new CDblPoint();
        public CDblPoint m_EndTangent = new CDblPoint();
        public List<double> m_Knots = new List<double>();
        public List<double> m_Weights = new List<double>();
        public List<CDblPoint> m_ControlPoints = new List<CDblPoint>();
        public List<CDblPoint> m_FitPoints = new List<CDblPoint>();
    }
    public class AcadText : EntAttribute
    {
        public AcadText()
        {
            this.m_RotationAngle = 0.0;
            this.m_HorAlign = 0;
            this.m_VerAlign = 0;
            this.m_Height = 0;
            this.m_WidthFactor = 0.0;
            this.m_IsObliqueAngleValid = false;
            this.m_ObliqueAngle = 0;
        }
        public void SetObliqueAngle(double a)
        {
            m_ObliqueAngle = a;
            m_IsObliqueAngleValid = true;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "TEXT");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbText");
            if (m_HorAlign == 0 && m_VerAlign == 0)
            {
                writer.coord(10, m_BaseLeftPoint.x, m_BaseLeftPoint.y);
            }
            else
            {
                writer.coord(11, m_InsertionPoint.x, m_InsertionPoint.y);
            }
            if (m_Height != 0.0)
            {
                writer.dxfReal(40, m_Height);
            }
            writer.dxfString(1, m_Text);
            if (m_RotationAngle != 0.0)
            {
                writer.dxfReal(50, m_RotationAngle);
            }
            if (m_WidthFactor != 0.0)
            {
                writer.dxfReal(41, m_WidthFactor);
            }
            if (m_IsObliqueAngleValid && m_ObliqueAngle != 0.0)
            {
                writer.dxfReal(51, m_ObliqueAngle);
            }
            if (!string.IsNullOrEmpty(m_StyleName))
            {
                writer.dxfString(7, m_StyleName);
            }
            if (m_HorAlign != 0)
            {
                writer.dxfInt(72, m_HorAlign);
            }
            writer.dxfString(100, "AcDbText");
            if (m_VerAlign != 0)
            {
                writer.dxfInt(73, m_VerAlign);
            }
        }

        // m_BaseLeftPoint可以从m_InsertionPoint, m_HorAlign, m_VerAlign等信息计算出。
        public CDblPoint m_BaseLeftPoint = new CDblPoint(); //文字左下角点
        public CDblPoint m_InsertionPoint = new CDblPoint(); //是文字对齐点，而非文字左下角点
        public string m_Text;
        public double m_RotationAngle; //in degrees
                                       //文字水平对正类型（可选；默认值 = 0）整数代码（非按位编码）
                                       //  0 = 左对正；1 = 居中对正；2 = 右对正
                                       //  3 = 对齐（如果垂直对齐 = 0）
                                       //  4 = 中间（如果垂直对齐 = 0）
                                       //  5 = 两端对齐（如果垂直对齐 = 0）
        public short m_HorAlign;
        //文字垂直对正类型（可选；默认值 = 0）整数代码（不是按位编码）
        //  0 = 基线对正；1 = 底端对正；2 = 居中对正；3 = 顶端对正
        public short m_VerAlign;
        //VerAlign      HorAlign
        //              0       1       2       3       4       5
        //0-基线对正        左       中       右       对齐      中间      两端对齐
        //1-底端对正        左下      中下      右下
        //2-居中对正        左中      正中      右中
        //3-顶端对正        左上      中上      右上

        public string m_StyleName;
        public double m_Height; //值为0.0时表示使用文字样式中的高度
        public double m_WidthFactor; //值为0.0时表示使用文字样式中的宽度系数

        private bool m_IsObliqueAngleValid; //值为false时表示使用文字样式中的文字倾角
        private double m_ObliqueAngle; //in degrees
    }
    public class AcadViewport : EntAttribute
    {
        public AcadViewport()
        {
            this.m_PaperspaceCenter = new CDblPoint(128.5, 97.5);
            this.m_PaperspaceWidth = 205.6;
            this.m_PaperspaceHeight = 156.0;
            this.m_ModelSpaceCenter = new CDblPoint(100, 100);
            this.m_ModelSpaceHeight = 100;
            this.m_TwistAngle = 0;
        }
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
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

            var clipEnt = m_ClipEnt.Target as EntAttribute;
            if (clipEnt != null)
            {
                writer.dxfInt(90, 0x10000 | 0x8060);
                writer.dxfHex(340, clipEnt.m_Handle);
            }
            else
            {
                writer.dxfInt(90, 0x8060);
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

        public CDblPoint m_PaperspaceCenter = new CDblPoint();
        public double m_PaperspaceWidth;
        public double m_PaperspaceHeight;
        public CDblPoint m_ModelSpaceCenter = new CDblPoint();
        public double m_ModelSpaceHeight;
        public double m_TwistAngle; //in degrees

        // if m_ClipEnt is valid, viewport will be clipped by an entity, usually a polygon.
        public WeakReference m_ClipEnt = new WeakReference(null);
    }
    public class AcadXLine : EntAttribute
    {
        internal override void WriteDxf(DxfWriter writer, bool bInPaperSpace)
        {
            writer.dxfString(0, "XLINE");
            WriteAttribute(writer, bInPaperSpace);
            writer.dxfString(100, "AcDbXline");
            writer.coord(10, m_First.x, m_First.y);
            writer.coord(11, m_Second.x, m_Second.y);
        }

        public CDblPoint m_First = new CDblPoint();
        public CDblPoint m_Second = new CDblPoint();
    }
    public class LayoutData
    {
        public LayoutData()
        {
            this.m_MinLim = new CDblPoint(-20, -7.5);
            this.m_MaxLim = new CDblPoint(277, 202);
            this.m_MinExt = new CDblPoint(25.7, 19.5);
            this.m_MaxExt = new CDblPoint(231.3, 175.5);
            this.m_LayoutOrder = 0;
        }
        internal void WriteDxf(DxfWriter writer)
        {
            // TODO: assign ID to viewport
            foreach (var pEnt in m_Objects)
            {
                pEnt.WriteDxf(writer, true);
            }
        }

        // Must be *Paper_Space, *Paper_Space0, *Paper_Space1, ...
        // Don't set it if you create DxfData manually. DxfWriter will set it.
        public string m_BlockName;
        public PlotSettings m_PlotSettings = new PlotSettings();
        // m_MinLim and m_MaxLim defines the limits in paperspace.
        public CDblPoint m_MinLim = new CDblPoint(); // in millimeters
        public CDblPoint m_MaxLim = new CDblPoint(); // in millimeters
                                                     // m_MinExt and m_MaxExt defines the default viewport size in paperspace.
        public CDblPoint m_MinExt = new CDblPoint(); // in millimeters
        public CDblPoint m_MaxExt = new CDblPoint(); // in millimeters
                                                     // 包含AcadViewport和AcadPolygonViewport对象
        public EntityList m_Objects = new EntityList();
        // Don't set it if you create DxfData manually. DxfWriter will set it.
        public int m_LayoutOrder;
    }

    public class BlockDef
    {

        public CDblPoint m_InsertPoint = new CDblPoint();
        public EntityList m_Objects = new EntityList();
    }
}
