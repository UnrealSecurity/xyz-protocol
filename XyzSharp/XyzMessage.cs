using System;
using System.Collections.Generic;
using System.Text;

namespace XyzSharp
{
    public class XyzMessage
    {
        private byte[] data;
        private int type = 0;

        public string String { get { return Encoding.UTF8.GetString(data); } }
        public byte[] Bytes { get { return data; } }
        public int Type { get { return type; } }


        public XyzMessage(byte[] message, int type = 0)
        {
            data = message;
        }
    }
}