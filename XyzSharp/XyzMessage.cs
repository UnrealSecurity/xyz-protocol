using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace XyzSharp
{
    public class XyzMessage
    {
        private byte[] data;
        public byte[] Bytes { get { return data; } }
        private int type = 0;
        public int Type { get { return type; } }

        public int Int { get { return BitConverter.ToInt32(data, 0); } }
        public long Long { get { return BitConverter.ToInt64(data, 0); } }
        public string String { get { return Encoding.UTF8.GetString(data); } }
        public string[] Strings { get { return Encoding.UTF8.GetString(data).Split('\0'); } }

        public bool[] Booleans { 
            get {
                bool[] list = new bool[this.data.Length];
                for (int i=0; i<list.Length; i++)
                {
                    list[i] = (int)this.data[i] == 1 ? true : false;
                }
                return list;
            } 
        }

        public int[] Ints
        {
            get
            {
                int[] list = new int[this.data.Length/4];
                int j = 0;
                for (int i = 0; i < list.Length; i++)
                {
                    list[i] = BitConverter.ToInt32(this.data, j);
                    j += 4;
                }
                return list;
            }
        }

        public long[] Longs
        {
            get
            {
                long[] list = new long[this.data.Length / 8];
                int j = 0;
                for (int i = 0; i < list.Length; i++)
                {
                    list[i] = BitConverter.ToInt64(this.data, j);
                    j += 8;
                }
                return list;
            }
        }

        public XyzMessage(byte[] message, int type = 0)
        {
            this.data = message;
            this.type = type;
        }

        public XyzMessage(bool[] message, int type = 0)
        {
            MemoryStream ms = new MemoryStream();
            foreach (bool value in message)
            {
                ms.Write(new byte[] { (byte)(value ? 1 : 0) }, 0, 1);
            }
            this.data = ms.ToArray();
            this.type = type;
        }

        public XyzMessage(int message, int type = 0)
        {
            this.data = BitConverter.GetBytes(message);
            this.type = type;
        }

        public XyzMessage(long message, int type = 0)
        {
            this.data = BitConverter.GetBytes(message);
            this.type = type;
        }

        public XyzMessage(string message, int type = 0)
        {
            this.data = Encoding.UTF8.GetBytes(message);
            this.type = type;
        }

        public XyzMessage(int[] message, int type = 0)
        {
            MemoryStream ms = new MemoryStream();
            foreach (int value in message)
            {
                byte[] bytes = BitConverter.GetBytes(value);
                ms.Write(bytes, 0, bytes.Length);
            }
            this.data = ms.ToArray();
            this.type = type;
        }

        public XyzMessage(long[] message, int type = 0)
        {
            MemoryStream ms = new MemoryStream();
            foreach (long value in message)
            {
                byte[] bytes = BitConverter.GetBytes(value);
                ms.Write(bytes, 0, bytes.Length);
            }
            this.data = ms.ToArray();
            this.type = type;
        }
    }
}
