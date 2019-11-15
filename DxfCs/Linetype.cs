using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace DXF
{
    public class Linetype_Descriptor
    {
        public Linetype_Descriptor()
        {
            type = 0;
            scale_length = 1.0;
            bRelative = true;
            rotate = 0;
            xoffset = 0;
            yoffset = 0;

        }
        public int type; //0 -- simple, 1--shape, 2--text

        public string shapename_text;
        public string shxfilename_style;
        public double scale_length;
        public bool bRelative;
        public double rotate; //in radians
        public double xoffset;
        public double yoffset;
    }

    public class Linetype
    {
        public bool LoadLinetype(LinetypeManager manager, string lineTypeName)
        {
            bool bFound = false;
            //Global.ASSERT_DEBUG_INFO(manager.m_LinFile);
            DXF.ifstream s_LinFile = manager.m_LinFile;
            //Global.ASSERT_DEBUG_INFO(s_LinFile.is_open());
            s_LinFile.reset();
            s_LinFile.setdelimiter(",");
            string buffer = null;
            while (!s_LinFile.eof())
            {
                s_LinFile.read(out buffer);
                if (buffer[0] == ';')
                {
                    s_LinFile.ignore(Int32.MaxValue, '\n');
                }
                else if (buffer[0] == '*')
                {
                    line_name = buffer.Substring(1);
                    if (string.Compare(line_name, lineTypeName, true) == 0)
                    {
                        bFound = true;
                    }
                    s_LinFile.ignore(Int32.MaxValue, ',');
                    s_LinFile.getline(out buffer, 128);
                    if (bFound)
                    {
                        line_descrip = buffer;
                    }
                }
                else
                {
                    if (!bFound)
                    {
                        s_LinFile.ignore(Int32.MaxValue, '\n');
                        continue;
                    }

                    char onechar = '\0';
                    s_LinFile.read(out onechar);
                    while (onechar == ',')
                    {
                        Linetype_Descriptor lts = new Linetype_Descriptor();
                        s_LinFile.read(out buffer);
                        if (buffer[0] == '[')
                        {
                            char anotherchar = '\0';
                            lts.shapename_text = buffer.Substring(1);
                            s_LinFile.skipws();
                            s_LinFile.read(out anotherchar);
                            s_LinFile.read(out buffer);
                            s_LinFile.skipws();
                            s_LinFile.read(out onechar);
                            lts.shxfilename_style = buffer;
                            if (lts.shapename_text.Contains(".shx"))
                            {
                                lts.type = 1; //shape linetype
                            }
                            else
                            {
                                lts.type = 2; //text linetype
                            }

                            do
                            {
                                long streampos1 = s_LinFile.tellg();
                                s_LinFile.ignore(Int32.MaxValue, ",]");
                                long streampos2 = s_LinFile.tellg();
                                s_LinFile.seekg(streampos1);
                                s_LinFile.read(out buffer, (int)(streampos2 - streampos1));
                                s_LinFile.read(out anotherchar);
                                string[] tokens = buffer.Split(new char[]{'='}, 2);
                                string transformtype = tokens[0];
                                transformtype = Global.TrimSpace(transformtype);
                                char code = transformtype[0];
                                string pValue = tokens[1];
                                int end = 0;
                                double value = StringToDouble.Parse(pValue, 0, out end);
                                char EndChar = '\0';
                                if (end < pValue.Length)
                                    EndChar = pValue[end];
                                Func<bool> ToRadian = () => {
                                    switch (EndChar)
                                    {
                                        case '\0':
                                        case 'd':
                                        case 'D':
                                            value *= Math.PI / 180.0;
                                            return true;
                                            //break;
                                        case 'r':
                                        case 'R':
                                            return true;
                                            //break;
                                        case 'g':
                                        case 'G':
                                            value = Math.Atan(value / 100);
                                            return true;
                                            //break;
                                        default:
                                            Global.ASSERT_DEBUG_INFO("false", false);
                                            return false;
                                            //break;
                                    };
                                };
                                switch (code)
                                {
                                    case 's':
                                    case 'S':
                                        lts.scale_length = value;
                                        break;
                                    case 'a':
                                    case 'A':
                                        lts.bRelative = false;
                                        ToRadian();
                                        break;
                                    case 'r':
                                    case 'R':
                                        {
                                            ToRadian();
                                            lts.rotate = value;
                                        }
                                        break;
                                    case 'x':
                                    case 'X':
                                        lts.xoffset = value;
                                        break;
                                    case 'y':
                                    case 'Y':
                                        lts.yoffset = value;
                                        break;
                                    default:
                                        Global.ASSERT_DEBUG_INFO("false", false);
                                        break;
                                };
                            } while (anotherchar == ',');
                        }
                        else
                        {
                            double temp = double.Parse(buffer);
                            lts.scale_length = temp;
                        }
                        descriptors.Add(lts);
                        s_LinFile.skipws();
                        s_LinFile.read(out onechar);
                    }
                    return true;
                }
            }

            return false;
        }
        public string line_name;
        public string line_descrip;
        public List<Linetype_Descriptor> descriptors = new List<Linetype_Descriptor>();
    }
    public class LinetypeManager
    {
        public void SetLinFile(string pLinFile)
        {
            if (m_LinFile != null)
                m_LinFile.Close();
            m_LinFile = new ifstream(new FileStream(pLinFile, FileMode.Open, FileAccess.Read));
        }

        public bool IsValid()
        {
            return m_LinFile != null;
        }

        public Linetype FindLinetype(string lineTypeName)
        {
            foreach (var pLt in m_Linetypes)
            {
                if (String.Compare(lineTypeName, pLt.line_name, true) == 0)
                {
                    return pLt;
                }
            }
            if (m_LinFile != null)
            {
                var plt = new Linetype();
                if (plt.LoadLinetype(this, lineTypeName))
                {
                    m_Linetypes.Add(plt);
                    return plt;
                }
            }
            return null;
        }

        //	friend class Linetype;
        internal DXF.ifstream m_LinFile = null;
        internal List<Linetype> m_Linetypes = new List<Linetype>();
    }
}