using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

public class XyzMessageDecoder
{
    byte[] message;

    public XyzMessageDecoder(byte[] message)
    {
        this.message = message;
    }

    public XyzMessage[] Decode()
    {
        List<XyzMessage> messages = new List<XyzMessage>();
        MemoryStream stream = new MemoryStream(message);

        while (stream.Position < stream.Length)
        {
            byte[] lenBytes = new byte[4];
            stream.Read(lenBytes, 0, lenBytes.Length);
            int length = BitConverter.ToInt32(lenBytes, 0);

            byte[] typeBytes = new byte[1];
            stream.Read(typeBytes, 0, typeBytes.Length);
            int type = (int)typeBytes[0];

            byte[] messageBytes = new byte[length];
            stream.Read(messageBytes, 0, messageBytes.Length);

            XyzMessage message = new XyzMessage(messageBytes, type);

            messages.Add(message);
        }

        return messages.ToArray();
    }
}