using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace DXF
{
    using VectorOfEntityList = List<EntityList>;
    public class DxfWriter
    {
        public DxfWriter(DxfData graph)
        {
            this.m_Graph = graph;
            this.m_Handle = 0x2FF;
            this.m_LayoutIndex = 0;
        }
        public void WriteDxf(string dxfFile, string linFile, string patFile, CDblPoint center, double height)
        {
            //RandomNumbers.Seed((uint)time(null));

            try
            {
                m_LinetypeManager.SetLinFile(linFile);
                if (!m_LinetypeManager.IsValid())
                {
                    return;
                }
                m_PatternManager.SetPatFile(patFile);
                if (!m_PatternManager.IsValid())
                {
                    return;
                }
                try
                {
                    m_DxfFile = new StreamWriter(dxfFile);

                }
                catch(Exception)
                {
                    Global.PRINT_DEBUG_INFO("Can't open file {0}，Maybe because it is opened by AutoCAD.", dxfFile);
                    return;
                }

                // Clear writing state
                Reset();
                InitPredefinedBlocks();
                int handle = m_Graph.PrepareBeforeWrite();
                PrepareBeforeWrite(handle);

                // Header
                HeaderSection();

                // Section TABLES
                BeginSection("TABLES");
                {
                    VPortTable(center, height);

                    BeginLTypeTable(3 + (int)m_Graph.m_Linetypes.Count);
                    LTypeEntry_ByBlock_ByLayer_Continuous();
                    foreach (string linetype in m_Graph.m_Linetypes)
                    {
                        LTypeEntry(linetype, ++m_Handle);
                    }
                    EndTable();

                    BeginLayerTable((int)m_Graph.m_Layers.Count);
                    foreach (var layer in m_Graph.m_Layers)
                    {
                        LayerEntry(layer, ++m_Handle);
                    }
                    EndTable();

                    BeginStyleTable((int)m_Graph.m_TextStyles.Count);
                    foreach (var textStyle in m_Graph.m_TextStyles)
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
                        string blockName = Global.GetPredefinedArrowHeadBlockName((AcDimArrowheadType)i);
                        // If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
                        if (m_Graph.m_RealBlockDefs.ContainsKey(blockName))
                        {
                            continue;
                        }
                        ++predefinedBlockCount;
                    }
                    BeginBlock_RecordTable(1 + (int)(m_Graph.m_Layouts.Count + m_Graph.m_RealBlockDefs.Count) + predefinedBlockCount);
                    BlockRecordEntry_ModelSpace();
                    foreach (var pair in m_Graph.m_Layouts)
                    {
                        BlockRecordEntry(pair.Value.m_BlockName, ++m_Handle, ++m_Handle);
                    }
                    foreach (var pair in m_Graph.m_RealBlockDefs)
                    {
                        BlockRecordEntry(pair.Key, ++m_Handle, 0);
                    }
                    for (int i = 0; i < 20; ++i)
                    {
                        string blockName = Global.GetPredefinedArrowHeadBlockName((AcDimArrowheadType)i);
                        // If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
                        if (m_Graph.m_RealBlockDefs.ContainsKey(blockName))
                        {
                            continue;
                        }
                        BlockRecordEntry(blockName, ++m_Handle, 0);
                    }
                    EndTable();

                    BeginDimStyleTable((int)m_Graph.m_DimStyles.Count);
                    foreach (var dimStyle in m_Graph.m_DimStyles)
                    {
                        DimStyleEntry(dimStyle, ++m_Handle);
                    }
                    EndTable();
                }
                EndSection();

                BeginSection("BLOCKS");
                {
                    Block_ModelSpace();
                    foreach (var pair in m_Graph.m_Layouts)
                    {
                        var pLayout = pair.Value;
                        BeginBlock(pLayout.m_BlockName, 0, 0, ++m_Handle);
                        // *Paper_Space 块定义内不允许有图元
                        if (pLayout.m_BlockName != "*Paper_Space")
                        {
                            // viewport need write ID which starts from 1
                            m_CurrentViewportID = 0;
                            m_CurrentBlockEntryHandle = LookupBlockEntryHandle(pLayout.m_BlockName);
                            pLayout.WriteDxf(this);
                        }
                        EndBlock(pLayout.m_BlockName, ++m_Handle);
                    }
                    foreach (var pair in m_Graph.m_RealBlockDefs)
                    {
                        m_CurrentBlockEntryHandle = LookupBlockEntryHandle(pair.Key);
                        Block(pair, false);
                    }
                    for (int i = 0; i < (int)s_PredefinedBlocks.Count; ++i)
                    {
                        string blockName = Global.GetPredefinedArrowHeadBlockName((AcDimArrowheadType)i);
                        // If this predefined block is defined in m_RealBlockDefs, don't repeat definition.
                        if (m_Graph.m_RealBlockDefs.ContainsKey(blockName))
                        {
                            continue;
                        }
                        BeginBlock(blockName, 0, 0, ++m_Handle);
                        foreach (var pEnt in s_PredefinedBlocks[i])
                        {
                            m_CurrentBlockEntryHandle = LookupBlockEntryHandle(blockName);
                            pEnt.WriteDxf(this, false);
                        }
                        EndBlock(blockName, ++m_Handle);
                    }
                }
                EndSection();

                BeginSection("ENTITIES");
                {
                    m_CurrentBlockEntryHandle = LookupBlockEntryHandle("*Model_Space");
                    foreach (var pEnt in m_Graph.m_Objects)
                    {
                        pEnt.WriteDxf(this, false);
                    }
                    // *Paper_Space 块定义内不允许有图元
                    // *Paper_Space 块中的图元定义在ENTITIES段
                    foreach (var pair in m_Graph.m_Layouts)
                    {
                        if (pair.Value.m_BlockName == "*Paper_Space")
                        {
                            // viewport need write ID which starts from 1
                            m_CurrentViewportID = 0;
                            m_CurrentBlockEntryHandle = LookupBlockEntryHandle("*Paper_Space");
                            pair.Value.WriteDxf(this);
                        }
                    }
                }
                EndSection();

                BeginSection("OBJECTS");
                {
                    Dictionaries();

                    // Write Layouts
                    var modelLayout = new KeyValuePair<string, LayoutData>("Model", new LayoutData());
                    modelLayout.Value.m_BlockName = "*Model_Space";
                    Layout(modelLayout);
                    foreach (var pair in m_Graph.m_Layouts)
                    {
                        Layout(pair);
                    }

                    // Write MLEADERSTYLE
                    foreach (var ls in m_Graph.m_MLeaderStyles)
                    {
                        ls.Value.WriteDxf(this, ls.Key, m_MLeaderStyleHandleTable[ls.Key]);
                    }

                    // Write TABLESTYLE
                    foreach (var ts in m_Graph.m_TableStyles)
                    {
                        ts.Value.WriteDxf(this, ts.Key, m_TableStyleHandleTable[ts.Key]);
                    }
                }
                EndSection();

                dxfEOF();

                m_DxfFile.Close();
            }
            catch (Exception e)
            {
                Global.PrintDebugInfo(e.StackTrace);
            }
        }

        private void BeginSection(string name)
        {
            dxfString(0, "SECTION");
            dxfString(2, name);
        }

        private void EndSection()
        {
            dxfString(0, "ENDSEC");
        }

        private void EndTable()
        {
            dxfString(0, "ENDTAB");
        }

        private void BeginLTypeTable(int num)
        {
            BeginTable("LTYPE", num, 5);
        }

        private void BeginLayerTable(int num)
        {
            BeginTable("LAYER", num, 2);
        }

        private void BeginStyleTable(int num)
        {
            BeginTable("STYLE", num, 3);
        }

        private void BeginDimStyleTable(int num)
        {
            BeginTable("DIMSTYLE", num, 0xA);
            dxfString(100, "AcDbDimStyleTable");
            dxfInt(71, 0);
            //下面一句似乎是指定缺省的或者当前的DimStyle
            dxfHex(340, 0x27); //handle of dimstyle ISO-25
        }

        //num = number of layouts + number of blocks + 2(*Model_Space,*Paper_Space)
        private void BeginBlock_RecordTable(int num)
        {
            BeginTable("BLOCK_RECORD", num, 1);
        }

        //还有一些标注箭头块需要添加
        private void BlockRecordEntry_ModelSpace()
        {
            BlockRecordEntry("*Model_Space", 0x1F, 0x22);
        }
        //handle是条目所指对象的句柄
        private void DictionaryEntry(string entryName, int handle)
        {
            dxfString(3, entryName);
            dxfHex(350, handle);
        }

        //在ENTITIES段中先输出输出模型空间图元，再*Paper_Space图纸空间图元。
        //区分图元属于模型空间和图纸空间的标志是组码 67。
        //*Paper_Space(n)图纸空间图元输出在BLOCKS段中


        //二进制DXF不得调用
        internal void Comment(string text)
        {
            dxfString(999, text);
        }

        internal void dxfEOF()
        {
            dxfString(0, "EOF");
        }

        internal void coord(int gc, double x, double y)
        {
            dxfReal(gc, x);
            dxfReal(gc + 10, y);
        }

        internal void coord(int gc, double[] value)
        {
            //_ASSERTE(value);
            coord(gc, value[0], value[1]);
        }

        internal Linetype FindLinetype(string lineTypeName)
        {
            return m_LinetypeManager.FindLinetype(lineTypeName);
        }

        internal Pattern FindPattern(string ptname)
        {
            return m_PatternManager.FindPattern(ptname);
        }

        private void Reset()
        {
            m_Handle = 0x2FF;
            m_LayoutIndex = 0;
            m_TextStyleHandleTable.Clear();
            m_DimStyleHandleTable.Clear();
            m_BlockEntryHandleTable.Clear();
            m_LayoutHandleTable.Clear();
            m_MLeaderStyleHandleTable.Clear();
            m_TableStyleHandleTable.Clear();
            m_LinetypeHandleTable.Clear();
            m_MLeaderStyleEntities.Clear();
            m_TableStyleEntities.Clear();
        }

        /**
        * Writes a real (double) variable to the DXF file.
        *
        * @param gc Group code.
        * @param value Double value
        */
        internal void dxfReal(int gc, double value)
        {
            string str = value.ToString("G", CultureInfo.InvariantCulture);
            dxfString(gc, str);
        }
        /**
        * Writes an int variable to the DXF file.
        *
        * @param gc Group code.
        * @param value Int value
        */
        internal void dxfInt(int gc, int value)
        {
            m_DxfFile.WriteLine(gc);
            m_DxfFile.WriteLine(value);
        }
        /**
        * Writes a hex int variable to the DXF file.
        *
        * @param gc Group code.
        * @param value Int value
        */
        internal void dxfHex(int gc, int value)
        {
            string str = string.Format("{0:X}", value);
            dxfString(gc, str);
        }
        internal void dxfString(int gc, string value)
        {
            //ASSERT_DEBUG_INFO(value);
            m_DxfFile.WriteLine(gc);
            m_DxfFile.WriteLine(value);
        }
        internal void dxfString(int gc, string value, int count)
        {
            dxfString(gc, value.Substring(0, count));
        }
        internal void dxfText(int gc, int gcLast, string text)
        {
            int len = text.Length;
            int start = 0;
            int cur = start;
            while (true)
            {
                cur = Global.AdvanceUtf8(text, start, 250);
                if (cur == len)
                {
                    dxfString(gcLast, text.Substring(start, cur - start));
                    break;
                }
                dxfString(gcLast, text.Substring(start, cur - start));
                start = cur;
            }
        }
        private int PrepareBeforeWrite(int handle)
        {
            foreach (var msi in m_Graph.m_MLeaderStyles)
            {
                m_MLeaderStyleHandleTable[msi.Key] = ++handle;
            }
            foreach (var tsi in m_Graph.m_TableStyles)
            {
                m_TableStyleHandleTable[tsi.Key] = ++handle;
            }
            return handle;
        }
        internal int LookupTextStyleHandle(string name)
        {
            int value;
            bool hasIt = m_TextStyleHandleTable.TryGetValue(name, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupDimStyleHandle(string name)
        {
            int value;
            bool hasIt = m_DimStyleHandleTable.TryGetValue(name, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupBlockEntryHandle(string blockName)
        {
            int value;
            bool hasIt = m_BlockEntryHandleTable.TryGetValue(blockName, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupLayoutHandle(string blockName)
        {
            int value;
            bool hasIt = m_LayoutHandleTable.TryGetValue(blockName, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupMLeaderStyleHandle(string name)
        {
            int value;
            bool hasIt = m_MLeaderStyleHandleTable.TryGetValue(name, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupTableStyleHandle(string name)
        {
            int value;
            bool hasIt = m_TableStyleHandleTable.TryGetValue(name, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal int LookupLinetypeHandle(string name)
        {
            int value;
            bool hasIt = m_LinetypeHandleTable.TryGetValue(name, out value);
            if (hasIt)
            {
                return value;
            }
            return 0;
        }
        internal void AddMLeaderToItsStyle(string mleaderStyle, EntAttribute pMLeader)
        {
            if (!string.IsNullOrEmpty(mleaderStyle))
            {
                List<EntAttribute> list = null;
                bool hasIt = m_MLeaderStyleEntities.TryGetValue(mleaderStyle, out list);
                if (!hasIt)
                {
                    list = new List<EntAttribute>();
                }
                list.Add(pMLeader);
            }
        }
        internal List<EntAttribute> GetMLeaders(string mleaderStyle)
        {
            Global.ASSERT_DEBUG_INFO("!string.IsNullOrEmpty(mleaderStyle)", !string.IsNullOrEmpty(mleaderStyle));
            List<EntAttribute> res = null;
            bool hasIt = m_MLeaderStyleEntities.TryGetValue(mleaderStyle, out res);
            if (hasIt)
                return res;
            else
                return new List<EntAttribute>();
        }
        internal void AddTableToItsStyle(string tableStyle, EntAttribute pTable)
        {
            if (!string.IsNullOrEmpty(tableStyle))
            {
                List<EntAttribute> list = null;
                bool hasIt = m_TableStyleEntities.TryGetValue(tableStyle, out list);
                if (!hasIt)
                {
                    list = new List<EntAttribute>();
                }
                list.Add(pTable);
            }
        }
        internal List<EntAttribute> GetTables(string tableStyle)
        {
            Global.ASSERT_DEBUG_INFO("!string.IsNullOrEmpty(tableStyle)", !string.IsNullOrEmpty(tableStyle));
            List<EntAttribute> res = null;
            bool hasIt = m_TableStyleEntities.TryGetValue(tableStyle, out res);
            if (hasIt)
                return res;
            else
                return new List<EntAttribute>();
        }
        private void HeaderSection()
        {
            Global.ASSERT_DEBUG_INFO("m_Graph.m_LinetypeScale > 0.0", m_Graph.m_LinetypeScale > 0.0);
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
            dxfInt(70, 1); //使用公制而不是英制
            dxfString(9, "$HANDSEED");
            dxfHex(5, 0xFFFFF);
            EndSection();
        }
        private void BeginTable(string name, int num, int handle)
        {
            dxfString(0, "TABLE");
            dxfString(2, name);
            dxfHex(5, handle);
            dxfHex(330, 0);
            dxfString(100, "AcDbSymbolTable");
            dxfInt(70, num);
        }
        private void TableEntryHead(string name, int handle, int tableHandle)
        {
            dxfString(0, name);
            int gc;
            if (string.Compare("DIMSTYLE", name, true) == 0)
            {
                gc = 105;
            }
            else
            {
                gc = 5;
            }
            dxfHex(gc, handle);
            dxfHex(330, tableHandle);
            dxfString(100, "AcDbSymbolTableRecord");
        }
        internal void VPortTable(CDblPoint center, double height)
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
            dxfInt(75, 0); //关闭捕捉
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
        private void LTypeEntry_ByBlock_ByLayer_Continuous()
        {
            Linetype lt = new Linetype();
            lt.line_name = "ByBlock";
            LTypeEntry(lt, 0x14);
            lt.line_name = "ByLayer";
            LTypeEntry(lt, 0x15);
            lt.line_name = "Continuous";
            lt.line_descrip = "Solid line";
            LTypeEntry(lt, 0x16);
        }
        private void LTypeEntry(string name, int handle)
        {
            Linetype lt = FindLinetype(name);
            Global.ASSERT_DEBUG_INFO("lt != null", lt != null);
            LTypeEntry(lt, handle);
        }
        private void LTypeEntry(Linetype lt, int handle)
        {
            m_LinetypeHandleTable[lt.line_name] = handle;
            TableEntryHead("LTYPE", handle, 5);
            dxfString(100, "AcDbLinetypeTableRecord");
            dxfString(2, lt.line_name);
            dxfInt(70, 0);
            dxfString(3, lt.line_descrip);
            dxfInt(72, 65);
            dxfInt(73, (int)lt.descriptors.Count);
            double len = 0;
            for (int i = 0; i < (int)lt.descriptors.Count; ++i)
            {
                len += Math.Abs(lt.descriptors[i].scale_length);
            }
            dxfReal(40, len);
            for (int i = 0; i < (int)lt.descriptors.Count; ++i)
            {
                dxfReal(49, lt.descriptors[i].scale_length);
                dxfInt(74, 0);
            }
        }
        internal void LayerEntry(KeyValuePair<string, LayerData> pair, int handle)
        {
            var name = pair.Key;
            Global.ASSERT_DEBUG_INFO("name.Length != 0", name.Length != 0);

            var data = pair.Value;
            int color = data.m_Color != 0 ? (int)data.m_Color : 7;
            string ltname = data.m_Linetype.Length == 0 ? "CONTINUOUS" : data.m_Linetype;

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
            if (data.m_LineWeight != AcLineWeight.acLnWtByLayer)
            {
                dxfInt(370, (int)data.m_LineWeight);
            }
            dxfHex(390, 0xF); //PlotStyleName 对象的硬指针 ID/句柄
        }
        private class File2Face
        {
            internal readonly string FileName;
            internal readonly string CadFaceName;
            internal readonly string WinFaceName;
            internal File2Face(string fn, string cadFn, string wFn)
            {
                FileName = fn;
                CadFaceName = cadFn;
                WinFaceName = wFn;
            }
        }
        //以下表格的第二列是大小写敏感的
        private static File2Face[] F2FMap =
        {
        new File2Face("SimSun.ttf", "SimSun", "宋体"),
        new File2Face("SIMHEI.TTF", "SimHei", "黑体"),
        new File2Face("SIMKAI.TTF", "KaiTi_GB2312", "楷体_GB2312"),
        new File2Face("SIMLI.TTF", "LiSu", "隶书"),
        new File2Face("SIMFANG.TTF", "FangSong_GB2312", "仿宋_GB2312"),
        new File2Face("SIMYOU.TTF", "YouYuan", "幼圆"),
        new File2Face("STCAIYUN.TTF", "STCaiyun", "华文彩云"),
        new File2Face("STFANGSO.TTF", "STFangsong", "华文仿宋"),
        new File2Face("STXIHEI.TTF", "STXihei", "华文细黑"),
        new File2Face("STXINGKA.TTF", "STXingkai", "华文行楷"),
        new File2Face("STXINWEI.TTF", "STXinwei", "华文新魏"),
        new File2Face("STZHONGS.TTF", "STZhongsong", "华文中宋"),
        new File2Face("FZSTK.TTF", "FZShuTi", "方正舒体"),
        new File2Face("FZYTK.TTF", "FZYaoTi", "方正姚体"),
        new File2Face("SURSONG.TTF", "Simsun (Founder Extended)", "宋体-方正超大字符集"),
        new File2Face("NSimSun.ttf", "NSimSun", "新宋体")
    };

        private static string WinFaceFromFileName(string FileName)
        {
            for (int i = 0; i < F2FMap.Length; ++i)
            {
                if (string.Compare(F2FMap[i].FileName, FileName, true) == 0)
                {
                    return F2FMap[i].WinFaceName;
                }
            }
            return F2FMap[0].WinFaceName;
        }

        private static string FileNameFromWinFace(string WinFaceName)
        {
            for (int i = 0; i < F2FMap.Length; ++i)
            {
                if (string.Compare(F2FMap[i].WinFaceName, WinFaceName, true) == 0)
                {
                    return F2FMap[i].FileName;
                }
            }
            return F2FMap[0].FileName;
        }

        private static string CadFaceFromFileName(string FileName)
        {
            for (int i = 0; i < F2FMap.Length; ++i)
            {
                if (string.Compare(F2FMap[i].FileName, FileName, true) == 0)
                {
                    return F2FMap[i].CadFaceName;
                }
            }
            return F2FMap[0].CadFaceName;
        }

        private static string FileNameFromCadFace(string cadFace)
        {
            for (int i = 0; i < F2FMap.Length; ++i)
            {
                if (string.Compare(F2FMap[i].CadFaceName, cadFace) == 0)
                {
                    return F2FMap[i].FileName;
                }
            }
            return F2FMap[0].FileName;
        }
        private void StyleEntry(KeyValuePair<string, TextStyleData> pair, int handle)
        {
            var name = pair.Key;
            var data = pair.Value;
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
            if (data.m_TrueType.Length != 0)
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
        private void ViewTable()
        {
            BeginTable("VIEW", 0, 6);
            EndTable();
        }
        private void UCSTable()
        {
            BeginTable("UCS", 0, 7);
            EndTable();
        }
        private void AppIdTable()
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
        internal void DimStyleEntry(KeyValuePair<string, DimStyleData> pair, int handle)
        {
            var name = pair.Key;
            DimStyleData data = pair.Value;
            m_DimStyleHandleTable[name] = handle;
            TableEntryHead("DIMSTYLE", handle, 0xA);
            dxfString(100, "AcDbDimStyleTableRecord");
            dxfString(2, name);
            dxfInt(70, 0);
            if (data.Text.Length != 0)
            {
                dxfString(3, data.Text);
                dxfString(4, data.Text);
            }
            dxfReal(41, data.ArrowHeadSize);
            dxfReal(42, data.ExtensionLineOffset);
            dxfReal(43, 3.75); //DIMDLI(尺寸线增量)
            dxfReal(44, data.ExtensionLineExtend);
            dxfReal(140, data.TextHeight);
            double centermarksize;
            if (data.CenterType == AcDimCenterType.acCenterMark)
            {
                centermarksize = data.CenterMarkSize;
            }
            else if (data.CenterType == AcDimCenterType.acCenterNone)
            {
                centermarksize = 0;
            }
            else
            {
                centermarksize = -data.CenterMarkSize;
            }
            dxfReal(141, centermarksize);
            if (data.LinearScaleFactor != 1.0)
            {
                dxfReal(144, data.LinearScaleFactor);
            }
            if (AcDimVerticalJustification.acAbove == data.VerticalTextPosition)
            {
                dxfInt(77, 1);
            }
            else
            {
                dxfInt(145, (int)data.VerticalTextPosition);
            }
            dxfReal(147, data.TextGap);
            dxfInt(73, data.TextAlign ? 1 : 0);
            dxfInt(74, data.TextAlign ? 1 : 0);
            dxfInt(75, data.ExtLine1Suppress ? 1 : 0);
            dxfInt(76, data.ExtLine2Suppress ? 1 : 0);
            int flag = 0;
            if (data.SuppressLeadingZeros)
            {
                flag |= 4;
            }
            if (data.SuppressTrailingZeros)
            {
                flag |= 8;
            }
            dxfInt(78, flag);
            flag = 0;
            if (data.AngleSuppressLeadingZeros)
            {
                flag |= 1;
            }
            if (data.AngleSuppressTrailingZeros)
            {
                flag |= 2;
            }
            dxfInt(79, flag);
            dxfInt(171, (int)data.UnitsPrecision);
            dxfInt(172, data.ForceLineInside ? 1 : 0);
            dxfInt(173, 1); //非零时使用单独的箭头块
            dxfInt(174, data.TextInside ? 1 : 0);
            dxfInt(176, (int)data.DimensionLineColor);
            dxfInt(177, (int)data.ExtensionLineColor);
            dxfInt(178, (int)data.TextColor);
            dxfInt(179, (int)data.AnglePrecision);
            dxfInt(271, (int)data.UnitsPrecision);
            dxfInt(273, (int)data.UnitsFormat);
            dxfInt(275, (int)data.AngleFormat);
            dxfInt(277, (int)data.UnitsFormat);
            dxfInt(278, '.');
            dxfInt(279, (int)data.TextMovement);
            dxfInt(280, (int)data.HorizontalTextPosition);
            dxfInt(281, data.DimLine1Suppress ? 1 : 0);
            dxfInt(282, data.DimLine2Suppress ? 1 : 0);
            dxfInt(289, (int)data.Fit);
            dxfHex(340, LookupTextStyleHandle((string)data.TextStyle));
            if (data.ArrowHead1Type == AcDimArrowheadType.acArrowUserDefined)
            {
                dxfHex(343, LookupBlockEntryHandle((string)data.ArrowHead1Block));
            }
            else
            {
                string blockName = Global.GetPredefinedArrowHeadBlockName(data.ArrowHead1Type);
                dxfHex(343, LookupBlockEntryHandle(blockName));
            }
            if (data.ArrowHead2Type == AcDimArrowheadType.acArrowUserDefined)
            {
                dxfHex(344, LookupBlockEntryHandle((string)data.ArrowHead2Block));
            }
            else
            {
                string blockName = Global.GetPredefinedArrowHeadBlockName(data.ArrowHead2Type);
                dxfHex(344, LookupBlockEntryHandle(blockName));
            }
            dxfInt(371, (int)data.DimensionLineWeight);
            dxfInt(372, (int)data.ExtensionLineWeight);
        }
        internal void BlockRecordEntry(string blockname, int handle, int layoutHandle)
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
        internal void BeginBlock(string blockName, double x, double y, int handle, bool bDimObliqueBlock = false)
        {
            dxfString(0, "BLOCK");
            dxfHex(5, handle);
            dxfHex(330, LookupBlockEntryHandle(blockName));
            dxfString(100, "AcDbEntity");
            if (string.Compare(blockName, 0, "*Paper_Space", 0, "*Paper_Space".Length) == 0)
            {
                dxfInt(67, 1);
            }
            dxfString(8, "0");
            dxfString(100, "AcDbBlockBegin");
            dxfString(2, blockName);
            dxfInt(70, bDimObliqueBlock ? 1 : 0);
            coord(10, x, y);
            dxfString(3, blockName);
            dxfString(1, "");
        }
        internal void EndBlock(string blockname, int handle)
        {
            dxfString(0, "ENDBLK");
            dxfHex(5, handle);
            dxfHex(330, LookupBlockEntryHandle(blockname));
            dxfString(100, "AcDbEntity");

            if (blockname.StartsWith("*Paper_Space"))
            {
                dxfInt(67, 1);
            }
            dxfString(8, "0");
            dxfString(100, "AcDbBlockEnd");
        }
        internal void Block_ModelSpace()
        {
            BeginBlock("*Model_Space", 0, 0, 0x20);
            EndBlock("*Model_Space", 0x21);
            //BeginBlock("*Paper_Space", 0, 0, ++m_Handle);
            //EndBlock("*Paper_Space", ++m_Handle);
        }
        internal void Block(KeyValuePair<string, BlockDef> pair, bool bInPaperSpace)
        {
            var blockName = pair.Key;
            var blockDef = pair.Value;
            BeginBlock(blockName, blockDef.m_InsertPoint.x, blockDef.m_InsertPoint.y, ++m_Handle);
            foreach (var pEnt in blockDef.m_Objects)
            {
                pEnt.WriteDxf(this, bInPaperSpace);
            }
            EndBlock(blockName, ++m_Handle);
        }
        internal void Dictionary(int handle, int parentHandle)
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
        internal void Dictionaries()
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
            foreach (var pair in m_Graph.m_Layouts)
            {
                var pLayout = pair.Value;
                DictionaryEntry(pair.Key, LookupLayoutHandle(pLayout.m_BlockName));
            }
            // End of ACAD_LAYOUT
            Dictionary(0x6A, 0xC); // ACAD_MATERIAL
                                   // Begin of ACAD_MLEADERSTYLE
            Dictionary(0x12D, 0xC);
            foreach (var pair in m_MLeaderStyleHandleTable)
            {
                DictionaryEntry(pair.Key, pair.Value);
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
            foreach (var pair in m_TableStyleHandleTable)
            {
                DictionaryEntry(pair.Key, pair.Value);
            }
            // End of ACAD_TABLESTYLE
            Dictionary(0xEF, 0xC); // ACAD_VISUALSTYLE
                                   //Dictionary(0x2F0, 0xC); // ACDB_RECOMPOSE_DATA
            Dictionary(0x5E, 0xC); // AcDbVariableDictionary
        }
        internal void Layout(KeyValuePair<string, LayoutData> pair)
        {
            var layoutName = pair.Key;
            var pLayout = pair.Value;
            bool isPaperSpace = pLayout.m_BlockName != "*Model_Space";
            dxfString(0, "LAYOUT");
            dxfHex(5, m_LayoutHandleTable[pLayout.m_BlockName]);
            dxfString(102, "{ACAD_REACTORS");
            dxfHex(330, 0x1A);
            dxfString(102, "}");
            dxfHex(330, 0x1A);
            dxfString(100, "AcDbPlotSettings");
            dxfString(1, "");
            dxfString(2, pLayout.m_PlotSettings.m_PlotConfigFile);
            dxfString(4, pLayout.m_PlotSettings.m_PaperName);
            dxfString(6, "");
            dxfReal(40, pLayout.m_PlotSettings.m_LeftMargin);
            dxfReal(41, pLayout.m_PlotSettings.m_BottomMargin);
            dxfReal(42, pLayout.m_PlotSettings.m_RightMargin);
            dxfReal(43, pLayout.m_PlotSettings.m_TopMargin);
            dxfReal(44, pLayout.m_PlotSettings.m_Width);
            dxfReal(45, pLayout.m_PlotSettings.m_Height);
            dxfReal(46, pLayout.m_PlotSettings.m_PlotOrigin.x);
            dxfReal(47, pLayout.m_PlotSettings.m_PlotOrigin.y);
            dxfReal(48, 0);
            dxfReal(49, 0);
            dxfReal(140, 0);
            dxfReal(141, 0);
            dxfReal(142, 1.0);
            dxfReal(143, 1.0);
            if (isPaperSpace)
            {
                dxfInt(70, 640);
            }
            else
            {
                dxfInt(70, 1712);
            }
            dxfInt(72, 1);
            dxfInt(73, pLayout.m_PlotSettings.m_PlotRotation);
            if (isPaperSpace)
            {
                dxfInt(74, 5);
            }
            else
            {
                dxfInt(74, 0);
            }
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
            dxfInt(71, pLayout.m_LayoutOrder);
            coord(10, pLayout.m_MinLim.x, pLayout.m_MinLim.y);
            coord(11, pLayout.m_MaxLim.x, pLayout.m_MaxLim.y);
            coord(12, 0, 0);
            coord(14, pLayout.m_MinExt.x, pLayout.m_MinExt.y);
            coord(15, pLayout.m_MaxExt.x, pLayout.m_MaxExt.y);
            dxfReal(146, 0);
            coord(13, 0, 0);
            coord(16, 1.0, 0);
            coord(17, 0, 1.0);
            dxfInt(76, 0);
            int blockEntryHandle;
            if (isPaperSpace)
            {
                blockEntryHandle = LookupBlockEntryHandle(pLayout.m_BlockName);
            }
            else
            {
                blockEntryHandle = LookupBlockEntryHandle("*Model_Space");
            }
            dxfHex(330, blockEntryHandle);
            dxfHex(331, 0);

            ++m_LayoutIndex;
        }

        internal static VectorOfEntityList s_PredefinedBlocks = new VectorOfEntityList();
        private static void Solid()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 0", s_PredefinedBlocks.Count == 0);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadSolid();
            ent0.m_Layer = "0";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Point1.SetPoint(-1, 1.0 / 6.0);
            ent0.m_Point2.SetPoint(0, 0);
            ent0.m_Point3.SetPoint(-1, -1.0 / 6.0);
            ent0.m_Point4.SetPoint(-1, 1.0 / 6.0);
            s_PredefinedBlocks[0].Add(ent0);
        }

        private static void ClosedBlank()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 1", s_PredefinedBlocks.Count == 1);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-1, 1.0 / 6.0);
            ent0.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[1].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(0, 0);
            ent1.m_EndPoint.SetPoint(-1, -1.0 / 6.0);
            s_PredefinedBlocks[1].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(-1, 1.0 / 6.0);
            ent2.m_EndPoint.SetPoint(-1, -1.0 / 6.0);
            s_PredefinedBlocks[1].Add(ent2);
        }

        private static void Closed()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 2", s_PredefinedBlocks.Count == 2);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-1, 1.0 / 6.0);
            ent0.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[2].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(0, 0);
            ent1.m_EndPoint.SetPoint(-1, -1.0 / 6.0);
            s_PredefinedBlocks[2].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(-1, 1.0 / 6.0);
            ent2.m_EndPoint.SetPoint(-1, -1.0 / 6.0);
            s_PredefinedBlocks[2].Add(ent2);

            var ent3 = new AcadLine();
            ent3.m_Layer = "0";
            ent3.m_Linetype = "ByBlock";
            ent3.m_Color = AcColor.acByBlock;
            ent3.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent3.m_StartPoint.SetPoint(0, 0);
            ent3.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[2].Add(ent3);
        }
        private static void Dot()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 3", s_PredefinedBlocks.Count == 3);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLWPLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Closed = true;
            ent0.SetConstWidth(0.5);
            ent0.m_Vertices.Add(new CDblPoint(-0.25, 0));
            ent0.SetBulge(0, 1.0);
            ent0.m_Vertices.Add(new CDblPoint(0.25, 0));
            ent0.SetBulge(1, 1.0);
            s_PredefinedBlocks[3].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-0.5, 0);
            ent1.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[3].Add(ent1);
        }

        private static void ArchTick()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 4", s_PredefinedBlocks.Count == 4);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLWPLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Closed = false;
            ent0.SetConstWidth(0.15);
            ent0.m_Vertices.Add(new CDblPoint(-0.5, -0.5));
            ent0.m_Vertices.Add(new CDblPoint(0.5, 0.5));
            s_PredefinedBlocks[4].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-1, 0);
            ent1.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[4].Add(ent1);
        }

        private static void Oblique()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 5", s_PredefinedBlocks.Count == 5);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-0.5, -0.5);
            ent0.m_EndPoint.SetPoint(0.5, 0.5);
            s_PredefinedBlocks[5].Add(ent0);
        }

        private static void Open()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 6", s_PredefinedBlocks.Count == 6);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-1, 1.0 / 6.0);
            ent0.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[6].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-1, -1.0 / 6.0);
            ent1.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[6].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(0, 0);
            ent2.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[6].Add(ent2);
        }
        private static void Origin()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 7", s_PredefinedBlocks.Count == 7);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadCircle();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_Center.SetPoint(0, 0);
            ent0.m_Radius = 0.5;
            s_PredefinedBlocks[7].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(0, 0);
            ent1.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[7].Add(ent1);
        }

        private static void Origin2()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 8", s_PredefinedBlocks.Count == 8);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadCircle();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_Center.SetPoint(0, 0);
            ent0.m_Radius = 0.5;
            s_PredefinedBlocks[8].Add(ent0);

            var copy = new AcadCircle(ent0);
            copy.m_Radius = 0.25;
            s_PredefinedBlocks[8].Add(copy);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-0.5, 0);
            ent1.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[8].Add(ent1);
        }

        private static void Open90()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 9", s_PredefinedBlocks.Count == 9);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-0.5, 0.5);
            ent0.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[9].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(0, 0);
            ent1.m_EndPoint.SetPoint(-0.5, -0.5);
            s_PredefinedBlocks[9].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(0, 0);
            ent2.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[9].Add(ent2);
        }

        private static void Open30()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 10", s_PredefinedBlocks.Count == 10);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-1, 0.26794919);
            ent0.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[10].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-1, -0.26794919);
            ent1.m_EndPoint.SetPoint(0, 0);
            s_PredefinedBlocks[10].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(0, 0);
            ent2.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[10].Add(ent2);
        }
        private static void DotSmall()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 11", s_PredefinedBlocks.Count == 11);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLWPLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Closed = true;
            ent0.SetConstWidth(0.5);
            ent0.m_Vertices.Add(new CDblPoint(-0.0625, 0));
            ent0.SetBulge(0, 1.0);
            ent0.m_Vertices.Add(new CDblPoint(0.0625, 0));
            ent0.SetBulge(1, 1.0);
            s_PredefinedBlocks[11].Add(ent0);
        }

        private static void DotBlank()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 12", s_PredefinedBlocks.Count == 12);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadCircle();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_Center.SetPoint(0, 0);
            ent0.m_Radius = 0.5;
            s_PredefinedBlocks[12].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-0.5, 0);
            ent1.m_EndPoint.SetPoint(-1.0, 0);
            s_PredefinedBlocks[12].Add(ent1);
        }

        private static void Small()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 13", s_PredefinedBlocks.Count == 13);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadCircle();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_Center.SetPoint(0, 0);
            ent0.m_Radius = 0.25;
            s_PredefinedBlocks[13].Add(ent0);
        }

        private static void BoxBlank()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 14", s_PredefinedBlocks.Count == 14);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(-0.5, -0.5);
            ent0.m_EndPoint.SetPoint(0.5, -0.5);
            s_PredefinedBlocks[14].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(0.5, -0.5);
            ent1.m_EndPoint.SetPoint(0.5, 0.5);
            s_PredefinedBlocks[14].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(0.5, 0.5);
            ent2.m_EndPoint.SetPoint(-0.5, 0.5);
            s_PredefinedBlocks[14].Add(ent2);

            var ent3 = new AcadLine();
            ent3.m_Layer = "0";
            ent3.m_Linetype = "ByBlock";
            ent3.m_Color = AcColor.acByBlock;
            ent3.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent3.m_StartPoint.SetPoint(-0.5, 0.5);
            ent3.m_EndPoint.SetPoint(-0.5, -0.5);
            s_PredefinedBlocks[14].Add(ent3);

            var ent4 = new AcadLine();
            ent4.m_Layer = "0";
            ent4.m_Linetype = "ByBlock";
            ent4.m_Color = AcColor.acByBlock;
            ent4.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent4.m_StartPoint.SetPoint(-0.5, 0);
            ent4.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[14].Add(ent4);
        }
        private static void BoxFilled()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 15", s_PredefinedBlocks.Count == 15);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadSolid();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Point1.SetPoint(-0.5, 0.5);
            ent0.m_Point2.SetPoint(0.5, 0.5);
            ent0.m_Point3.SetPoint(-0.5, -0.5);
            ent0.m_Point4.SetPoint(0.5, -0.5);
            s_PredefinedBlocks[15].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-0.5, 0);
            ent1.m_EndPoint.SetPoint(-1, 0);
            s_PredefinedBlocks[15].Add(ent1);
        }

        private static void DatumBlank()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 16", s_PredefinedBlocks.Count == 16);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadLine();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_StartPoint.SetPoint(0, 0.5773502700000001);
            ent0.m_EndPoint.SetPoint(-1.0, 0);
            s_PredefinedBlocks[16].Add(ent0);

            var ent1 = new AcadLine();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_StartPoint.SetPoint(-1.0, 0);
            ent1.m_EndPoint.SetPoint(0, -0.5773502700000001);
            s_PredefinedBlocks[16].Add(ent1);

            var ent2 = new AcadLine();
            ent2.m_Layer = "0";
            ent2.m_Linetype = "ByBlock";
            ent2.m_Color = AcColor.acByBlock;
            ent2.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent2.m_StartPoint.SetPoint(0, 0.5773502700000001);
            ent2.m_EndPoint.SetPoint(0, -0.5773502700000001);
            s_PredefinedBlocks[16].Add(ent2);
        }

        private static void DatumFilled()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 17", s_PredefinedBlocks.Count == 17);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadSolid();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Point1.SetPoint(0, 0.5773502700000001);
            ent0.m_Point2.SetPoint(-1.0, 0);
            ent0.m_Point3.SetPoint(0, -0.5773502700000001);
            ent0.m_Point4.SetPoint(0, -0.5773502700000001);
            s_PredefinedBlocks[17].Add(ent0);
        }

        private static void Integral()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 18", s_PredefinedBlocks.Count == 18);
            s_PredefinedBlocks.Add(new EntityList());
            var ent0 = new AcadArc();
            ent0.m_Layer = "0";
            ent0.m_Linetype = "ByBlock";
            ent0.m_Color = AcColor.acByBlock;
            ent0.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent0.m_Center.SetPoint(0.44488802, -0.09133463);
            ent0.m_Radius = 0.4541666700000002;
            ent0.m_StartAngle = 101.9999999980395;
            ent0.m_EndAngle = 167.9999999799193;
            s_PredefinedBlocks[18].Add(ent0);

            var ent1 = new AcadArc();
            ent1.m_Layer = "0";
            ent1.m_Linetype = "ByBlock";
            ent1.m_Color = AcColor.acByBlock;
            ent1.m_Lineweight = AcLineWeight.acLnWtByBlock;
            ent1.m_Center.SetPoint(-0.44488802, 0.09133463);
            ent1.m_Radius = 0.4541666700000002;
            ent1.m_StartAngle = 282.0000000215427;
            ent1.m_EndAngle = 348.0000000034225;
            s_PredefinedBlocks[18].Add(ent1);
        }

        private static void None()
        {
            Global.ASSERT_DEBUG_INFO("s_PredefinedBlocks.Count == 19", s_PredefinedBlocks.Count == 19);
            s_PredefinedBlocks.Add(new EntityList());
        }
        private static void InitPredefinedBlocks()
        {
            //static std::mutex g_mutex;
            //std::lock_guard<std::mutex> lock(g_mutex);
            if (s_PredefinedBlocks.Count != 0)
            {
                return;
            }
            Solid();
            ClosedBlank();
            Closed();
            Dot();
            ArchTick();
            Oblique();
            Open();
            Origin();
            Origin2();
            Open90();
            Open30();
            DotSmall();
            DotBlank();
            Small();
            BoxBlank();
            BoxFilled();
            DatumBlank();
            DatumFilled();
            Integral();
            None();
        }

        public int m_CurrentBlockEntryHandle;
        public int m_CurrentViewportID;


        private StreamWriter m_DxfFile;
        private DxfData m_Graph;
        private LinetypeManager m_LinetypeManager = new LinetypeManager();
        private PatternManager m_PatternManager = new PatternManager();
        private int m_Handle;
        private int m_LayoutIndex;
        private SortedDictionary<string, int> m_TextStyleHandleTable = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_DimStyleHandleTable = new SortedDictionary<string, int>();
        // block name . block entry handle
        private SortedDictionary<string, int> m_BlockEntryHandleTable = new SortedDictionary<string, int>();
        // layout block name . layout handle
        private SortedDictionary<string, int> m_LayoutHandleTable = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_MLeaderStyleHandleTable = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_TableStyleHandleTable = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_LinetypeHandleTable = new SortedDictionary<string, int>();
        private SortedDictionary<string, List<EntAttribute>> m_MLeaderStyleEntities = new SortedDictionary<string, List<EntAttribute>>();
        private SortedDictionary<string, List<EntAttribute>> m_TableStyleEntities = new SortedDictionary<string, List<EntAttribute>>();
    }
}
