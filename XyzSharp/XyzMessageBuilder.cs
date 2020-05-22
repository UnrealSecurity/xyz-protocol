﻿using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

public class XyzMessageBuilder
{
    MemoryStream data = new MemoryStream();

    public XyzMessageBuilder()
    {

    }

    private byte[] Copy(byte[] from, byte[] to)
    {
        for (int i = 0; i < to.Length; i++)
        {
            if (i == from.Length) break;
            to[i] = from[i];
        }
        return to;
    }

    public byte[] ToArray()
    {
        return data.ToArray();
    }

    public XyzMessageBuilder Add(byte[] message, int type = 0)
    {
        MemoryStream stream = new MemoryStream();

        byte[] lenBytes = BitConverter.GetBytes(message.Length);
        stream.Write(lenBytes, 0, lenBytes.Length);

        byte[] typeBytes = new byte[] { (byte)type };
        stream.Write(typeBytes, 0, typeBytes.Length);

        stream.Write(message, 0, message.Length);

        byte[] bytes = stream.ToArray();
        data.Write(bytes, 0, bytes.Length);
        stream.Dispose();
        return this;
    }

    public XyzMessageBuilder Add(string message, int type = 0)
    {
        Add(Encoding.UTF8.GetBytes(message), type);
        return this;
    }

    public XyzMessageBuilder Add(XyzMessage message)
    {
        Add(message.Bytes, message.Type);
        return this;
    }
}