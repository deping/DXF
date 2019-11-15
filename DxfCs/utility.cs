using System;
using System.IO;
using System.Threading;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace DXF
{
 
    public class ThreadDebugInfo
    {
        // ofstream can not be copied.
        public StreamWriter m_DebugInfo;
        public string m_FileName;
        public bool m_Enabled;
    }

    public class DxfException : Exception
    {
        public DxfException(string message) : base(message)
        {
        }
    }

    public partial class Global
    {

        private static SortedDictionary<int, ThreadDebugInfo> g_ThreadDebugInfos = new SortedDictionary<int, ThreadDebugInfo>();

        public static void SetThreadDebugInfo(string fileName)
        {
            ThreadDebugInfo value = null;
            bool hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out value);
            if (!hasIt)
            {
                value = new ThreadDebugInfo();
                g_ThreadDebugInfos[Thread.CurrentThread.ManagedThreadId] = value;
                value.m_Enabled = true;
            }
            if (value.m_DebugInfo != null)
                value.m_DebugInfo.Close();
            value.m_DebugInfo = new StreamWriter(fileName);
            value.m_FileName = fileName;
        }

        public static string GetThreadDebugInfoFileName()
        {
            ThreadDebugInfo value = null;
            bool hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out value);
            if (hasIt)
            {
                return value.m_FileName;
            }
            return "DxfDebugInfo-" + Thread.CurrentThread.ManagedThreadId + ".log";
        }

        public static void EnableDebugInfo(bool value)
        {
            ThreadDebugInfo tdi = null;
            bool hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out tdi);
            if (hasIt)
            {
                tdi.m_Enabled = value;
            }
            else
            {
                SetThreadDebugInfo(GetThreadDebugInfoFileName());
                hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out tdi);
                if (hasIt)
                {
                    tdi.m_Enabled = value;
                }
            }
        }

        public static StreamWriter GetThreadDebugInfoStream()
        {
            ThreadDebugInfo tdi = null;
            bool hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out tdi);
            if (!hasIt)
            {
                SetThreadDebugInfo(GetThreadDebugInfoFileName());
            }
            hasIt = g_ThreadDebugInfos.TryGetValue(Thread.CurrentThread.ManagedThreadId, out tdi);
            if (hasIt)
            {
                if (tdi.m_Enabled)
                {
                    return tdi.m_DebugInfo;
                }
           }
            return null;
        }
        public static void PrintDebugInfo(string szMsg, params object[] args)
        {
            var debugInfo = GetThreadDebugInfoStream();
            if (debugInfo == null)
            {
                return;
            }
            string msg = String.Format(szMsg, args);
            debugInfo.WriteLine(msg);
            debugInfo.Flush();
        }

        public static void PrintDebugInfo(string fileName, int lineNo, string funcName, string szMsg, params object[] args)
        {
            var debugInfo = GetThreadDebugInfoStream();
            if (debugInfo == null)
            {
                return;
            }
            debugInfo.Write(fileName);
            debugInfo.Write(":");
            debugInfo.Write(lineNo);
            debugInfo.Write("@");
            debugInfo.Write(funcName);
            string msg = String.Format(szMsg, args);
            debugInfo.WriteLine(msg);
            debugInfo.Flush();
        }

        [Conditional("DEBUG")]
        public static void PRINT_DEBUG_INFO(string szMsg, params object[] args)
        {
            StackTrace trace = new StackTrace(true);
            StackFrame frame = trace.GetFrame(1);
            PrintDebugInfo(frame.GetFileName(), frame.GetFileLineNumber(), frame.GetMethod().Name, szMsg, args);
        }

       [Conditional("DEBUG")]
       public static void ASSERT_DEBUG_INFO(string boolExp, bool boolExpVal)
        {
            if (!(boolExpVal)) {
                StackTrace trace = new StackTrace(true);
                StackFrame frame = trace.GetFrame(1);
                PrintDebugInfo(frame.GetFileName(), frame.GetFileLineNumber(), frame.GetMethod().Name, "Assertion failed:{0}", boolExp);
		        throw new DxfException("Program error!");
            }
        }

       [Conditional("DEBUG")]
        public static void THROW_INVALID_DXF()
        {
            StackTrace trace = new StackTrace(true);
            StackFrame frame = trace.GetFrame(1);
            PrintDebugInfo(frame.GetFileName(), frame.GetFileLineNumber(), frame.GetMethod().Name, "Invalid DXF file.");
            throw new DxfException("Invalid DXF file.");
        }

        public static string right(string src, int num)
        {
            if (src.Length <= num)
            {
                return src;
            }
            return src.Substring(src.Length - num);
        }

        public static bool RightCompareNoCase(string str1, int num, string str2)
        {
            return right(str1, num).ToLower() == str2.ToLower();
        }

        public static string TrimLeftSpace(string str)
        {
            return str.TrimStart(new char[]{ ' ', '\t', '\r', '\n' });
        }

        public static string TrimRightSpace(string str)
        {
            return str.TrimEnd(new char[] { ' ', '\t', '\r', '\n' });
        }

        public static string TrimSpace(string str)
        {
            return str.Trim(new char[] { ' ', '\t', '\r', '\n' });
        }

        public static int AdvanceUtf8(string str, int start, int bytes)
        {
            Global.ASSERT_DEBUG_INFO(String.Format("{0} != null && {1} >=0 && {2} > 0", str, start, bytes), str != null && start >= 0 && bytes > 0);
            if (start == str.Length)
                return str.Length;
            char[] chars = str.ToCharArray();
            int cur = start;
            int byteCount = 0;
            while (cur < str.Length)
            {
                UTF8Encoding utf8 = new UTF8Encoding();
                byteCount += utf8.GetByteCount(chars, cur, 1);
                if (byteCount > bytes)
                {
                    return cur;
                }
                cur += 1;
            }
            return str.Length;
        }
        public static int hextoi(string src)
        {
            int value = int.Parse(src, System.Globalization.NumberStyles.HexNumber);
            return value;
        }
    }

#if DEBUG
    public class MethodLogger : IDisposable
    {
        public MethodLogger(string szMsg, params object[] args)
        {
            StackTrace trace = new StackTrace(true);
            StackFrame frame = trace.GetFrame(1);
            Global.PrintDebugInfo(frame.GetFileName(), frame.GetFileLineNumber(), frame.GetMethod().Name, szMsg, args);
        }

        public void Dispose()
        {
            StackTrace trace = new StackTrace(true);
            StackFrame frame = trace.GetFrame(1);
            Global.PrintDebugInfo(frame.GetFileName(), 0, frame.GetMethod().Name, "Exit");
        }
    }
#endif


    public class CDblPoint
    {
        public CDblPoint()
        {
            x = 0.0;
            y = 0.0;
        }
        public CDblPoint(double X, double Y)
        {
            x = X;
            y = Y;
        }
        public CDblPoint(CDblPoint Point)
        {
            x = Point.x;
            y = Point.y;
        }
        public void SetPoint(double u, double v)
        {
            x = u;
            y = v;
        }
        public void SetPoint(CDblPoint Point)
        {
            x = Point.x;
            y = Point.y;
        }
        //绕this点把point点旋转angle弧度到新的位置
        public void Rotate(ref CDblPoint point, double angle)
        {
            double detX = point.x - x;
            double detY = point.y - y;
            double cosAngle = Math.Cos(angle);
            double sinAngle = Math.Sin(angle);
            double detX2 = detX * cosAngle - detY * sinAngle;
            double detY2 = detX * sinAngle + detY * cosAngle;
            point.x = x + detX2;
            point.y = y + detY2;
        }
        public double x;
        public double y;
    }
}
