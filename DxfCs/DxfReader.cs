using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace DXF
{
    public class DxfReader
    {
        private const string g_PaperSpace = "*Paper_Space";
        private const int BUFSIZE = 256;

        public DxfReader(DxfData graph)
        {
            this.m_Graph = graph;
        }

        public bool ReadDxf(string dxfFileName, bool OnlyReadModelSpace)
        {
            bool bSuccess = true;
#if DEBUG
            using (var log = new MethodLogger("(dxfFileName = {0}, OnlyReadModelSpace = {1})", dxfFileName, OnlyReadModelSpace ? "true" : "false"))
            {
#endif
                m_Graph.Reset();
                Reset();

                m_OnlyReadModelSpace = OnlyReadModelSpace;
                try
                {
                    m_DxfFile = new ifstream(new FileStream(dxfFileName, FileMode.Open, FileAccess.Read));
                    bSuccess = m_DxfFile != null;
                    bSuccess = bSuccess && ReadHeader();

                    bSuccess = bSuccess && ReadTables();
                    bSuccess = bSuccess && ReadBlocks();
                    bSuccess = bSuccess && ReadEntities();
                    bSuccess = bSuccess && ReadObjects();
                    if (m_DxfFile != null)
                    {
                        m_DxfFile.Close();
                    }
                    ConvertHandle();
                }
                catch(Exception e)
                {
                    bSuccess = false;
                    Global.PrintDebugInfo(e.StackTrace);
                }
                finally
                {
                    if (m_DxfFile != null)
                        m_DxfFile.Close();
                }
                
                if (!bSuccess)
                {
                    m_Graph.Reset();
                }
#if DEBUG
            }
#endif
            return bSuccess;
        }

        private void ReadPair(out int groupCode, out string value, int bufsize = BUFSIZE)
        {
            // Don't use TRACE_FUNCTION, it is a high frequent function.

            m_DxfFile.read(out groupCode);
            m_DxfFile.ignore(Int32.MaxValue, '\n');
            m_DxfFile.getline(out value, bufsize);
            value = Global.TrimSpace(value);
        }

        private bool AdvanceToSection(string sectionName)
        {
            string buffer = null;
            int num = 0;
            int phase = 0;
            while (phase < 2 && !m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (0 == phase)
                {
                    if (num != 0 || (string.Compare("SECTION", buffer, true) != 0))
                    {
                        continue;
                    }
                }
                else if (1 == phase)
                {
                    if (num != 2 || (string.Compare(buffer, sectionName, true) != 0))
                    {
                        phase = 0;
                        continue;
                    }
                }
                ++phase;
            }
            return 2 == phase;
        }
        private bool AdvanceToObject(string objectType)
        {
            string buffer = null;
            int num = 0;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0 && (string.Compare(objectType, buffer, true) == 0))
                {
                    m_DxfFile.seekg(pos);
                    return true;
                }
            }
            return false;
        }

        private bool AdvanceToObjectStopAtLayout(string objectType)
        {
            string buffer = null;
            int num = 0;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    if (string.Compare(objectType, buffer, true) == 0)
                    {
                        m_DxfFile.seekg(pos);
                        return true;
                    }
                    else if (string.Compare(objectType, "LAYOUT", true) == 0)
                    {
                        m_DxfFile.seekg(pos);
                        return true;
                    }
                }
            }
            return false;
        }
        private bool NextVariable(out string varName, out string varVal)
        {
            int num = 0;
            ReadPair(out num, out varName);
            if (num != 9)
            {
                varVal = null;
                return false;
            }
            ReadPair(out num, out varVal);
            return true;
        }
        private bool ReadHeader()
        {
            bool bSuccess = AdvanceToSection("HEADER");
            if (!bSuccess)
            {
                return bSuccess;
            }
            string varName = null;
            string varVal = null;
            while (NextVariable(out varName, out varVal) && !m_DxfFile.eof())
            {
                if (string.Compare(varName, "$LTSCALE", true) == 0)
                {
                    m_Graph.m_LinetypeScale = Convert.ToDouble(varVal);
                }
            }
            return bSuccess;
        }
        private bool NextTable(out string tableName)
        {
            string buffer = null;
            int num = 0;
            int phase = 0;
            tableName = null;
            while (phase < 2 && !m_DxfFile.eof())
            {
                if (0 == phase)
                {
                    ReadPair(out num, out buffer);
                    if (num == 0)
                    {
                        if (string.Compare("ENDSEC", buffer, true) == 0)
                        {
                            return false; //遇到段结尾退出，防止不停地找下去
                        }
                        else if (string.Compare("TABLE", buffer, true) != 0)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (1 == phase)
                {
                    ReadPair(out num, out tableName);
                    if (num != 2)
                    {
                        phase = 0;
                        continue;
                    }
                }
                ++phase;
            }
            return 2 == phase;
        }
        private bool NextTableEntry(string entryName)
        {
            string buffer = null;
            int num = 0;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    if (string.Compare("ENDTAB", buffer, true) == 0)
                    {
                        return false; //遇到表结尾退出，防止不停地找下去
                    }
                    else if (string.Compare(entryName, buffer, true) == 0)
                    {
                        return true;
                    }
                }
            }
            return false;
        }
        private void ReadTableEntry(out string name, object data)
        {
            int groupCode = 0;
            string buffer = null;
            name = null;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out groupCode, out buffer);
                if (groupCode != 0)
                {
                    HandleTableEntryCode(this, ref name, data, groupCode, buffer);
                }
                else
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
            }
        }
        private void HandleBlockRecordEntryCode(ref string name, object data, int groupCode, string value)
        {
            IntBoxing handle = (IntBoxing)data;
            switch (groupCode)
            {
                case 2:
                    name = value;
                    break;
                case 5:
                    handle.value = Global.hextoi(value);
                    break;
                default:
                    break;
            }
        }
        private void HandleLayerEntryCode(ref string name, object data, int groupCode, string value)
        {
            LayerData ld = (LayerData)data;
            switch (groupCode)
            {
                case 2:
                    name = value;
                    break;
                case 62:
                    ld.m_Color = (AcColor)Convert.ToInt32(value);
                    break;
                case 6:
                    ld.m_Linetype = value;
                    break;
                case 290:
                    ld.m_Plottable = Convert.ToInt32(value) != 0;
                    break;
                case 370:
                    ld.m_LineWeight = (AcLineWeight)Convert.ToInt32(value);
                    break;
                default:
                    break;
            }
        }

        class IntBoxing
        {
            public int value;
        }
        private void HandleLTypeEntryCode(ref string name, object data, int groupCode, string value)
        {
            IntBoxing handle = data as IntBoxing;
            switch (groupCode)
            {
                case 2:
                    name = value;
                    break;
                case 5:
                    handle.value = Global.hextoi(value);
                    break;
                default:
                    break;
            }
        }

        private class TextStyleDataExt
        {
            internal TextStyleData tsd = new TextStyleData();
            internal int handle;
        }

        private void HandleStyleEntryCode(ref string name, object data, int groupCode, string value)
        {
            TextStyleDataExt tsd = (TextStyleDataExt)data;
            switch (groupCode)
            {
                case 2:
                    name = value;
                    break;
                case 3:
                    tsd.tsd.m_PrimaryFontFile = value;
                    break;
                case 1001:
                    if (string.Compare("ACAD", value) == 0)
                    {
                        tsd.tsd.m_TrueType = "true";
                    }
                    break;
                case 1000:
                    if (tsd.tsd.m_TrueType == "true")
                    {
                        tsd.tsd.m_TrueType = value;
                    }
                    break;
                case 4:
                    tsd.tsd.m_BigFontFile = value;
                    break;
                case 5:
                    tsd.handle = Global.hextoi(value);
                    break;
                case 40:
                    tsd.tsd.m_Height = Convert.ToDouble(value);
                    break;
                case 41:
                    tsd.tsd.m_WidthFactor = Convert.ToDouble(value);
                    break;
                case 50:
                    tsd.tsd.m_ObliqueAngle = Convert.ToDouble(value);
                    break;
                default:
                    break;
            }
        }
        private void HandleDimStyleEntryCode(ref string name, object data, int groupCode, string value)
        {
            DimStyleData dsd = (DimStyleData)data;
            switch (groupCode)
            {
                case 2:
                    name = value;
                    break;
                case 3:
                    //case 4:
                    dsd.Text = value;
                    break;
                case 41:
                    dsd.ArrowHeadSize = Convert.ToDouble(value);
                    break;
                case 42:
                    dsd.ExtensionLineOffset = Convert.ToDouble(value);
                    break;
                case 44:
                    dsd.ExtensionLineExtend = Convert.ToDouble(value);
                    break;
                case 140:
                    dsd.TextHeight = Convert.ToDouble(value);
                    break;
                case 141:
                    dsd.CenterMarkSize = Convert.ToDouble(value);
                    {
                        if (dsd.CenterMarkSize > 0)
                        {
                            dsd.CenterType = AcDimCenterType.acCenterMark;
                        }
                        else if (dsd.CenterMarkSize == 0)
                        {
                            dsd.CenterType = AcDimCenterType.acCenterNone;
                        }
                        else
                        {
                            dsd.CenterType = AcDimCenterType.acCenterLine;
                            dsd.CenterMarkSize = -dsd.CenterMarkSize;
                        }
                    }
                    break;
                case 144:
                    dsd.LinearScaleFactor = Convert.ToDouble(value);
                    break;
                case 77:
                    {
                        int temp = Convert.ToInt32(value);
                        if (1 == temp)
                        {
                            dsd.VerticalTextPosition = AcDimVerticalJustification.acAbove;
                        }
                    }
                    break;
                case 145:
                    dsd.VerticalTextPosition = (AcDimVerticalJustification)Convert.ToInt32(value);
                    break;
                case 147:
                    dsd.TextGap = Convert.ToDouble(value);
                    break;
                case 73:
                    //case 74:
                    dsd.TextAlign = Convert.ToInt32(value) == 1;
                    break;
                case 75:
                    dsd.ExtLine1Suppress = Convert.ToInt32(value) == 1;
                    break;
                case 76:
                    dsd.ExtLine2Suppress = Convert.ToInt32(value) == 1;
                    break;
                case 78:
                    {
                        int flag = Convert.ToInt32(value);
                        dsd.SuppressLeadingZeros = (flag & 4) != 0;
                        dsd.SuppressTrailingZeros = (flag & 8) != 0;
                    }
                    break;
                case 79:
                    {
                        int flag = Convert.ToInt32(value);
                        dsd.AngleSuppressLeadingZeros = (flag & 1) != 0;
                        dsd.AngleSuppressTrailingZeros = (flag & 2) != 0;
                    }
                    break;
                case 171:
                    dsd.UnitsPrecision = (AcDimPrecision)Convert.ToInt32(value);
                    break;
                case 172:
                    dsd.ForceLineInside = Convert.ToInt32(value) == 1;
                    break;
                case 174:
                    dsd.TextInside = Convert.ToInt32(value) == 1;
                    break;
                case 176:
                    dsd.DimensionLineColor = (AcColor)Convert.ToInt32(value);
                    break;
                case 177:
                    dsd.ExtensionLineColor = (AcColor)Convert.ToInt32(value);
                    break;
                case 178:
                    dsd.TextColor = (AcColor)Convert.ToInt32(value);
                    break;
                case 179:
                    dsd.AnglePrecision = (AcDimPrecision)Convert.ToInt32(value);
                    break;
                case 271:
                    dsd.UnitsPrecision = (AcDimPrecision)Convert.ToInt32(value);
                    break;
                //case 273:
                case 277:
                    dsd.UnitsFormat = (AcDimLUnits)Convert.ToInt32(value);
                    break;
                case 275:
                    dsd.AngleFormat = (AcAngleUnits)Convert.ToInt32(value);
                    break;
                case 279:
                    dsd.TextMovement = (AcDimTextMovement)Convert.ToInt32(value);
                    break;
                case 280:
                    dsd.HorizontalTextPosition = (AcDimHorizontalJustification)Convert.ToInt32(value);
                    break;
                case 281:
                    dsd.DimLine1Suppress = Convert.ToInt32(value) == 1;
                    break;
                case 282:
                    dsd.DimLine2Suppress = Convert.ToInt32(value) == 1;
                    break;
                case 289:
                    dsd.Fit = (AcDimFit)Convert.ToInt32(value);
                    break;
                case 340:
                    // 此时调用LookupTextStyleName(handle)一般不会成功。
                    dsd.TextStyle = value;
                    break;
                case 342:
                    // 此时调用LookupBlockEntryName(handle)一般不会成功。
                    dsd.ArrowHead1Block = value;
                    dsd.ArrowHead2Block = value;
                    break;
                case 343:
                    dsd.ArrowHead1Block = value;
                    break;
                case 344:
                    dsd.ArrowHead2Block = value;
                    break;
                case 371:
                    dsd.DimensionLineWeight = (AcLineWeight)Convert.ToInt32(value);
                    break;
                case 372:
                    dsd.ExtensionLineWeight = (AcLineWeight)Convert.ToInt32(value);
                    break;
                default:
                    break;
            }
        }

        private delegate void HandleTableEntryCodeDelegate(DxfReader reader, ref string name, object data, int groupCode, string value);
        private HandleTableEntryCodeDelegate HandleTableEntryCode;
        private bool ReadTables()
        {
            //TRACE_FUNCTION("()");

            bool bSuccess = AdvanceToSection("TABLES");
            if (!bSuccess)
            {
                return bSuccess;
            }
            string tableName;
            while (NextTable(out tableName))
            {
                Global.PrintDebugInfo("Read Table {0}", tableName);
                while (NextTableEntry(tableName))
                {
                    object data = null;
                    if (string.Compare(tableName, "BLOCK_RECORD", true) == 0)
                    {
                        HandleTableEntryCode = (DxfReader reader, ref string name, object data1, int groupCode, string value) =>
                        { reader.HandleBlockRecordEntryCode(ref name, data1, groupCode, value); };
                        data = new IntBoxing();
                    }
                    else if (string.Compare(tableName, "LAYER", true) == 0)
                    {
                        HandleTableEntryCode = (DxfReader reader, ref string name, object data1, int groupCode, string value) =>
                        { reader.HandleLayerEntryCode(ref name, data1, groupCode, value); };
                        LayerData ld = new LayerData();
                        ld.m_Plottable = true;
                        ld.m_LineWeight = AcLineWeight.acLnWtByLayer;
                        data = ld;
                    }
                    else if (string.Compare(tableName, "LTYPE", true) == 0)
                    {
                        HandleTableEntryCode = (DxfReader reader, ref string name, object data1, int groupCode, string value) =>
                        { reader.HandleLTypeEntryCode(ref name, data1, groupCode, value); };
                        data = new IntBoxing();
                    }
                    else if (string.Compare(tableName, "STYLE", true) == 0)
                    {
                        HandleTableEntryCode = (DxfReader reader, ref string name, object data1, int groupCode, string value) =>
                        { reader.HandleStyleEntryCode(ref name, data1, groupCode, value); };
                        TextStyleDataExt tsd = new TextStyleDataExt();
                        tsd.tsd.m_WidthFactor = 1.0;
                        tsd.tsd.m_ObliqueAngle = 0.0;
                        data = tsd;
                    }
                    else if (string.Compare(tableName, "DIMSTYLE", true) == 0)
                    {
                        HandleTableEntryCode = (DxfReader reader, ref string name, object data1, int groupCode, string value) =>
                        { reader.HandleDimStyleEntryCode(ref name, data1, groupCode, value); };
                        data = new DimStyleData();
                    }
                    else
                    {
                        HandleTableEntryCode = null;
                    }

                    if (HandleTableEntryCode != null)
                    {
                        string name = null;
                        ReadTableEntry(out name, data);
                        Global.PrintDebugInfo("Read Entry {0}", name);
                        if (string.Compare(tableName, "BLOCK_RECORD", true) == 0)
                        {
                            IntBoxing handle = (IntBoxing)data;
                            if (!string.IsNullOrEmpty(name))
                            {
                                m_BlockEntryHandleTable[handle.value] = name;
                            }
                            data = null;
                        }
                        else if (string.Compare(tableName, "LAYER", true) == 0)
                        {
                            LayerData ld = (LayerData)data;
                            if (!string.IsNullOrEmpty(name))
                            {
                                m_Graph.m_Layers[name] = ld;
                            }
                            data = null;
                        }
                        else if (string.Compare(tableName, "LTYPE", true) == 0)
                        {
                            // The 3 linetypes are not exist in acad.lin or acadiso.lin file
                            if (name != "ByBlock" && name != "ByLayer" && name != "Continuous" && !string.IsNullOrEmpty(name))
                            {
                                m_Graph.m_Linetypes.Add(name);
                            }
                            IntBoxing handle = (IntBoxing)data;
                            if (!string.IsNullOrEmpty(name))
                            {
                                m_LinetypeHandleTable[handle.value] = name;
                            }
                            data = null;
                        }
                        else if (string.Compare(tableName, "STYLE", true) == 0)
                        {
                            TextStyleDataExt tsd = (TextStyleDataExt)data;
                            if (!string.IsNullOrEmpty(name))
                            {
                                m_Graph.m_TextStyles[name] = tsd.tsd;
                                m_TextStyleHandleTable[tsd.handle] = name;
                            }
                            tsd = null;
                        }
                        else if (string.Compare(tableName, "DIMSTYLE", true) == 0)
                        {
                            DimStyleData dsd = (DimStyleData)data;
                            if (!string.IsNullOrEmpty(name))
                            {
                                m_Graph.m_DimStyles[name] = dsd;
                                m_ToBeConvertedTextStyleName.Add(name, Global.hextoi(dsd.TextStyle));
                                if (!string.IsNullOrEmpty(dsd.ArrowHead1Block))
                                {
                                    m_ToBeConvertedArrow1Name.Add(name, Global.hextoi(dsd.ArrowHead1Block));
                                }
                                if (!string.IsNullOrEmpty(dsd.ArrowHead2Block))
                                {
                                    m_ToBeConvertedArrow2Name.Add(name, Global.hextoi(dsd.ArrowHead2Block));
                                }
                            }
                            dsd = null;
                        }
                    }

                }
            }
            return bSuccess;
        }
        private bool NextBlock()
        {
            int num = 0;
            string buffer = null;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 0 && string.Compare(buffer, "BLOCK", true) == 0)
                {
                    return true;
                }
                if (num == 0 && string.Compare(buffer, "ENDSEC", true) == 0)
                {
                    return false;
                }
            }
            return false;
        }
        private bool NextObject(string objectType)
        {
            int num = 0;
            string buffer = null;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    if (string.Compare(buffer, objectType, true) == 0)
                    {
                        ReadPair(out num, out buffer);
                        if (num != 5)
                        {
                            Global.THROW_INVALID_DXF();
                            return false;
                        }
                        m_CurObjectHandle = Global.hextoi(buffer);
                        return true;
                    }
                    else
                    {
                        m_DxfFile.seekg(pos);
                        return false;
                    }
                }
            }
            return false;
        }
        private void ReadDictionaryEntries(Func<NameHandle, bool> HandleDicEntry)
        {
            int num = 0;
            string buffer = null;
            NameHandle curDicEntry = new NameHandle();
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    return;
                }
                switch (num)
                {
                    case 3:
                        curDicEntry.name = buffer;
                        break;
                    case 350:
                        curDicEntry.handle = Global.hextoi(buffer);
                        if (HandleDicEntry(curDicEntry))
                        {
                            return;
                        }
                        break;
                }
            }
        }
        private LayoutData ReadBlockBegin(out string blockName, BlockDef block)
        {
            blockName = null;
            int groupCode = 0;
            string buffer = null;
            LayoutData pLayout = null;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out groupCode, out buffer);
                if (groupCode != 0)
                {
                    switch (groupCode)
                    {
                        case 2:
                            blockName = buffer;
                            if (buffer[0] == '*')
                            {
                                // Paperspace blocks
                                if (buffer.StartsWith(g_PaperSpace, StringComparison.InvariantCultureIgnoreCase))
                                {
                                    pLayout = new LayoutData();
                                }
                                // Temporary blocks
                            }
                            break;
                        case 10:
                            block.m_InsertPoint.x = Convert.ToDouble(buffer);
                            break;
                        case 20:
                            block.m_InsertPoint.y = Convert.ToDouble(buffer);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
            }
            return pLayout;
        }
        private bool NextEntity(out string entityType)
        {
            entityType = null;
            int num = 0;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out entityType);
                if (num == 0)
                {
                    if (string.Compare(entityType, "ENDSEC", true) == 0 || string.Compare(entityType, "ENDBLK", true) == 0)
                    {
                        return false;
                    }
                    else
                    {
                        return true;
                    }

                }
            }
            return false;
        }
        private void ReadAllEntities(EntityList pObjList, bool bIn_ENTITIES_Section)
        {
            EntAttribute att = new EntAttribute();
            string buffer = null;
            EntityList pContainerList = pObjList;
            EntityList pModelSpaceList = null;
            EntityList pPaperSpaceList = null;
            LayoutData pDefaultPaperSpace = null;
            if (bIn_ENTITIES_Section)
            {
                pDefaultPaperSpace = LookupLayout(g_PaperSpace);
                if (pDefaultPaperSpace != null)
                {
                    pPaperSpaceList = pDefaultPaperSpace.m_Objects;
                }
                pModelSpaceList = m_Graph.m_Objects;
            }
            while (NextEntity(out buffer))
            {
                ReadAttribute();
                if (bIn_ENTITIES_Section)
                {
                    if (m_Attribute.m_IsInPaperspace)
                    {
                        if (pDefaultPaperSpace == null)
                        {
                            continue;
                        }
                        pContainerList = pPaperSpaceList;
                    }
                    else
                    {
                        pContainerList = pModelSpaceList;
                    }
                }

                Global.PrintDebugInfo("Read {0}{1}", buffer, m_Attribute.m_IsInPaperspace ? "(PS)" : "");

                //按出现概率大小排序
                if (string.Compare(buffer, "LINE", true) == 0)
                {
                    ReadLine(pContainerList);
                }
                else if (string.Compare(buffer, "TEXT", true) == 0)
                {
                    ReadText(pContainerList);
                }
                else if (string.Compare(buffer, "LWPOLYLINE", true) == 0)
                {
                    ReadLwpolyline(pContainerList);
                }
                else if (string.Compare(buffer, "ARC", true) == 0)
                {
                    ReadArc(pContainerList);
                }
                else if (string.Compare(buffer, "DIMENSION", true) == 0)
                {
                    ReadDimension(pContainerList);
                }
                else if (string.Compare(buffer, "INSERT", true) == 0)
                {
                    ReadInsert(pContainerList);
                }
                else if (string.Compare(buffer, "CIRCLE", true) == 0)
                {
                    ReadCircle(pContainerList);
                }
                else if (string.Compare(buffer, "ELLIPSE", true) == 0)
                {
                    ReadEllipse(pContainerList);
                }
                else if (string.Compare(buffer, "HATCH", true) == 0)
                {
                    ReadHatch(pContainerList);
                }
                else if (string.Compare(buffer, "LEADER", true) == 0)
                {
                    ReadLeader(pContainerList);
                }
                else if (string.Compare(buffer, "MULTILEADER", true) == 0)
                {
                    ReadMLeader(pContainerList);
                }
                else if (string.Compare(buffer, "MTEXT", true) == 0)
                {
                    ReadMtext(pContainerList);
                }
                else if (string.Compare(buffer, "SPLINE", true) == 0)
                {
                    ReadSpline(pContainerList);
                }
                else if (string.Compare(buffer, "VIEWPORT", true) == 0)
                {
                    ReadViewport(pContainerList);
                }
                else if (string.Compare(buffer, "SOLID", true) == 0)
                {
                    ReadSolid(pContainerList);
                }
                else if (string.Compare(buffer, "POINT", true) == 0)
                {
                    ReadPoint(pContainerList);
                }
                else if (string.Compare(buffer, "ACAD_TABLE", true) == 0)
                {
                    ReadTable(pContainerList);
                }
                else if (string.Compare(buffer, "ATTDEF", true) == 0)
                {
                    ReadAttDef(pContainerList);
                }
                else if (string.Compare(buffer, "ATTRIB", true) == 0)
                {
                    ReadAttrib(pContainerList);
                }
            }
        }
        private bool ReadBlocks()
        {
            //TRACE_FUNCTION("()");

            bool bSuccess = AdvanceToSection("BLOCKS");
            string blockName = null;
            while (NextBlock())
            {
                var pBlockDef = new BlockDef();
                LayoutData pLayout = ReadBlockBegin(out blockName, pBlockDef);
                Global.PrintDebugInfo("Read Block {0}", blockName);
                if (pLayout != null)
                {
                    if (!m_OnlyReadModelSpace)
                    {
                        pLayout.m_BlockName = blockName;
                        // The key should be layout name, but layout name is not available at this time.
                        // So use unqiue block name temporarily.
                        m_Graph.m_Layouts[blockName] = pLayout;
                        ReadAllEntities(pLayout.m_Objects, false);
                    }
                }
                else
                {
                    if (string.IsNullOrEmpty(blockName) || blockName == "*Model_Space")
                    {
                        continue;
                    }
                    // Block name poutix meanings:
                    // '_' - predefined arrow head block
                    // '*Paper_Space' - Paperspace blocks
                    // '*' - temporary blocks
                    m_Graph.m_RealBlockDefs[blockName] = pBlockDef;
                    ReadAllEntities(pBlockDef.m_Objects, false);
                }
            }
            return bSuccess;
        }
        private bool ReadEntities()
        {
            //TRACE_FUNCTION("()");

            bool bSuccess = AdvanceToSection("ENTITIES");
            ReadAllEntities(null, true);
            return bSuccess;
        }
        private void ReadLayout(out string layoutName, LayoutData lo)
        {
            layoutName = null;
            //bool IsThis330 = false;
            long pos;
            int num = 0;
            string buffer = new string(new char[BUFSIZE]);
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 100 && string.Compare(buffer, "AcDbLayout", true) == 0)
                {
                    break;
                }
                switch (num)
                {
                    case 2:
                        lo.m_PlotSettings.m_PlotConfigFile = buffer;
                        break;
                    case 4:
                        lo.m_PlotSettings.m_PaperName = buffer;
                        break;
                    case 40:
                        lo.m_PlotSettings.m_LeftMargin = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        lo.m_PlotSettings.m_BottomMargin = Convert.ToDouble(buffer);
                        break;
                    case 42:
                        lo.m_PlotSettings.m_RightMargin = Convert.ToDouble(buffer);
                        break;
                    case 43:
                        lo.m_PlotSettings.m_TopMargin = Convert.ToDouble(buffer);
                        break;
                    case 44:
                        lo.m_PlotSettings.m_Width = Convert.ToDouble(buffer);
                        break;
                    case 45:
                        lo.m_PlotSettings.m_Height = Convert.ToDouble(buffer);
                        break;
                    case 46:
                        lo.m_PlotSettings.m_PlotOrigin.x = Convert.ToDouble(buffer);
                        break;
                    case 47:
                        lo.m_PlotSettings.m_PlotOrigin.y = Convert.ToDouble(buffer);
                        break;
                    case 73:
                        lo.m_PlotSettings.m_PlotRotation = Convert.ToInt32(buffer);
                        break;
                    default:
                        break;
                }
            }
            while (!m_DxfFile.eof())
            {
                pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 1:
                        layoutName = buffer;
                        break;
                    case 71:
                        lo.m_LayoutOrder = Convert.ToInt32(buffer);
                        break;
                    case 10:
                        lo.m_MinLim.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        lo.m_MinLim.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        lo.m_MaxLim.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        lo.m_MaxLim.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        lo.m_MinExt.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        lo.m_MinExt.y = Convert.ToDouble(buffer);
                        break;
                    case 15:
                        lo.m_MaxExt.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        lo.m_MaxExt.y = Convert.ToDouble(buffer);
                        break;
                    case 330:
                        {
                            int handle = Global.hextoi(buffer);
                            lo.m_BlockName = LookupBlockEntryName(handle);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadMLeaderStyle(MLeaderStyle mls)
        {
            int num = 0;
            string buffer = new string(new char[BUFSIZE]);
            long pos = m_DxfFile.tellg();
            ContentType ct = ContentType.MTEXT_TYPE;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 170:
                        if ((ct = (ContentType)(Convert.ToInt32(buffer))) == ContentType.BLOCK_TYPE)
                        {
                            mls.m_Content = new StyleBlockPart();
                        }
                        else
                        {
                            mls.m_Content = new StyleMTextPart();
                        }
                        break;
                    case 90:
                        mls.m_MaxLeaderPoints = Convert.ToInt32(buffer);
                        break;
                    case 40:
                        mls.m_FirstSegAngleConstraint = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        mls.m_SecondSegAngleConstraint = Convert.ToDouble(buffer);
                        break;
                    case 173:
                        mls.m_LeaderType = (AcMLeaderType)(Convert.ToInt32(buffer));
                        break;
                    case 91:
                        mls.m_LineColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 340:
                        mls.m_LineType = LookupLinetypeName(Global.hextoi(buffer));
                        break;
                    case 92:
                        mls.m_LineWeight = (AcLineWeight)(Convert.ToInt32(buffer));
                        break;
                    case 290:
                        mls.m_EnableLanding = Convert.ToInt32(buffer) != 0;
                        break;
                    case 42:
                        mls.m_LandingGap = Convert.ToDouble(buffer);
                        break;
                    case 291:
                        mls.m_EnableDogleg = Convert.ToInt32(buffer) != 0;
                        break;
                    case 43:
                        mls.m_DoglegLength = Convert.ToDouble(buffer);
                        break;
                    case 44:
                        mls.m_ArrowSize = Convert.ToDouble(buffer);
                        break;
                    case 296:
                        mls.m_IsAnnotative = Convert.ToInt32(buffer) != 0;
                        break;
                    case 143:
                        mls.m_BreakGapSize = Convert.ToDouble(buffer);
                        break;
                    case 341:
                        mls.m_ArrowHead = LookupBlockEntryName(Global.hextoi(buffer));
                        break;
                    default:
                        break;
                }
            }

            if (ct == ContentType.BLOCK_TYPE)
            {
                ReadStyleBlockPart((StyleBlockPart)mls.m_Content);
            }
            else
            {
                ReadStyleMTextPart((StyleMTextPart)mls.m_Content);
            }
        }
        private void ReadStyleBlockPart(StyleBlockPart block)
        {
            int num = 0;
            string buffer = new string(new char[BUFSIZE]);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 343:
                        block.m_BlockName = LookupBlockEntryName(Global.hextoi(buffer));
                        break;
                    case 177:
                        block.m_BlockConnectionType = (AcBlockConnectionType)(Convert.ToInt32(buffer));
                        break;
                    case 94:
                        block.m_BlockColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 47:
                        block.m_BlockScale.x = Convert.ToDouble(buffer);
                        break;
                    case 49:
                        block.m_BlockScale.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadStyleMTextPart(StyleMTextPart mtext)
        {
            int num = 0;
            string buffer = new string(new char[BUFSIZE]);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 342:
                        mtext.m_TextStyle = LookupTextStyleName(Global.hextoi(buffer));
                        break;
                    case 175:
                        mtext.m_TextAngleType = (AcTextAngleType)(Convert.ToInt32(buffer));
                        break;
                    case 93:
                        mtext.m_TextColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 45:
                        mtext.m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 46:
                        mtext.m_AlignSpace = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadTable(EntityList pObjList)
        {
            int num = 0;
            string buffer = null;
            var pTable = new AcadTable();
            SetAttribute(pTable);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 2:
                        pTable.m_TableBlockName = buffer;
                        break;
                    case 10:
                        pTable.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pTable.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 342:
                        //此时转换TableStyle handle到name不会成功。
                        m_ToBeConvertedTableStyleName.Add(pTable.m_Handle, Global.hextoi(buffer));
                        break;
                    case 343:
                        break;
                    case 91:
                        pTable.m_RowCount = Convert.ToInt32(buffer);
                        //pTable.m_RowHeights.reserve(pTable.m_RowCount);
                        break;
                    case 92:
                        pTable.m_ColCount = Convert.ToInt32(buffer);
                        //pTable.m_ColWidths.reserve(pTable.m_ColCount);
                        if (pTable.m_RowCount > 0 && pTable.m_ColCount > 0)
                        {
                            //pTable.m_Cells.reserve(pTable.m_RowCount * pTable.m_ColCount);
                        }
                        break;
                    case 141:
                        pTable.m_RowHeights.Add(Convert.ToDouble(buffer));
                        break;
                    case 142:
                        pTable.m_ColWidths.Add(Convert.ToDouble(buffer));
                        break;
                    case 171:
                        {
                            // Suppose cell begins with group code 171.
                            pTable.m_Cells.Add(new Cell((CellType)(Convert.ToInt32(buffer))));
                            Cell cell = pTable.m_Cells.Last();
                            ReadCell(cell);
                        }
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pTable);
        }
        private void ReadCell(Cell cell)
        {
            int num = 0;
            string buffer = null;
            bool bFirst91 = true;
            bool bBefore301 = true;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 304)
                {
                    // buffer is "ACVALUE_END"
                    break;
                }
                switch (num)
                {
                    case 172:
                        cell.m_Flag = Convert.ToInt32(buffer);
                        break;
                    case 173:
                        cell.m_Merged = Convert.ToInt32(buffer) != 0;
                        break;
                    case 174:
                        cell.m_AutoFit = Convert.ToInt32(buffer) != 0;
                        break;
                    case 175:
                        cell.m_ColSpan = Convert.ToInt32(buffer);
                        break;
                    case 176:
                        cell.m_RowSpan = Convert.ToInt32(buffer);
                        break;
                    case 93:
                        cell.m_Flag93 = Convert.ToInt32(buffer);
                        break;
                    case 90:
                        cell.m_Flag90 = Convert.ToInt32(buffer);
                        break;
                    case 91:
                        if (bFirst91)
                        {
                            cell.m_OverrideFlag1 = Convert.ToInt32(buffer);
                            bFirst91 = false;
                        }
                        else
                        {
                            cell.m_OverrideFlag2 = Convert.ToInt32(buffer);
                        }
                        break;
                    case 178:
                        cell.m_VirtualEdgeFlag = Convert.ToInt32(buffer);
                        break;
                    case 145:
                        cell.m_Rotation = Convert.ToDouble(buffer);
                        break;
                    case 170:
                        cell.m_Alignment = (AcCellAlignment)(Convert.ToInt32(buffer));
                        break;
                    case 283:
                        cell.m_BgColorEnabled = Convert.ToInt32(buffer) != 0;
                        break;
                    case 63:
                        cell.m_BgColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 64:
                        cell.m_TextColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 140:
                        cell.m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 288:
                        cell.m_LeftBorderVisible = Convert.ToInt32(buffer) != 0;
                        break;
                    case 285:
                        cell.m_RightBorderVisible = Convert.ToInt32(buffer) != 0;
                        break;
                    case 289:
                        cell.m_TopBorderVisible = Convert.ToInt32(buffer) != 0;
                        break;
                    case 286:
                        cell.m_BottomBorderVisible = Convert.ToInt32(buffer) != 0;
                        break;
                    case 302:
                    case 303:
                        if (cell.m_Content.GetContentType() == CellType.TEXT_CELL)
                        {
                            CellText pCell = (CellText)cell.m_Content;
                            pCell.m_Text += buffer;
                        }
                        break;
                    case 7:
                        if (cell.m_Content.GetContentType() == CellType.TEXT_CELL)
                        {
                            CellText pCell = (CellText)cell.m_Content;
                            pCell.m_TextStyle = buffer;
                        }
                        break;
                    case 340:
                        if (cell.m_Content.GetContentType() == CellType.BLOCK_CELL)
                        {
                            CellBlock pCell = (CellBlock)cell.m_Content;
                            pCell.m_BlockName = LookupBlockEntryName(Global.hextoi(buffer));
                        }
                        break;
                    case 144:
                        if (cell.m_Content.GetContentType() == CellType.BLOCK_CELL)
                        {
                            CellBlock pCell = (CellBlock)cell.m_Content;
                            pCell.m_BlockScale = Convert.ToDouble(buffer);
                        }
                        break;
                    case 331:
                        if (cell.m_Content.GetContentType() == CellType.BLOCK_CELL)
                        {
                            CellBlock pCell = (CellBlock)cell.m_Content;
                            var sp = m_Graph.FindEntity(Global.hextoi(buffer));
                            var ad = (sp) as AcadAttDef;
                            if (ad != null)
                            {
                                pCell.m_AttrDefs.Add(ad);
                            }
                            else
                            {
                                Global.ASSERT_DEBUG_INFO("Invalid ATTDEF handle.", true);
                            }
                        }
                        break;
                    case 301:
                        if (string.Compare(buffer, "CELL_VALUE") == 0)
                        {
                            bBefore301 = false;
                        }
                        break;
                    case 300:
                        if (bBefore301 && cell.m_Content.GetContentType() == CellType.BLOCK_CELL)
                        {
                            CellBlock pCell = (CellBlock)cell.m_Content;
                            pCell.m_AttrValues.Add(buffer);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadTableStyle(TableStyle ts)
        {
            int num = 0;
            string buffer = null;
            long pos = m_DxfFile.tellg();
            bool first280IsRead = false;
            SortedDictionary<int, int> cellAttribCount = new SortedDictionary<int, int>();
            int curCellIndex = -1;
            Func<int, bool> PushCellIfFirst = (int num1) =>
            {
                int it = 0;
                bool hasIt = cellAttribCount.TryGetValue(num1, out it);
                if (!hasIt)
                {
                    cellAttribCount[num1] = 0;
                }
                else
                {
                    cellAttribCount[num1] = ++it;
                }
                if (it > curCellIndex)
                {
                    ts.m_Cells.Add(new CellInTableStyle());
                    curCellIndex = it;
                }
                return true;
            };
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 40:
                        ts.m_HorCellMargin = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        ts.m_VerCellMargin = Convert.ToDouble(buffer);
                        break;
                    case 280:
                        if (!first280IsRead)
                        {
                            first280IsRead = true;
                        }
                        else
                        {
                            ts.m_HasNoTitle = Convert.ToInt32(buffer) != 0;
                        }
                        break;
                    case 281:
                        ts.m_HasNoColumnHeading = Convert.ToInt32(buffer) != 0;
                        break;
                    case 7:
                        ts.m_TextStyle = buffer;
                        break;
                    case 140:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 170:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_Alignment = (AcCellAlignment)(Convert.ToInt32(buffer));
                        break;
                    case 62:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_TextColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 63:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_BgColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 283:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_BgColorEnabled = Convert.ToInt32(buffer) != 0;
                        break;
                    case 90:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_CellDataType = (AcValueDataType)(Convert.ToInt32(buffer));
                        break;
                    case 91:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_CellUnitType = (AcValueUnitType)(Convert.ToInt32(buffer));
                        break;
                    case 274:
                    case 275:
                    case 276:
                    case 277:
                    case 278:
                    case 279:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_BorderLineWeight[num - 274] = (AcLineWeight)(Convert.ToInt32(buffer));
                        break;
                    case 284:
                    case 285:
                    case 286:
                    case 287:
                    case 288:
                    case 289:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_BorderVisible[num - 284] = Convert.ToInt32(buffer) != 0;
                        break;
                    case 64:
                    case 65:
                    case 66:
                    case 67:
                    case 68:
                    case 69:
                        PushCellIfFirst(num);
                        ts.m_Cells.Last().m_BorderColor[num - 64] = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    default:
                        break;
                }
            }
        }
        private bool ReadObjects()
        {
            //TRACE_FUNCTION("()");

            if (m_OnlyReadModelSpace)
            {
                return true;
            }
            bool bSuccess = AdvanceToSection("OBJECTS");

            // Get handle of dictionary ACAD_MLEADERSTYLE and ACAD_TABLESTYLE
            bSuccess = bSuccess && NextObject("DICTIONARY");
            int mLeaderStyleDicHandle = 0;
            int tableStyleDicHandle = 0;
            ReadDictionaryEntries((NameHandle curDicEntry) =>
            {
                if (curDicEntry.name == "ACAD_MLEADERSTYLE")
                {
                    mLeaderStyleDicHandle = curDicEntry.handle;
                    return false; // continue
                }
                else if (curDicEntry.name == "ACAD_TABLESTYLE")
                {
                    tableStyleDicHandle = curDicEntry.handle;
                    return true; // break
                }
                return false; // continue
            });

            while (bSuccess && NextObject("DICTIONARY"))
            {
                if (m_CurObjectHandle == mLeaderStyleDicHandle)
                {
                    ReadDictionaryEntries((NameHandle curDicEntry) =>
                    {
                        m_MLeaderStyleHandleTable[curDicEntry.handle] = curDicEntry.name;
                        return false; // continue
                    });
                    break;
                }
            }

            // DICTIONARY isn't continuous.
            while (AdvanceToObjectStopAtLayout("DICTIONARY"))
            {
                if (bSuccess && NextObject("DICTIONARY"))
                {
                    if (m_CurObjectHandle == tableStyleDicHandle)
                    {
                        ReadDictionaryEntries((NameHandle curDicEntry) =>
                        {
                            m_TableStyleHandleTable[curDicEntry.handle] = curDicEntry.name;
                            return false; // continue
                        });
                        break;
                    }
                }
            }

            string layoutName = null;
            SortedDictionary<string, LayoutData> layouts = new SortedDictionary<string, LayoutData>();
            bSuccess = bSuccess && AdvanceToObject("LAYOUT");
            while (bSuccess && NextObject("LAYOUT"))
            {
                LayoutData lo = new LayoutData();
                ReadLayout(out layoutName, lo);
                Global.PrintDebugInfo("Read Layout {0}", layoutName);
                if (lo.m_BlockName.StartsWith(g_PaperSpace, StringComparison.InvariantCultureIgnoreCase))
                {
                    var pLayout = LookupLayout(lo.m_BlockName);
                    if (pLayout != null)
                    {
                        pLayout.m_LayoutOrder = lo.m_LayoutOrder;
                        pLayout.m_PlotSettings = lo.m_PlotSettings;
                        layouts[layoutName] = pLayout;
                    }
                }
            }
            // Now Layouts's key is layout name.
            m_Graph.m_Layouts = layouts;

            // MLEADERSTYLE is after LAYOUT
            bSuccess = bSuccess && AdvanceToObject("MLEADERSTYLE");
            while (bSuccess && NextObject("MLEADERSTYLE"))
            {
                MLeaderStyle mls = new MLeaderStyle();
                ReadMLeaderStyle(mls);
                string name = LookupMLeaderStyleName(m_CurObjectHandle);
                Global.PrintDebugInfo("Read MLEADERSTYLE {0}", name);
                m_Graph.m_MLeaderStyles[name] = mls;
            }

            // TABLESTYLE is after MLEADERSTYLE
            bSuccess = bSuccess && AdvanceToObject("TABLESTYLE");
            while (bSuccess && NextObject("TABLESTYLE"))
            {
                TableStyle tbls = new TableStyle();
                ReadTableStyle(tbls);
                string name = LookupTableStyleName(m_CurObjectHandle);
                Global.PrintDebugInfo("Read TABLESTYLE {0}", name);
                m_Graph.m_TableStyles[name] = tbls;
            }

            return bSuccess;
        }
        private string LookupTextStyleName(int handle)
        {
            string value = null;
            bool hasIt = m_TextStyleHandleTable.TryGetValue(handle, out value);
            if (hasIt)
            {
                return value;
            }
            Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to TextStyle name", handle);
            return string.Empty;
        }
        private string LookupBlockEntryName(int handle)
        {
            string value;
            bool hasIt = m_BlockEntryHandleTable.TryGetValue(handle, out value);
            if (hasIt)
            {
                return value;
            }
            Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to Block name", handle);
            return string.Empty;
        }
        private string LookupMLeaderStyleName(int handle)
        {
            string value;
            bool hasIt = m_MLeaderStyleHandleTable.TryGetValue(handle, out value);
            if (hasIt)
            {
                return value;
            }
            Global.PRINT_DEBUG_INFO("Can't convert handle 0x%X to MLeaderStyle name", handle);
            return string.Empty;
        }
        private string LookupLinetypeName(int handle)
        {
            string value;
            bool hasIt = m_LinetypeHandleTable.TryGetValue(handle, out value);
            if (hasIt)
            {
                return value;
            }
            Global.PRINT_DEBUG_INFO("Can't convert handle 0x%X to Linetype name", handle);
            return string.Empty;
        }
        private string LookupTableStyleName(int handle)
        {
            string value;
            bool hasIt = m_TableStyleHandleTable.TryGetValue(handle, out value);
            if (hasIt)
            {
                return value;
            }
            Global.PRINT_DEBUG_INFO("Can't convert handle 0x%X to TableStyle name", handle);
            return string.Empty;
        }
        private LayoutData LookupLayout(string blockName)
        {
            foreach (var lo in m_Graph.m_Layouts)
            {
                if (string.Compare(lo.Value.m_BlockName, blockName, true) == 0)
                {
                    return lo.Value;
                }
            }
            Global.PRINT_DEBUG_INFO("Can't find layout which block name is %s", blockName);
            return null;
        }
        private void ConvertHandle()
        {
            //TRACE_FUNCTION("()");
            foreach(var pair in m_ToBeConvertedReactors)
            {
                int handle = pair.Key;
                var ent = m_Graph.FindEntity(handle);
                if (ent != null)
                {
                    foreach(var reactorHandle in pair.Value)
                    {
                        var reactor = m_Graph.FindEntity(reactorHandle);
                        if (reactor != null)
                        {
                            ent.m_Reactors.Add(new WeakReference(reactor));
                        }
                        else
                        {
                            Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to Reactor", reactorHandle);
                        }
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedViewportClipEnts)
            {
                int handle = pair.Key;
                var vp = m_Graph.FindEntity(handle) as AcadViewport;
                if (vp != null)
                {
                    int clipEntHandle = pair.Value;
                    var clipEnt = m_Graph.FindEntity(clipEntHandle);
                    if (clipEnt != null)
                    {
                        vp.m_ClipEnt = new WeakReference(clipEnt);
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to Clip entity", clipEntHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedLeaderAnnotation)
            {
                int handle = pair.Key;
                var leader = m_Graph.FindEntity(handle) as AcadLeader;
                if (leader != null)
                {
                    int annotationHandle = pair.Value;
                    var annotation = m_Graph.FindEntity(annotationHandle);
                    if (annotation != null)
                    {
                        leader.m_Annotation = new WeakReference(annotation);
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to Annotation", annotationHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedMLeaderAttDefs)
            {
                int handle = pair.Key;
                var mleader = m_Graph.FindEntity(handle) as AcadMLeader;
                if (mleader != null)
                {
                    foreach (var attDefHandle in pair.Value)
                    {
                        var attDef = m_Graph.FindEntity(attDefHandle) as AcadAttDef;
                        if (attDef != null)
                        {
                            mleader.m_AttrDefs.Add(attDef);
                        }
                        else
                        {
                            Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to AttDef", attDef);
                        }
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedMLeaderStyleName)
            {
                int handle = pair.Key;
                var mleader = m_Graph.FindEntity(handle) as AcadMLeader;
                if (mleader != null)
                {
                    int styleHandle = pair.Value;
                    string name = LookupMLeaderStyleName(styleHandle);
                    if (!string.IsNullOrEmpty(name))
                    {
                        mleader.m_LeaderStyle = name;
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to MLeaderStyleName", styleHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedTableStyleName)
            {
                int handle = pair.Key;
                var table = m_Graph.FindEntity(handle) as AcadTable;
                if (table != null)
                {
                    int styleHandle = pair.Value;
                    string name = LookupTableStyleName(styleHandle);
                    if (!string.IsNullOrEmpty(name))
                    {
                        table.m_TableStyle = name;
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to TableStyleName", styleHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedTextStyleName)
            {
                DimStyleData dsd = m_Graph.m_DimStyles[pair.Key];
                if (dsd != null)
                {
                    int styleHandle = pair.Value;
                    string name = LookupTextStyleName(styleHandle);
                    if (!string.IsNullOrEmpty(name))
                    {
                        dsd.TextStyle = name;
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to TextStyle", styleHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedArrow1Name)
            {
                DimStyleData dsd = m_Graph.m_DimStyles[pair.Key];
                if (dsd != null)
                {
                    int blockHandle = pair.Value;
                    string name = LookupBlockEntryName(blockHandle);
                    AcDimArrowheadType arrowType = Global.GetArrowHeadType(name);
                    if (!string.IsNullOrEmpty(name))
                    {
                        dsd.ArrowHead1Block = name;
                        dsd.ArrowHead1Type = arrowType;
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to BlockName", blockHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedArrow2Name)
            {
                DimStyleData dsd = m_Graph.m_DimStyles[pair.Key];
                if (dsd != null)
                {
                    int blockHandle = pair.Value;
                    string name = LookupBlockEntryName(blockHandle);
                    AcDimArrowheadType arrowType = Global.GetArrowHeadType(name);
                    if (!string.IsNullOrEmpty(name))
                    {
                        dsd.ArrowHead2Block = name;
                        dsd.ArrowHead2Type = arrowType;
                    }
                    else
                    {
                        Global.PRINT_DEBUG_INFO("Can't convert handle 0x{0:X} to BlockName", blockHandle);
                    }
                }
            }

            foreach (var pair in m_ToBeConvertedHatchAssociatedEnts)
            {
                int handle = pair.Key;
                var hatch = m_Graph.FindEntity(handle) as AcadHatch;
                if (hatch != null)
                {
                    var list = pair.Value;
                    foreach (var handleLoopIndex in list)
                    {
                        var boundaryEntHandle = handleLoopIndex.Key;
                        var loopIndex = handleLoopIndex.Value;
                        var boundaryEnt = m_Graph.FindEntity(boundaryEntHandle);
                        if (boundaryEnt != null)
                        {
                            hatch.AddAssociatedEntity(boundaryEnt, loopIndex);
                        }
                        else
                        {
                            Global.PRINT_DEBUG_INFO("Source handle 0x{0:X} of loop {1} in Hatch is invalid.", handle, loopIndex);
                        }
                    }
                }
            }
        }
        private void ReadAttribute()
        {
            int num;
            string buffer;
            // Reset to default value.
            m_Attribute.m_IsInPaperspace = false;
            m_Attribute.m_Layer = "0";
            m_Attribute.m_Color = AcColor.acByLayer;
            m_Attribute.m_Linetype = "ByLayer";
            m_Attribute.m_LinetypeScale = 1.0;
            m_Attribute.m_Lineweight = AcLineWeight.acLnWtByLayer;
            m_Attribute.m_ReactorHandles = new List<int>(); // Create a new object, Don't use Clear to reuse original object.
            bool bReactor = false;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 100 && string.Compare(buffer, "AcDbEntity", true) != 0 || num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 67:
                        m_Attribute.m_IsInPaperspace = Convert.ToInt32(buffer) == 1;
                        break;
                    case 5:
                        m_Attribute.m_Handle = Global.hextoi(buffer);
                        break;
                    case 8:
                        m_Attribute.m_Layer = buffer;
                        break;
                    case 6:
                        m_Attribute.m_Linetype = buffer;
                        break;
                    case 62:
                        m_Attribute.m_Color = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 370:
                        m_Attribute.m_Lineweight = (AcLineWeight)(Convert.ToInt32(buffer));
                        break;
                    case 48:
                        m_Attribute.m_LinetypeScale = Convert.ToDouble(buffer);
                        break;
                    case 102:
                        if (string.Compare("{ACAD_REACTORS", buffer) == 0)
                        {
                            bReactor = true;
                        }
                        else if (bReactor)
                        {
                            bReactor = false;
                        }
                        break;
                    case 330:
                        if (bReactor)
                        {
                            m_Attribute.m_ReactorHandles.Add(Global.hextoi(buffer));
                        }
                        else
                        {
                            // int blockEntryHandle = hextoi(buffer);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        private void SetAttribute(EntAttribute pObj)
        {
            pObj.m_Handle = m_Attribute.m_Handle;
            pObj.m_IsInPaperspace = m_Attribute.m_IsInPaperspace;
            pObj.m_Layer = m_Attribute.m_Layer;
            pObj.m_Linetype = m_Attribute.m_Linetype;
            pObj.m_Color = m_Attribute.m_Color;
            pObj.m_Lineweight = m_Attribute.m_Lineweight;
            pObj.m_LinetypeScale = m_Attribute.m_LinetypeScale;
            int reactorSize = m_Attribute.m_ReactorHandles.Count;
            m_ToBeConvertedReactors.Add(m_Attribute.m_Handle, m_Attribute.m_ReactorHandles);
        }

        private void Reset()
        {
            m_TextStyleHandleTable.Clear();
            m_BlockEntryHandleTable.Clear();
            m_MLeaderStyleHandleTable.Clear();
            m_LinetypeHandleTable.Clear();
            m_TableStyleHandleTable.Clear();

            m_ToBeConvertedReactors.Clear();
            m_ToBeConvertedViewportClipEnts.Clear();
            m_ToBeConvertedLeaderAnnotation.Clear();
            m_ToBeConvertedMLeaderAttDefs.Clear();
            m_ToBeConvertedMLeaderStyleName.Clear();
            m_ToBeConvertedTableStyleName.Clear();
            m_ToBeConvertedTextStyleName.Clear();
            m_ToBeConvertedArrow1Name.Clear();
            m_ToBeConvertedArrow2Name.Clear();
            m_ToBeConvertedHatchAssociatedEnts.Clear();
        }

        private void ReadArc(EntityList pObjList)
        {
            int num;
            string buffer;
            var pArc = new AcadArc();
            SetAttribute(pArc);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pArc.m_Center.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pArc.m_Center.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pArc.m_Radius = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pArc.m_StartAngle = Convert.ToDouble(buffer);
                        break;
                    case 51:
                        pArc.m_EndAngle = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pArc);
        }

        private void ReadAttDef(EntityList pObjList)
        {
            int num;
            string buffer;
            var pAttDef = new AcadAttDef();
            SetAttribute(pAttDef);
            bool bFirst280 = true;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pAttDef.m_BaseLeftPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pAttDef.m_BaseLeftPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pAttDef.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pAttDef.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pAttDef.m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pAttDef.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    case 7:
                        pAttDef.m_TextStyle = buffer;
                        break;
                    case 72:
                        pAttDef.m_HorAlign = (short)Convert.ToInt32(buffer);
                        break;
                    case 280:
                        if (bFirst280)
                        {
                            bFirst280 = false;
                        }
                        else
                        {
                            pAttDef.m_DuplicateFlag = Convert.ToInt32(buffer);
                        }
                        break;
                    case 1:
                        pAttDef.m_Text = buffer;
                        break;
                    case 2:
                        pAttDef.m_Tag = buffer;
                        break;
                    case 3:
                        pAttDef.m_Prompt = buffer;
                        break;
                    case 70:
                        pAttDef.m_Flags = (AcAttributeMode)(Convert.ToInt32(buffer));
                        break;
                    case 74:
                        pAttDef.m_VerAlign = (short)Convert.ToInt32(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pAttDef);
        }

        private void ReadAttrib(EntityList pObjList)
        {
            AcadAttrib att = new AcadAttrib();
            int num;
            string buffer;
            SetAttribute(att);
            bool bFirst280 = true;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        att.m_BaseLeftPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        att.m_BaseLeftPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        att.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        att.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        att.m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        att.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    case 7:
                        att.m_TextStyle = buffer;
                        break;
                    case 72:
                        att.m_HorAlign = (short)Convert.ToInt32(buffer);
                        break;
                    case 280:
                        if (bFirst280)
                        {
                            bFirst280 = false;
                        }
                        else
                        {
                            att.m_DuplicateFlag = Convert.ToInt32(buffer);
                        }
                        break;
                    case 1:
                        att.m_Text = buffer;
                        break;
                    case 2:
                        att.m_Tag = buffer;
                        break;
                    case 70:
                        att.m_Flags = (AcAttributeMode)(Convert.ToInt32(buffer));
                        break;
                    case 74:
                        att.m_VerAlign = (short)Convert.ToInt32(buffer);
                        break;
                    default:
                        break;
                }
            }

            if (pObjList.Count != 0)
            {
                AcadBlockInstance pInsert = pObjList.Last() as AcadBlockInstance;
                if (pInsert != null)
                {
                    pInsert.m_Attribs.Add(att);
                    return;
                }
            }
            Global.PRINT_DEBUG_INFO("ATTRIB must be after an INSERT.");
        }

        private void ReadInsert(EntityList pObjList)
        {
            int num;
            string buffer;
            var pBlock = new AcadBlockInstance();
            pBlock.m_Xscale = pBlock.m_Yscale = pBlock.m_Zscale = 1.0;
            SetAttribute(pBlock);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 2:
                        pBlock.m_Name = buffer;
                        break;
                    case 10:
                        pBlock.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pBlock.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pBlock.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pBlock.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pBlock.m_Xscale = Convert.ToDouble(buffer);
                        break;
                    case 42:
                        pBlock.m_Yscale = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pBlock.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pBlock);
        }
        private void ReadCircle(EntityList pObjList)
        {
            int num;
            string buffer;
            var pCircle = new AcadCircle();
            SetAttribute(pCircle);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pCircle.m_Center.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pCircle.m_Center.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pCircle.m_Radius = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pCircle);
        }
        private void ReadDimCommon()
        {
            int num;
            string buffer;
            CDblPoint defPoint = new CDblPoint();
            CDblPoint textPoint = new CDblPoint();
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 100 && string.Compare(buffer, "AcDbDimension", true) != 0 || num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 2:
                        m_Attribute.m_BlockName = buffer;
                        break;
                    case 10:
                        m_Attribute.m_DefPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        m_Attribute.m_DefPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        m_Attribute.m_TextPosition.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        m_Attribute.m_TextPosition.y = Convert.ToDouble(buffer);
                        break;
                    case 70:
                        m_Attribute.m_DimType = Convert.ToInt32(buffer);
                        break;
                    case 1:
                        m_Attribute.OverrideText(buffer);
                        break;
                    case 3:
                        m_Attribute.m_DimStyleName = buffer;
                        break;
                    case 53:
                        m_Attribute.OverrideTextRotation(Convert.ToDouble(buffer));
                        break;
                    default:
                        break;
                }
            }
        }
        private void SetDimCommon(AcadDim pDim)
        {
            pDim.m_DimStyleName = m_Attribute.m_DimStyleName;
            pDim.m_BlockName = m_Attribute.m_BlockName;
            pDim.m_DefPoint.SetPoint(m_Attribute.m_DefPoint);
            pDim.m_TextPosition.SetPoint(m_Attribute.m_TextPosition);
            pDim.m_DimType = m_Attribute.m_DimType;
            pDim.m_Attachment = m_Attribute.m_Attachment;
            pDim.m_TextRotation = m_Attribute.m_TextRotation;
        }
        private void ReadDimension(EntityList pObjList)
        {
            ReadDimCommon();
            int type = m_Attribute.m_DimType & 0xF;
            switch (type)
            {
                case 0:
                    ReadDimRot(pObjList);
                    break;
                case 1:
                    ReadDimAln(pObjList);
                    break;
                case 2:
                    ReadDimAng(pObjList);
                    break;
                case 3:
                    ReadDimDia(pObjList);
                    break;
                case 4:
                    ReadDimRad(pObjList);
                    break;
                case 5:
                    ReadDimAng3P(pObjList);
                    break;
                case 6:
                    ReadDimOrd(pObjList);
                    break;
            }
        }
        private void ReadDimAln(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimAln();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 13:
                        pDim.m_ExtLine1Point.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pDim.m_ExtLine1Point.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        pDim.m_ExtLine2Point.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        pDim.m_ExtLine2Point.y = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pDim.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimOrd(EntityList pObjList)
        {
            int num;
            string buffer = new string(new char[BUFSIZE]);
            var pDim = new AcadDimOrd();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 13:
                        pDim.m_OrdPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pDim.m_OrdPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        pDim.m_LeaderPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        pDim.m_LeaderPoint.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimRot(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimRot();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 13:
                        pDim.m_ExtLine1Point.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pDim.m_ExtLine1Point.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        pDim.m_ExtLine2Point.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        pDim.m_ExtLine2Point.y = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pDim.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimDia(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimDia();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 15:
                        pDim.m_ChordPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        pDim.m_ChordPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pDim.m_LeaderLength = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimRad(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimRad();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 15:
                        pDim.m_ChordPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        pDim.m_ChordPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pDim.m_LeaderLength = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimAng3P(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimAng3P();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 13:
                        pDim.m_FirstEnd.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pDim.m_FirstEnd.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        pDim.m_SecondEnd.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        pDim.m_SecondEnd.y = Convert.ToDouble(buffer);
                        break;
                    case 15:
                        pDim.m_AngleVertex.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        pDim.m_AngleVertex.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadDimAng(EntityList pObjList)
        {
            int num;
            string buffer;
            var pDim = new AcadDimAng();
            SetAttribute(pDim);
            SetDimCommon(pDim);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 13:
                        pDim.m_FirstStart.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pDim.m_FirstStart.y = Convert.ToDouble(buffer);
                        break;
                    case 14:
                        pDim.m_FirstEnd.x = Convert.ToDouble(buffer);
                        break;
                    case 24:
                        pDim.m_FirstEnd.y = Convert.ToDouble(buffer);
                        break;
                    case 15:
                        pDim.m_SecondStart.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        pDim.m_SecondStart.x = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pDim);
        }
        private void ReadEllipse(EntityList pObjList)
        {
            int num;
            string buffer;
            var pEllipse = new AcadEllipse();
            SetAttribute(pEllipse);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pEllipse.m_Center.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pEllipse.m_Center.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pEllipse.m_MajorAxisPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pEllipse.m_MajorAxisPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pEllipse.m_MinorAxisRatio = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pEllipse.m_StartAngle = Convert.ToDouble(buffer);
                        break;
                    case 42:
                        pEllipse.m_EndAngle = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pEllipse);
        }
        private void ReadHatch(EntityList pObjList)
        {
            int num;
            string buffer;
            var pHatch = new AcadHatch();
            SetAttribute(pHatch);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 2:
                        pHatch.m_PatternName = buffer;
                        if (Global.RightCompareNoCase(pHatch.m_PatternName, 3, ",_O") || Global.RightCompareNoCase(pHatch.m_PatternName, 3, ",_I"))
                        {
                            pHatch.m_PatternName = pHatch.m_PatternName.Substring(0, pHatch.m_PatternName.Length - 3);
                        }
                        break;
                    case 70:
                        pHatch.m_FillFlag = (AcadHatch.FillFlag)Convert.ToInt32(buffer);
                        break;
                    case 71:
                        // 关联性标志（关联 = 1；无关联 = 0）；
                        break;
                    case 75:
                        pHatch.m_HatchStyle = (AcHatchStyle)Convert.ToInt32(buffer);
                        break;
                    case 76:
                        pHatch.m_PatternType = (AcPatternType)Convert.ToInt32(buffer);
                        break;
                    case 52:
                        pHatch.m_PatternAngle = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pHatch.m_PatternScale = Convert.ToDouble(buffer);
                        break;
                    case 91:
                        {
                            // 解析边界路径
                            int loopCount = Convert.ToInt32(buffer);
                            // loopIndex == -1, outer loop; loopIndex >=0, inner loops
                            int loopIndex = -2;
                            BoundaryEntType entType = new BoundaryEntType();
                            BoundaryPathFlag pathFlag = new BoundaryPathFlag();
                            CDblPoint point1 = new CDblPoint();
                            CDblPoint point2 = new CDblPoint();
                            int entCount = 0;
                            //union
                            //{
                            AcadLWPLine pLWPLine = null;
                            AcadLine pLine = null;
                            AcadArc pArc = null;
                            AcadEllipse pEllipseArc = null;
                            AcadSpline pSpline = null;
                            //};
                            while (!m_DxfFile.eof())
                            {
                                pos = m_DxfFile.tellg();
                                ReadPair(out num, out buffer);
                                // Last boundary loop
                                if (loopIndex == loopCount - 2)
                                {
                                    if (num == 75 || num == 76 || num == 70 || num == 71 || num == 52 || num == 41 || num == 77 || num == 78)
                                    {
                                        // 如果遇到这些属于Hatch但不属于边界路径的code，必须结束边界路径解析。
                                        m_DxfFile.seekg(pos);
                                        break;
                                    }
                                }
                                if (num == 0)
                                {
                                    // 边界路径解析没能在上面的情形退出，那么至少在遇到0时必须退出。一般不会发生这种情况。
                                    m_DxfFile.seekg(pos);
                                    break;
                                }
                                switch (num)
                                {
                                    // 边界路径类型标志（按位编码）：
                                    // 0 = 默认；1 = 外部；2 = 多段线
                                    // 4 = 导出；8 = 文本框；16 = 最外层
                                    case 92:
                                        ++loopIndex;
                                        pLWPLine = null; // This clears all union members.
                                        pLine = null;
                                        pArc = null;
                                        pEllipseArc = null;
                                        pSpline = null;
                                        pathFlag = (BoundaryPathFlag)(Convert.ToInt32(buffer));
                                        if (loopIndex == -1)
                                        {
                                            pHatch.m_OuterFlag = pathFlag;
                                        }
                                        else
                                        {
                                            pHatch.m_InnerFlags.Add(pathFlag);
                                        }
                                        if (((int)pathFlag & (int)BoundaryPathFlag.LWPline) != 0)
                                        {
                                            var spLWPLine = new AcadLWPLine();
                                            pHatch.AddEntity(spLWPLine, loopIndex);
                                            pLWPLine = spLWPLine;
                                        }
                                        break;
                                    // 该边界路径中的边数（仅当边界不是多段线时）
                                    // 多段线顶点数
                                    case 93:
                                        entCount = Convert.ToInt32(buffer);
                                        break;
                                    // 源边界对象数/样条曲线拟合数据数目
                                    case 97:
                                        break;
                                    // 330 - 源边界对象的参照（多个条目）
                                    case 330:
                                        {
                                            int handle = Global.hextoi(buffer);
                                            List<KeyValuePair<int,int>> list = null;
                                            var hasIt = m_ToBeConvertedHatchAssociatedEnts.TryGetValue(pHatch.m_Handle, out list);
                                            if (!hasIt)
                                            {
                                                list = new List<KeyValuePair<int, int>>();
                                                m_ToBeConvertedHatchAssociatedEnts.Add(pHatch.m_Handle, list);
                                            }
                                            list.Add(new KeyValuePair<int, int>(handle, loopIndex));
                                        }
                                        break;
                                    // 边类型（当边界不是多段线时）：
                                    // 1 = 直线；2 = 圆弧；3 = 椭圆弧；4 = 样条曲线
                                    // "有凸度"标志（当边界是多段线时）
                                    case 72:
                                        if ((pathFlag & BoundaryPathFlag.LWPline) == 0)
                                        {
                                            entType = (BoundaryEntType)(Convert.ToInt32(buffer));
                                            --entCount;
                                            switch (entType)
                                            {
                                                case BoundaryEntType.Line:
                                                    {
                                                        var spLine = new AcadLine();
                                                        pHatch.AddEntity(spLine, loopIndex);
                                                        pLine = spLine;
                                                    }
                                                    break;
                                                case BoundaryEntType.Arc:
                                                    {
                                                        var spArc = new AcadArc();
                                                        pHatch.AddEntity(spArc, loopIndex);
                                                        pArc = spArc;
                                                    }
                                                    break;
                                                case BoundaryEntType.EllipseArc:
                                                    {
                                                        var spEllipse = new AcadEllipse();
                                                        pHatch.AddEntity(spEllipse, loopIndex);
                                                        pEllipseArc = spEllipse;
                                                    }
                                                    break;
                                                case BoundaryEntType.Spline:
                                                    {
                                                        var spSpline = new AcadSpline();
                                                        pHatch.AddEntity(spSpline, loopIndex);
                                                        pSpline = spSpline;
                                                    }
                                                    break;
                                            }
                                        }
                                        break;
                                    case 73:
                                        if ((pathFlag & BoundaryPathFlag.LWPline) != 0)
                                        {
                                            pLWPLine.m_Closed = Convert.ToInt32(buffer) != 0;
                                        }
                                        else if (entType == BoundaryEntType.Spline)
                                        {
                                            int value = Convert.ToInt32(buffer);
                                            if (value != 0)
                                            {
                                                pSpline.m_Flag |= 4;
                                            }
                                        }
                                        break;
                                    case 74:
                                        if (entType == BoundaryEntType.Spline)
                                        {
                                            int value = Convert.ToInt32(buffer);
                                            if (value != 0)
                                            {
                                                pSpline.m_Flag |= 4;
                                            }
                                        }
                                        break;
                                    case 95:
                                        break;
                                    case 96:
                                        break;
                                    // 多段线顶点/线段起点/[椭]圆弧中心/样条曲线控制点（在 OCS 中）
                                    case 10:
                                        point1.x = Convert.ToDouble(buffer);
                                        break;
                                    case 20:
                                        point1.y = Convert.ToDouble(buffer);
                                        if ((pathFlag & BoundaryPathFlag.LWPline) != 0)
                                        {
                                            // must new a point
                                            pLWPLine.m_Vertices.Add(new CDblPoint(point1));
                                        }
                                        else if (entType == BoundaryEntType.Line)
                                        {
                                            // must new a point
                                            pLine.m_StartPoint = new CDblPoint(point1);
                                        }
                                        else if (entType == BoundaryEntType.Arc)
                                        {
                                            // must new a point
                                            pArc.m_Center = new CDblPoint(point1);
                                        }
                                        else if (entType == BoundaryEntType.EllipseArc)
                                        {
                                            // must new a point
                                            pEllipseArc.m_Center = new CDblPoint(point1);
                                        }
                                        else if (entType == BoundaryEntType.Spline)
                                        {
                                            // must new a point
                                            pSpline.m_ControlPoints.Add(new CDblPoint(point1));
                                        }
                                        break;
                                    // 线段终点/椭圆相对于中心点的长轴端点/样条线拟合点（在 OCS 中）
                                    case 11:
                                        point2.x = Convert.ToDouble(buffer);
                                        break;
                                    case 21:
                                        point2.y = Convert.ToDouble(buffer);
                                        if (entType == BoundaryEntType.Line)
                                        {
                                            // must new a point
                                            pLine.m_EndPoint = new CDblPoint(point2);
                                        }
                                        else if (entType == BoundaryEntType.EllipseArc)
                                        {
                                            // must new a point
                                            pEllipseArc.m_MajorAxisPoint = new CDblPoint(point2);
                                        }
                                        else if (entType == BoundaryEntType.Spline)
                                        {
                                            // must new a point
                                            pSpline.m_FitPoints.Add(new CDblPoint(point2));
                                        }
                                        break;
                                    case 12:
                                        if (entType == BoundaryEntType.Spline)
                                        {
                                            pSpline.m_StartTangent.x = Convert.ToDouble(buffer);
                                        }
                                        break;
                                    case 22:
                                        if (entType == BoundaryEntType.Spline)
                                        {
                                            pSpline.m_StartTangent.y = Convert.ToDouble(buffer);
                                        }
                                        break;
                                    case 13:
                                        if (entType == BoundaryEntType.Spline)
                                        {
                                            pSpline.m_EndTangent.x = Convert.ToDouble(buffer);
                                        }
                                        break;
                                    case 23:
                                        if (entType == BoundaryEntType.Spline)
                                        {
                                            pSpline.m_EndTangent.y = Convert.ToDouble(buffer);
                                        }
                                        break;
                                    // 多段线凸度（可选；默认值 = 0）
                                    case 42:
                                        if ((pathFlag & BoundaryPathFlag.LWPline) != 0)
                                        {
                                            pLWPLine.SetBulge(pLWPLine.m_Vertices.Count - 1, Convert.ToDouble(buffer));
                                        }
                                        break;
                                    // 圆的半径/短轴的长度（占长轴长度的比例）/节点值（多个条目）
                                    case 40:
                                        if (entType == BoundaryEntType.Arc)
                                        {
                                            pArc.m_Radius = Convert.ToDouble(buffer);
                                        }
                                        else if (entType == BoundaryEntType.EllipseArc)
                                        {
                                            pEllipseArc.m_MinorAxisRatio = Convert.ToDouble(buffer);
                                        }
                                        else if (entType == BoundaryEntType.Spline)
                                        {
                                            pSpline.m_Knots.Add(Convert.ToDouble(buffer));
                                        }
                                        break;
                                    // 起点角度
                                    case 50:
                                        if (entType == BoundaryEntType.Arc)
                                        {
                                            pArc.m_StartAngle = Convert.ToDouble(buffer);
                                        }
                                        else if (entType == BoundaryEntType.EllipseArc)
                                        {
                                            pEllipseArc.m_StartAngle = Convert.ToDouble(buffer);
                                        }
                                        break;
                                    // 终点角度
                                    case 51:
                                        if (entType == BoundaryEntType.Arc)
                                        {
                                            pArc.m_EndAngle = Convert.ToDouble(buffer);
                                        }
                                        else if (entType == BoundaryEntType.EllipseArc)
                                        {
                                            pEllipseArc.m_EndAngle = Convert.ToDouble(buffer);
                                        }
                                        break;
                                }
                            }
                        }
                        break;
                    case 47:
                        pHatch.m_PixelSize = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pHatch);
        }
        private void ReadLeader(EntityList pObjList)
        {
            int num;
            string buffer;
            var pLeader = new AcadLeader();
            SetAttribute(pLeader);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 3:
                        pLeader.m_StyleName = buffer;
                        break;
                    case 71:
                        pLeader.m_Type = (AcLeaderType)((int)pLeader.m_Type & (Convert.ToInt32(buffer) << 1));
                        break;
                    case 72:
                        pLeader.m_Type = (AcLeaderType)((int)pLeader.m_Type & Convert.ToInt32(buffer));
                        break;
                    case 10:
                        pLeader.m_Vertices.Add(new CDblPoint(Convert.ToDouble(buffer), 0));
                        break;
                    case 20:
                        pLeader.m_Vertices[pLeader.m_Vertices.Count - 1].y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pLeader.m_CharHeight = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pLeader.m_TextWidth = Convert.ToDouble(buffer);
                        break;
                    case 340:
                        {
                            int handle = Global.hextoi(buffer);
                            m_ToBeConvertedLeaderAnnotation.Add(pLeader.m_Handle, handle);
                        }
                        break;
                    case 1040:
                        pLeader.m_ArrowSize = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pLeader);
        }
        private void ReadMLeader(EntityList pObjList)
        {
            int num;
            string buffer;
            var pLeader = new AcadMLeader();
            SetAttribute(pLeader);
            ReadContextData(pLeader.m_ContextData);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 340:
                        //此时转换MLeaderStyle handle到name不会成功。
                        m_ToBeConvertedMLeaderStyleName.Add(pLeader.m_Handle, Global.hextoi(buffer));
                        break;
                    case 170:
                        pLeader.m_LeaderType = (AcMLeaderType)(Convert.ToInt32(buffer));
                        break;
                    case 341:
                        pLeader.m_LineType = LookupLinetypeName(Global.hextoi(buffer));
                        break;
                    case 171:
                        pLeader.m_LineWeight = (AcLineWeight)(Convert.ToInt32(buffer));
                        break;
                    case 290:
                        pLeader.m_EnableLanding = Convert.ToInt32(buffer) != 0;
                        break;
                    case 291:
                        pLeader.m_EnableDogleg = Convert.ToInt32(buffer) != 0;
                        break;
                    //case 41:
                    //	pLeader->m_DoglegLength = atof(buffer);
                    //	break;
                    //case 42:
                    //	pLeader->m_ArrowSize = atof(buffer);
                    //	break;
                    //case 343:
                    //	pLeader->m_TextStyle = LookupTextStyleName(hextoi(buffer));
                    //	break;
                    case 330:
                        {
                            List<int> list;
                            bool hasIt = m_ToBeConvertedMLeaderAttDefs.TryGetValue(pLeader.m_Handle, out list);
                            if (!hasIt)
                            {
                                list = new List<int>();
                                m_ToBeConvertedMLeaderAttDefs.Add(pLeader.m_Handle, list);
                            }
                            list.Add(Global.hextoi(buffer));
                        }
                        break;
                    case 302:
                        pLeader.m_AttrValues.Add(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pLeader);
        }
        private void ReadContextData(CONTEXT_DATA cd)
        {
            int num;
            string buffer;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                // Begin of CONTEXT_DATA
                if (num == 300)
                {
                    break;
                }
            }
            ContentType ct = GetContentData(cd);
            if (ct == ContentType.BLOCK_TYPE)
            {
                cd.m_Content = new BlockPart();
                ReadBlockPart((BlockPart)cd.m_Content);
            }
            else
            {
                cd.m_Content = new MTextPart();
                ReadMTextPart((MTextPart)cd.m_Content);
            }
            ReadLeaderPart(cd.m_Leader);
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                // End of CONTEXT_DATA
                if (num == 301)
                {
                    break;
                }
            }
        }
        private void ReadLeaderPart(LeaderPart leader)
        {
            int num;
            string buffer;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 303)
                {
                    break;
                }
                if (num == 304)
                {
                    //leader.m_Lines.Resize(leader.m_Lines.Count + 1);
                    leader.m_Lines.Add(new LeaderLine());
                    ReadLeaderLine(leader.m_Lines.Last());
                }
                switch (num)
                {
                    case 10:
                        leader.m_CommonPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        leader.m_CommonPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        leader.m_DoglegLength = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadLeaderLine(LeaderLine leaderLine)
        {
            int num;
            string buffer;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                if (num == 305)
                {
                    break;
                }
                switch (num)
                {
                    case 10:
                        //leaderLine.m_Points.Resize(leaderLine.m_Points.Count + 1);
                        leaderLine.m_Points.Add(new CDblPoint());
                        leaderLine.m_Points.Last().x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        leaderLine.m_Points.Last().y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
        }
        private ContentType GetContentData(CONTEXT_DATA cd)
        {
            long pos = m_DxfFile.tellg();
            int num;
            string buffer;
            ContentType ct = ContentType.MTEXT_TYPE;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                // Start of LEADER
                if (num == 302)
                {
                    break;
                }
                switch (num)
                {
                    case 10:
                        cd.m_LandingPosition.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        cd.m_LandingPosition.y = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        cd.m_TextHeight = Convert.ToDouble(buffer);
                        break;
                    case 140:
                        cd.m_ArrowSize = Convert.ToDouble(buffer);
                        break;
                    case 145:
                        cd.m_LandingGap = Convert.ToDouble(buffer);
                        break;
                    case 290:
                        if (Convert.ToInt32(buffer) != 0)
                        {
                            ct = ContentType.MTEXT_TYPE;
                        }
                        break;
                    case 296:
                        if (Convert.ToInt32(buffer) != 0)
                        {
                            ct = ContentType.BLOCK_TYPE;
                        }
                        break;
                    default:
                        break;
                }
            }

            m_DxfFile.seekg(pos);
            return ct;
        }
        private void ReadBlockPart(BlockPart block)
        {
            int num;
            string buffer;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                // Start of LEADER
                if (num == 302)
                {
                    break;
                }
                switch (num)
                {
                    case 341:
                        block.m_BlockName = LookupBlockEntryName(Global.hextoi(buffer));
                        break;
                    case 93:
                        block.m_BlockColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 15:
                        block.m_BlockPosition.x = Convert.ToDouble(buffer);
                        break;
                    case 25:
                        block.m_BlockPosition.y = Convert.ToDouble(buffer);
                        break;
                    case 16:
                        block.m_BlockScale.x = Convert.ToDouble(buffer);
                        break;
                    case 26:
                        block.m_BlockScale.y = Convert.ToDouble(buffer);
                        break;
                    case 46:
                        block.m_BlockRotation = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadMTextPart(MTextPart mtext)
        {
            int num;
            string buffer;
            while (!m_DxfFile.eof())
            {
                ReadPair(out num, out buffer);
                // Start of LEADER
                if (num == 302)
                {
                    break;
                }
                switch (num)
                {
                    case 304:
                        mtext.m_Text = buffer;
                        break;
                    case 43:
                        mtext.m_TextWidth = Convert.ToDouble(buffer);
                        break;
                    case 340:
                        mtext.m_TextStyle = LookupTextStyleName(Global.hextoi(buffer));
                        break;
                    case 90:
                        mtext.m_TextColor = (AcColor)(Convert.ToInt32(buffer));
                        break;
                    case 12:
                        mtext.m_TextLocation.x = Convert.ToDouble(buffer);
                        break;
                    case 22:
                        mtext.m_TextLocation.y = Convert.ToDouble(buffer);
                        break;
                    case 42:
                        mtext.m_TextRotation = Convert.ToDouble(buffer);
                        break;
                    case 171:
                        mtext.m_AttachmentPoint = (AcAttachmentPoint)(Convert.ToInt32(buffer));
                        break;
                    default:
                        break;
                }
            }
        }
        private void ReadLine(EntityList pObjList)
        {
            int num;
            string buffer;
            var pLine = new AcadLine();
            SetAttribute(pLine);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pLine.m_StartPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pLine.m_StartPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pLine.m_EndPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pLine.m_EndPoint.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pLine);
        }
        private void ReadLwpolyline(EntityList pObjList)
        {
            int num;
            double temp;
            string buffer;
            var pLine = new AcadLWPLine();
            SetAttribute(pLine);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 70:
                        pLine.m_Closed = (Convert.ToInt32(buffer) & 1) != 0;
                        break;
                    case 43:
                        pLine.SetConstWidth(Convert.ToDouble(buffer));
                        break;
                    case 10:
                        temp = Convert.ToDouble(buffer);
                        pLine.m_Vertices.Add(new CDblPoint(temp, 0));
                        break;
                    case 20:
                        pLine.m_Vertices[pLine.m_Vertices.Count - 1].y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pLine.SetStartWidth(pLine.m_Vertices.Count - 1, Convert.ToDouble(buffer));
                        break;
                    case 41:
                        pLine.SetEndWidth(pLine.m_Vertices.Count - 1, Convert.ToDouble(buffer));
                        break;
                    case 42:
                        pLine.SetBulge(pLine.m_Vertices.Count - 1, Convert.ToDouble(buffer));
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pLine);
        }
        private void ReadMtext(EntityList pObjList)
        {
            int num;
            double cosRot = 0;
            double sinRot;
            string buffer = new string(new char[BUFSIZE]);
            var pMText = new AcadMText();
            SetAttribute(pMText);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 1:
                    case 3:
                        pMText.m_Text += buffer;
                        break;
                    case 7:
                        pMText.m_StyleName = buffer;
                        break;
                    case 10:
                        pMText.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pMText.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        cosRot = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        sinRot = Convert.ToDouble(buffer);
                        pMText.m_RotationAngle = Math.Atan2(sinRot, cosRot) * 180.0 / Math.PI;
                        break;
                    case 40:
                        pMText.m_CharHeight = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pMText.m_Width = Convert.ToDouble(buffer);
                        break;
                    case 44:
                        pMText.m_LineSpacingFactor = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pMText.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    case 71:
                        pMText.m_AttachmentPoint = (AcAttachmentPoint)Convert.ToInt32(buffer);
                        break;
                    case 72:
                        pMText.m_DrawingDirection = (AcDrawingDirection)Convert.ToInt32(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pMText);
        }
        private void ReadPoint(EntityList pObjList)
        {
            int num;
            string buffer;
            var pPoint = new AcadPoint();
            SetAttribute(pPoint);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pPoint.m_Point.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pPoint.m_Point.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pPoint);
        }
        private void ReadSolid(EntityList pObjList)
        {
            int num;
            string buffer = new string(new char[BUFSIZE]);
            var pSolid = new AcadSolid();
            SetAttribute(pSolid);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 10:
                        pSolid.m_Point1.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pSolid.m_Point1.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pSolid.m_Point2.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pSolid.m_Point2.y = Convert.ToDouble(buffer);
                        break;
                    case 12:
                        pSolid.m_Point3.x = Convert.ToDouble(buffer);
                        break;
                    case 22:
                        pSolid.m_Point3.y = Convert.ToDouble(buffer);
                        break;
                    case 13:
                        pSolid.m_Point4.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pSolid.m_Point4.y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pSolid);
        }
        private void ReadSpline(EntityList pObjList)
        {
            int num;
            string buffer;
            int numberOfKnots;
            int numberOfControlPoints;
            int numberOfFitPoints;
            var pSpline = new AcadSpline();
            SetAttribute(pSpline);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 70:
                        pSpline.m_Flag = Convert.ToInt32(buffer);
                        break;
                    case 71:
                        pSpline.m_Degree = Convert.ToInt32(buffer);
                        break;
                    case 72:
                        numberOfKnots = Convert.ToInt32(buffer);
                        break;
                    case 73:
                        numberOfControlPoints = Convert.ToInt32(buffer);
                        break;
                    case 74:
                        numberOfFitPoints = Convert.ToInt32(buffer);
                        break;
                    case 12:
                        pSpline.m_StartTangent.x = Convert.ToDouble(buffer);
                        break;
                    case 22:
                        pSpline.m_StartTangent.y = Convert.ToDouble(buffer);
                        break;
                    case 13:
                        pSpline.m_EndTangent.x = Convert.ToDouble(buffer);
                        break;
                    case 23:
                        pSpline.m_EndTangent.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pSpline.m_Knots.Add(Convert.ToDouble(buffer));
                        break;
                    case 41:
                        pSpline.m_Weights.Add(Convert.ToDouble(buffer));
                        break;
                    case 10:
                        pSpline.m_ControlPoints.Add(new CDblPoint(Convert.ToDouble(buffer), 0));
                        break;
                    case 20:
                        pSpline.m_ControlPoints[pSpline.m_ControlPoints.Count - 1].y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pSpline.m_FitPoints.Add(new CDblPoint(Convert.ToDouble(buffer), 0));
                        break;
                    case 21:
                        pSpline.m_FitPoints[pSpline.m_FitPoints.Count - 1].y = Convert.ToDouble(buffer);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pSpline);
        }
        private void ReadText(EntityList pObjList)
        {
            int num;
            string buffer;
            var pText = new AcadText();
            SetAttribute(pText);
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 1:
                        pText.m_Text = buffer;
                        break;
                    case 7:
                        pText.m_StyleName = buffer;
                        break;
                    case 10:
                        pText.m_BaseLeftPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        pText.m_BaseLeftPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 11:
                        pText.m_InsertionPoint.x = Convert.ToDouble(buffer);
                        break;
                    case 21:
                        pText.m_InsertionPoint.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        pText.m_Height = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        pText.m_WidthFactor = Convert.ToDouble(buffer);
                        break;
                    case 50:
                        pText.m_RotationAngle = Convert.ToDouble(buffer);
                        break;
                    case 51:
                        pText.SetObliqueAngle(Convert.ToDouble(buffer));
                        break;
                    case 72:
                        pText.m_HorAlign = (short)(Convert.ToInt32(buffer) & 0x7);
                        break;
                    case 73:
                        pText.m_VerAlign = (short)((Convert.ToInt32(buffer) & 0x3) << 16);
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(pText);
        }
        private void ReadViewport(EntityList pObjList)
        {
            int num;
            string buffer;
            var vp = new AcadViewport();
            SetAttribute(vp);
            bool HasClipEnt = false;
            int viewPortID = 0;
            while (!m_DxfFile.eof())
            {
                long pos = m_DxfFile.tellg();
                ReadPair(out num, out buffer);
                if (num == 0)
                {
                    m_DxfFile.seekg(pos);
                    break;
                }
                switch (num)
                {
                    case 69:
                        viewPortID = Convert.ToInt32(buffer);
                        break;
                    case 10:
                        vp.m_PaperspaceCenter.x = Convert.ToDouble(buffer);
                        break;
                    case 20:
                        vp.m_PaperspaceCenter.y = Convert.ToDouble(buffer);
                        break;
                    case 40:
                        vp.m_PaperspaceWidth = Convert.ToDouble(buffer);
                        break;
                    case 41:
                        vp.m_PaperspaceHeight = Convert.ToDouble(buffer);
                        break;
                    case 12:
                        vp.m_ModelSpaceCenter.x = Convert.ToDouble(buffer);
                        break;
                    case 22:
                        vp.m_ModelSpaceCenter.y = Convert.ToDouble(buffer);
                        break;
                    case 45:
                        vp.m_ModelSpaceHeight = Convert.ToDouble(buffer);
                        break;
                    case 51:
                        vp.m_TwistAngle = Convert.ToDouble(buffer);
                        break;
                    case 90:
                        {
                            int flag = Convert.ToInt32(buffer);
                            HasClipEnt = (flag & 0x10000) != 0;
                        }
                        break;
                    case 340:
                        {
                            int handle = Global.hextoi(buffer);
                            m_ToBeConvertedViewportClipEnts.Add(vp.m_Handle, handle);
                        }
                        break;
                    default:
                        break;
                }
            }
            pObjList.Add(vp);
        }

        private DXF.ifstream m_DxfFile = null;
        private DxfData m_Graph;
        private AcadDimInternal m_Attribute = new AcadDimInternal();
        private bool m_OnlyReadModelSpace;
        private SortedDictionary<int, string> m_TextStyleHandleTable = new SortedDictionary<int, string>();
        private SortedDictionary<int, string> m_BlockEntryHandleTable = new SortedDictionary<int, string>();
        private SortedDictionary<int, string> m_MLeaderStyleHandleTable = new SortedDictionary<int, string>();
        private SortedDictionary<int, string> m_LinetypeHandleTable = new SortedDictionary<int, string>();
        private SortedDictionary<int, string> m_TableStyleHandleTable = new SortedDictionary<int, string>();

        private SortedDictionary<int, List<int>> m_ToBeConvertedReactors = new SortedDictionary<int, List<int>>();
        private SortedDictionary<int, int> m_ToBeConvertedViewportClipEnts = new SortedDictionary<int, int>();
        private SortedDictionary<int, int> m_ToBeConvertedLeaderAnnotation = new SortedDictionary<int, int>();
        private SortedDictionary<int, List<int>> m_ToBeConvertedMLeaderAttDefs = new SortedDictionary<int, List<int>>();
        private SortedDictionary<int, int> m_ToBeConvertedMLeaderStyleName = new SortedDictionary<int, int>();
        private SortedDictionary<int, int> m_ToBeConvertedTableStyleName = new SortedDictionary<int, int>();
        private SortedDictionary<string, int> m_ToBeConvertedTextStyleName = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_ToBeConvertedArrow1Name = new SortedDictionary<string, int>();
        private SortedDictionary<string, int> m_ToBeConvertedArrow2Name = new SortedDictionary<string, int>();
        private SortedDictionary<int, List<KeyValuePair<int, int>>> m_ToBeConvertedHatchAssociatedEnts = new SortedDictionary<int, List<KeyValuePair<int, int>>>();
        private int m_CurObjectHandle;
    }
}
