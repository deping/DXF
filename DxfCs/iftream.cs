using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;

namespace DXF
{
    public static class StreamReaderExtensions
    {
        readonly static FieldInfo charPosField = typeof(StreamReader).GetField("charPos", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance | BindingFlags.DeclaredOnly);
        readonly static FieldInfo byteLenField = typeof(StreamReader).GetField("byteLen", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance | BindingFlags.DeclaredOnly);
        readonly static FieldInfo charBufferField = typeof(StreamReader).GetField("charBuffer", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance | BindingFlags.DeclaredOnly);

        public static long GetPosition(this StreamReader reader)
        {
            //shift position back from BaseStream.Position by the number of bytes read
            //into internal buffer.
            int byteLen = (int)byteLenField.GetValue(reader);
            var position = reader.BaseStream.Position - byteLen;

            //if we have consumed chars from the buffer we need to calculate how many
            //bytes they represent in the current encoding and add that to the position.
            int charPos = (int)charPosField.GetValue(reader);
            if (charPos > 0)
            {
                var charBuffer = (char[])charBufferField.GetValue(reader);
                var encoding = reader.CurrentEncoding;
                var bytesConsumed = encoding.GetBytes(charBuffer, 0, charPos).Length;
                position += bytesConsumed;
            }

            return position;
        }

        public static void SetPosition(this StreamReader reader, long position)
        {
            reader.DiscardBufferedData();
            reader.BaseStream.Seek(position, SeekOrigin.Begin);
        }
    }
    
    //所有影响流指针的操作，除了seekg，如果流状态失败(fail()==true)，都会立即返回。
    public class ifstream : StreamReader
    {
        private char[] m_delimiter;
        public ifstream(Stream file)
            : base(file, Encoding.UTF8)
        {
        }

        public void reset()
        {
            seekg(0);
        }

        public bool eof()
        {
            return tellg() >= BaseStream.Length;
        }
        public ifstream read(out string Str)
        {
            Str = getfld();
            return (this);
        }
        public ifstream read(out char Val)
        {
            int c = Read();
            Val = (char)c;
            return (this);
        }
        public ifstream read(out short Val)
        {
            long temp = 0;
            do_get(out temp);
            Val = (short)temp;
            return (this);
        }
        public ifstream read(out ushort Val)
        {
            long temp = 0;
            do_get(out temp);
            Val = (ushort)temp;
            return (this);
        }
        public ifstream read(out int Val)
        {
            long temp = 0;
            do_get(out temp);
            Val = (int)temp;
            return (this);
        }
        public ifstream read(out uint Val)
        {
            long temp = 0;
            do_get(out temp);
            Val = (uint)temp;
            return (this);
        }
        public ifstream read(out long Val)
        {
            do_get(out Val);
            return (this);
        }
        public ifstream read(out ulong Val)
        {
            long temp = 0;
            do_get(out temp);
            Val = (uint)temp;
            return (this);
        }
        public ifstream read(out float Val)
        {
            double temp = 0;
            do_get(out temp);
            Val = (float)temp;
            return (this);
        }
        public ifstream read(out double Val)
        {
            do_get(out Val);
            return (this);
        }
        public ifstream read(out bool Val)
        {
            string[] boolname = { "false", "true" };
            string buffer = getfld();
            for (int i = 0; i < 2; ++i)
            {
                if (string.Compare(buffer, boolname[i], true) == 0)
                {
                    Val = i != 0;
                    return (this);
                }
            }
            long temp = long.Parse(buffer);
            Val = temp != 0;
            return (this);
        }

        public void setdelimiter(string delimiter)
        {
            if (String.IsNullOrEmpty(delimiter))
                m_delimiter = null;
            else
                m_delimiter = delimiter.ToArray<char>();
        }

        public ifstream skipws()
        {
            while (!eof())
            {
                int c = Peek();
                if (Char.IsWhiteSpace((char)c))//不可使用isspace，否则文本文件有中文字符时，导致调试时断言失败
                {
                    Read();
                }
                else
                    break;
            }
            return this;

        }
        public bool isdelim(char code)
        {
            if (Char.IsWhiteSpace(code))//不可使用isspace，否则文本文件有中文字符时，导致调试时断言失败
                return true;
            if (m_delimiter != null)
            {
                foreach (var delimiter in m_delimiter)
                {
                    if (code == delimiter)
                        return true;
                }
            }
            return false;
        }

        public long tellg()
        {
            return StreamReaderExtensions.GetPosition(this);
        }

        public ifstream seekg(long pos)
	    {

            StreamReaderExtensions.SetPosition(this, pos);
		    return this;
	    }
        public long getsize()
        {
            return BaseStream.Length;
        }
        public ifstream getline(out string Str, int count)
        {
            int i = 0;
            StringBuilder sb = new StringBuilder();
            while (i < count && !eof())
            {
                char c = (char)Read();
                if (c != '\n' && c != '\r')
                {
                    sb.Append(c);
                }
                else
                {
                    c = (char)Peek();
                    if (c == '\n' || c == '\r')
                    {
                        Read();
                    }
                    break;
                }
                ++i;
            }
            Str = sb.ToString();
            return this;
        }
        public ifstream read(out string Str, int count)
        {
            int i = 0;
            StringBuilder sb = new StringBuilder();
            while (i < count && !eof())
            {
                char c = (char)Read();
                sb.Append(c);
                ++i;
            }
            Str = sb.ToString();
            return this;
        }
        //当count等于numeric_limits<unsigned int>::max()时表示只考虑stop,不考虑count。
        //流的当前指针前进count个字节。如果在这之前找到stop字符，那么流的当前指针停在stop
        //字符之后。
        public ifstream ignore(int count, char stop)
        {
            int i = 0;
            while(i < count && !eof())
            {
                char code = (char)Read();
                if (code == stop)
                {
                    break;
                }
            }
            return this;
        }
        //同上。不同的是可以传入多个stop字符。
        public ifstream ignore(uint count, string stop)
        {
            int len = stop.Length;
            int i = 0;
            while (i < count && !eof())
            {
                char code = (char)Read();
                for (int j = 0; j < len; ++j)
                {
                    if (code == stop[j])
                    {
                        goto exit;
                    }
                }
            }
        exit:
            return this;
        }
        //流的当前指针回退count个字节。如果退到文件开始之前，failbit将被设置，
        //同时，流的当前指针指向文件开始处。
        ifstream backward(int count)
        {
            long pos = tellg();
            if (pos > count)
                StreamReaderExtensions.SetPosition(this, pos - count);
            else
                StreamReaderExtensions.SetPosition(this, 0);
            return this;
        }
        // get field from file at current read position into buffer
        private string getfld()
        {
            skipws();
            StringBuilder sb = new StringBuilder();
            while (!eof())
            {
                char c = (char)Peek();
                if (isdelim(c))
                    break;
                sb.Append(c);
                Read();
            }
            return sb.ToString();
        }

        private void do_get(out long val)
        {
            string buffer = getfld();
            val = long.Parse(buffer);
        }
        private void do_get(out double val)
        {
            string buffer = getfld();
            val = double.Parse(buffer);
        }
    }
}
