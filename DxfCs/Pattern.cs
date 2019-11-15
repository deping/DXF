using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace DXF
{
    public class Pattern_Line
    {
        public double angle; // in degrees
        public double x_origin;
        public double y_origin;
        public double delta_x;
        public double delta_y;
        public List<double> dashes = new List<double>();
    }

    public class Pattern
    {
        //返回true表示在图案文件patFile中找到patName图案，否则没找到。
        public bool LoadPattern(PatternManager manager, string patName)
        {
            bool bFound = false;
            Global.ASSERT_DEBUG_INFO("manager.m_PatFile != null", manager.m_PatFile != null);
            DXF.ifstream s_PatFile = manager.m_PatFile;
            s_PatFile.reset();
            s_PatFile.setdelimiter(",");
            string buffer = null;
            while (!s_PatFile.eof())
            {
                s_PatFile.read(out buffer);
                if (buffer[0] == ';')
                {
                    if (bFound)
                    {
                        return true;
                    }
                    s_PatFile.ignore(Int32.MaxValue, '\n');
                }
                else if (buffer[0] == '*')
                {
                    if (bFound)
                    {
                        return true;
                    }
                    name = buffer.Substring(1);
                    if (String.Compare(name, patName, true) == 0)
                    {
                        bFound = true;
                    }
                    s_PatFile.ignore(Int32.MaxValue, ',');
                    s_PatFile.getline(out buffer, 512);
                    if (bFound)
                    {
                        description = buffer;
                    }
                }
                else
                {
                    if (!bFound)
                    {
                        s_PatFile.ignore(Int32.MaxValue, '\n');
                        continue;
                    }

                    var dashline = new Pattern_Line();
                    dashline.angle = Convert.ToDouble(buffer);
                    s_PatFile.ignore(Int32.MaxValue, ',');
                    s_PatFile.read(out dashline.x_origin);
                    s_PatFile.ignore(Int32.MaxValue, ',');
                    s_PatFile.read(out dashline.y_origin);
                    s_PatFile.ignore(Int32.MaxValue, ',');
                    s_PatFile.read(out dashline.delta_x);
                    s_PatFile.ignore(Int32.MaxValue, ',');
                    s_PatFile.read(out dashline.delta_y);

                    s_PatFile.getline(out buffer, 512);
                    string[] parts = buffer.Split(',');
                    foreach (var part in parts)
                    {
                        var tmp = part.Trim();
                        if (tmp.Length == 0)
                            continue;
                        double dash = double.Parse(tmp);
                        dashline.dashes.Add(dash);
                    }
                    dashlines.Add(dashline);
                }
            }

            return false;
        }

        public string name;
        public string description;
        public List<Pattern_Line> dashlines = new List<Pattern_Line>();
    }
    public class PatternManager
    {
        public void SetPatFile(string pPatFile)
        {
            if (m_PatFile != null)
                m_PatFile.Close();
            m_PatFile = new ifstream(new FileStream(pPatFile, FileMode.Open, FileAccess.Read));
        }

        public bool IsValid()
        {
            return m_PatFile != null;
        }

        public Pattern FindPattern(string patName)
        {
            foreach (var pPattern in m_Patterns)
            {
                if (String.Compare(patName, pPattern.name, true) == 0)
                {
                    return pPattern;
                }
            }
            if (m_PatFile != null)
            {
                var pPat = new Pattern();
                if (pPat.LoadPattern(this, patName))
                {
                    m_Patterns.Add(pPat);
                    return pPat;
                }
            }
            return null;
        }

        internal DXF.ifstream m_PatFile = null;
        internal List<Pattern> m_Patterns = new List<Pattern>();
    }
}
