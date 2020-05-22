using System;
using System.Collections.Generic;
using System.Text;

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
        public bool[] Booleans { get {
                bool[] list = new bool[this.data.Length];
                for (int i=0; i<list.Length; i++)
                {
                    list[i] = (int)this.data[i] == 1 ? true : false;
                }
                return list;
            } }

        public XyzMessage(byte[] message, int type = 0)
        {
            data = message;
        }
    }
}
