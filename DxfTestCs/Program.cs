using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using DXF;
using System.IO;

namespace DxfTestCs
{
    class Program
    {
        static void Main(string[] args)
        {
            String curfile = System.Reflection.Assembly.GetExecutingAssembly().Location;
            String curDirectory = Path.GetDirectoryName(curfile);

            string text = "一二三四五六七八九二二三四五六七八九三二三四五六七八九四二三四五六七八九五二三四五六七八九六二三四五六七八九七二三四五六七八九八二三四五六七八九九二三四五六七八九十二三四五六七八九";
            int len = text.Length;
            int start = 0;
            int cur = start;
            while (true)
            {
                cur = Global.AdvanceUtf8(text, start, 250);
                if (cur == len || cur == -1)
                {
                    Console.WriteLine(text.Substring(start, cur - start));
                    break;
                }
                Console.WriteLine(text.Substring(start, cur - start));
                start = cur;
            }

            // Test DXF read/write
            Global.EnableDebugInfo(true);
            DxfData dxf = new DxfData();
            //auto pDim = std::make_shared<AcadDimAln>();
            //pDim->m_DefPoint.SetPoint(80, 120);
            //pDim->m_TextPosition.SetPoint(30, 70);
            //pDim->m_ExtLine1Point.SetPoint(0, 0);
            //pDim->m_ExtLine2Point.SetPoint(100, 100);
            //pDim->m_DimStyleName = "ISO-25";
            //dxf.m_Objects.push_back(pDim);
            //auto pTable = std::make_shared<AcadTable>();
            //pTable->m_InsertionPoint.SetPoint(0, 0);
            //pTable->InitializeCells(2, 2, DXF::CellType::TEXT_CELL);
            //pTable->SetColWidth(600);
            //pTable->SetRowHeight(330);
            //for (int i=0; i<pTable->m_RowCount; ++i)
            //	for (int j = 0; j < pTable->m_ColCount; ++j)
            //	{
            //		Cell& c = pTable->GetCell(i, j);
            //		c.m_TextHeight = 250;
            //		std::shared_ptr<CellText>& ct = dynamic_pointer_cast<CellText>(c.m_Content);
            //		ct->m_Text = "0*0";
            //	}
            //dxf.m_Objects.push_back(pTable);

            //====================== 文字样式 ====================== 
            TextStyleData txtStyle = new TextStyleData();
            //
            txtStyle.m_TrueType = "宋体";  //SimSun 
            //
            txtStyle.m_PrimaryFontFile = "fsdb_e.shx";
            txtStyle.m_BigFontFile = "hztxt.shx";
            //
            txtStyle.m_ObliqueAngle = 20;
            txtStyle.m_WidthFactor = 0.7;
            txtStyle.m_Height = 2.5;
            //
            dxf.m_TextStyles.Add("字体1", txtStyle);


            //====================== 标注样式 ====================== 
            DimStyleData dimStyle = new DimStyleData();
            dimStyle.TextStyle = "字体1";
            dimStyle.VerticalTextPosition = AcDimVerticalJustification.acAbove;
            //
            dxf.m_DimStyles.Add("标注1", dimStyle);

            AcadDimRot dim = new AcadDimRot();
            dim.m_DefPoint.SetPoint(80, 120);
            dim.m_TextPosition.SetPoint(30, 70);
            dim.m_ExtLine1Point.SetPoint(0, 0);
            dim.m_ExtLine2Point.SetPoint(100, 100);
            dim.m_DimStyleName = "标注1";
            dxf.m_Objects.Add(dim);


            //====================== 填充 ====================== 
            var pH = new AcadHatch();
            var pC = new AcadCircle();
            pC.m_Center.SetPoint(10, 10);
            pC.m_Radius = 50;
            pH.AddEntity(pC, -1);
            pH.AddAssociatedEntity(pC, -1);
            pH.m_PatternName = "SOLID";
            pC.m_Reactors.Add(new WeakReference(pH));
            dxf.m_Objects.Add(pC);
            dxf.m_Objects.Add(pH);

            //====================== 线宽 ====================== 
            var pP2a = new AcadLWPLine();
            pP2a.m_Vertices.Add(new CDblPoint(20, 0));
            pP2a.m_Vertices.Add(new CDblPoint(30, 0));
            //pP2a.SetStartWidth(0, 0);
            //pP2a.SetEndWidth(0, 10);
            pP2a.SetWidth(0, 0, 10);
            // pP2a.SetWidth(1, 10, 10);
            dxf.m_Objects.Add(pP2a);

            var pP2b = new AcadLWPLine();
            pP2b.m_Vertices.Add(new CDblPoint(20, 20));
            pP2b.m_Vertices.Add(new CDblPoint(30, 20));
            pP2b.SetConstWidth(10);
            dxf.m_Objects.Add(pP2b);

            //====================== 填充 ====================== 
            var pP3 = new AcadLWPLine();
            pP3.m_Vertices.Add(new CDblPoint(30, 30));
            pP3.m_Vertices.Add(new CDblPoint(40, 40));
            pP3.m_Vertices.Add(new CDblPoint(40, 20));
            pP3.m_Closed = true;

            var pH3 = new AcadHatch();
            pH3.AddEntity(pP3, -1);
            pH3.AddAssociatedEntity(pP3, -1);
            pH3.m_PatternName = "SOLID";
            pP3.m_Reactors.Add(new WeakReference(pH3));
            dxf.m_Objects.Add(pP3);
            dxf.m_Objects.Add(pH3);


            DxfWriter dxfWriter = new DxfWriter(dxf);
            //dxfWriter.WriteDxf(@"G:\SharedWithVM\test2.dxf",  @"..\DxfCs\acadiso.lin", @"..\DxfCs\acadiso.pat", new CDblPoint(0, 0), 100);
            dxfWriter.WriteDxf(curDirectory + @"\test2.dxf", curDirectory + @"\acadiso.lin", curDirectory + @"\acadiso.pat", new CDblPoint(0, 0), 100);
            if (args.Length == 1)
            {
                DxfReader dxfReader = new DxfReader(dxf);
                if (dxfReader.ReadDxf(args[0], false))
                {
                    dxfWriter.WriteDxf(@"G:\SharedWithVM\test3.dxf", @"..\DXF\acadiso.lin", @"..\DXF\acadiso.pat", new CDblPoint(2000, 1400), 3300);
                }
            }
        }
    }
}
