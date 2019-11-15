using System;
using System.Collections.Generic;
using System.IO;

namespace DXF
{

    public class DxfData
    {
        public DxfData()
        {
            this.m_LinetypeScale = 1.0;
            this.m_TmpBlockNo = 0;
        }
        public EntAttribute FindEntity(int handle)
        {
            var it = m_Objects.FindEntity(handle);
            if (it != -1)
            {
                return m_Objects[it];
            }

            foreach (var pair in m_RealBlockDefs)
            {
                it = pair.Value.m_Objects.FindEntity(handle);
                if (it != -1)
                {
                    return pair.Value.m_Objects[it];
                }
            }

            foreach (var ld in m_Layouts)
            {
                it = ld.Value.m_Objects.FindEntity(handle);
                if (it != -1)
                {
                    return ld.Value.m_Objects[it];
                }
            }

            return null;
        }
        public void Reset()
        {
            m_Linetypes.Clear();
            m_Layers.Clear();
            m_TextStyles.Clear();
            m_DimStyles.Clear();
            m_MLeaderStyles.Clear();
            m_TableStyles.Clear();

            m_Objects.Clear();
            m_Layouts.Clear();
            m_RealBlockDefs.Clear();
            m_TmpBlockNo = 0;
        }

        public void AddContent(DxfData another, string modelBlockName)
        {
            foreach (var pair in another.m_Linetypes)            {                if (!m_Linetypes.Contains(pair))                    m_Linetypes.Add(pair);            }            foreach (var pair in another.m_Layers)            {                if (!m_Layers.ContainsKey(pair.Key))                    m_Layers.Add(pair.Key, pair.Value);            }            foreach (var pair in another.m_TextStyles)            {                if (!m_TextStyles.ContainsKey(pair.Key))                    m_TextStyles.Add(pair.Key, pair.Value);            }            foreach (var pair in another.m_DimStyles)            {                if (!m_DimStyles.ContainsKey(pair.Key))                    m_DimStyles.Add(pair.Key, pair.Value);            }            foreach (var pair in another.m_MLeaderStyles)            {                if (!m_MLeaderStyles.ContainsKey(pair.Key))                    m_MLeaderStyles.Add(pair.Key, pair.Value);            }            foreach (var pair in another.m_TableStyles)            {                if (!m_TableStyles.ContainsKey(pair.Key))                    m_TableStyles.Add(pair.Key, pair.Value);            }            foreach (var pair in another.m_RealBlockDefs)            {                if (!m_RealBlockDefs.ContainsKey(pair.Key))                    m_RealBlockDefs.Add(pair.Key, pair.Value);            }            if (!string.IsNullOrEmpty(modelBlockName) && !m_RealBlockDefs.ContainsKey(modelBlockName))            {                DXF.BlockDef blockDef = new DXF.BlockDef();                blockDef.m_Objects = another.m_Objects;                m_RealBlockDefs.Add(modelBlockName, blockDef);            }
        }

        public double m_LinetypeScale;

        //被图元引用的线型、图层、文字样式、标注样式不可以被删除，也不可以被更名。
        public SortedSet<string> m_Linetypes = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
        public SortedDictionary<string, LayerData> m_Layers = new SortedDictionary<string, LayerData>();
        public SortedDictionary<string, TextStyleData> m_TextStyles = new SortedDictionary<string, TextStyleData>();
        public SortedDictionary<string, DimStyleData> m_DimStyles = new SortedDictionary<string, DimStyleData>();
        public SortedDictionary<string, MLeaderStyle> m_MLeaderStyles = new SortedDictionary<string, MLeaderStyle>();
        public SortedDictionary<string, TableStyle> m_TableStyles = new SortedDictionary<string, TableStyle>();

        //保存模型空间对象
        public EntityList m_Objects = new EntityList();
        //保存所有布局, include *Papaer_Space
        public SortedDictionary<string, LayoutData> m_Layouts = new SortedDictionary<string, LayoutData>();
        //保存AutoCAD块的定义
        public SortedDictionary<string, BlockDef> m_RealBlockDefs = new SortedDictionary<string, BlockDef>();

        delegate void GenerateBlockNameFn(int blockNo);
        internal int PrepareBeforeWrite()
        {
            // Remove must-not elements
            m_Linetypes.Remove("ByBlock");
            m_Linetypes.Remove("ByLayer");
            m_Linetypes.Remove("Continuous");

            // Add must-have elements
            if (!m_Layers.ContainsKey("0"))
            {
                m_Layers["0"] = new LayerData();
            }

            if (!m_TextStyles.ContainsKey("Standard"))
            {
                m_TextStyles["Standard"] = new TextStyleData();
            }

            if (!m_DimStyles.ContainsKey("ISO-25"))
            {
                m_DimStyles["ISO-25"] = new DimStyleData(0);
            }

            if (!m_MLeaderStyles.ContainsKey("Standard"))
            {
                m_MLeaderStyles["Standard"] = new MLeaderStyle();
            }

            if (!m_MLeaderStyles.ContainsKey("Annotative"))
            {
                m_MLeaderStyles["Annotative"] = new MLeaderStyle();
                m_MLeaderStyles["Annotative"].m_IsAnnotative = true;
            }

            if (!m_TableStyles.ContainsKey("Standard"))
            {
                m_TableStyles["Standard"] = new TableStyle();
            }

            // Generate blocks for dimensions
            string blockName = null;
            GenerateBlockNameFn GenerateBlockName = (int blockNo) =>
            {
                blockName = String.Format("*D{0}", blockNo);
            };
            foreach (var ent in m_Objects)
            {
                AcadDim pDim = ent as AcadDim;
                if (pDim != null && String.IsNullOrEmpty(pDim.m_BlockName))
                {
                    GenerateBlockName(++m_TmpBlockNo);
                    pDim.GenerateBlock(this, blockName);
                }
            }

            foreach (var pair in m_RealBlockDefs)
            {
                foreach (var ent in pair.Value.m_Objects)
                {
                    AcadDim pDim = ent as AcadDim;
                    if (pDim != null && String.IsNullOrEmpty(pDim.m_BlockName))
                    {
                        GenerateBlockName(++m_TmpBlockNo);
                        pDim.GenerateBlock(this, blockName);
                    }
                }
            }

            foreach (var ld in m_Layouts)
            {
                foreach (var ent in ld.Value.m_Objects)
                {
                    AcadDim pDim = ent as AcadDim;
                    if (pDim != null && String.IsNullOrEmpty(pDim.m_BlockName))
                    {
                        GenerateBlockName(++m_TmpBlockNo);
                        pDim.GenerateBlock(this, blockName);
                    }
                }
            }

            // Assign handle to entities. Start from 10000, it should be enough to hold other handles.
            int handle = 10000;

            // It is harmless to assign handle to entities even if it is not exported to DXF.
            foreach (var pb in DxfWriter.s_PredefinedBlocks)
            {
                foreach (var ent in pb)
                {
                    ent.AssignHandle(ref handle);
                }
            }

            foreach (var ent in m_Objects)
            {
                ent.AssignHandle(ref handle);
            }

            foreach (var pair in m_RealBlockDefs)
            {
                foreach (var ent in pair.Value.m_Objects)
                {
                    ent.AssignHandle(ref handle);
                }
            }

            // Add must-have layout
            if (m_Layouts.Count == 0)
            {
                var pLayout = new LayoutData();
                m_Layouts["Layout1"] = pLayout;
            }

            int i = -1;
            foreach (var ld in m_Layouts)
            {
                // Assign block name to layouts
                if (i == -1)
                {
                    ld.Value.m_BlockName = "*Paper_Space";
                }
                else
                {
                    ld.Value.m_BlockName = String.Format("*Paper_Space{0}", i);
                }
                ld.Value.m_LayoutOrder = i + 2;
                ++i;
                foreach (var ent in ld.Value.m_Objects)
                {
                    ent.AssignHandle(ref handle);
                }
            }

            return handle;
        }

        //	friend class DxfReader;
        // Keep last generated block number, start from 1, *D1, *D2, etc
        private int m_TmpBlockNo;
    }
}
