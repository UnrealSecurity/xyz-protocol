using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

class XyzClient
{
    private TcpClient client;
    private XyzSession session;

    private Action<XyzSession> on_Connect = null;
    public Action<XyzSession> OnConnect { get { return on_Connect; } set { on_Connect = value; } }

    private Action<XyzSession> on_Disconnect = null;
    public Action<XyzSession> OnDisconnect { get { return on_Disconnect; } set { on_Disconnect = value; } }

    private Action<byte[], int> on_Message = null;
    public Action<byte[], int> OnMessage { get { return on_Message; } set { on_Message = value; } }


    public XyzClient()
    {
        this.client = new TcpClient();
    }

    public XyzClient(string host, int port)
    {
        this.client = new TcpClient();
        this.Connect(host, port);
    }

    public XyzClient(TcpClient client)
    {
        this.client = client;
    }

    public bool Connect(string host, int port)
    {
		try
		{
            this.client.Connect(host, port);

            this.session = new XyzSession(this.client)
            {
                OnConnect = this.on_Connect,
                OnDisconnect = this.on_Disconnect,
                OnMessage = this.on_Message,
            };

            this.OnConnect?.Invoke(this.session);
            return true;
		}
		catch (Exception)
		{
            this.OnDisconnect?.Invoke(this.session);
        }
        return false;
    }

    public void Send(byte[] data, int type = 0)
    {
        this.session.Send(data, type);
    }

    public void Send(string data, int type = 0)
    {
        this.session.Send(Encoding.UTF8.GetBytes(data), type);
    }

    public void Send(XyzMessage[] messages, int type = 0)
    {
        XyzMessageBuilder builder = new XyzMessageBuilder();
        foreach (XyzMessage message in messages)
        {
            builder.Add(message);
        }
        this.Send(builder.ToArray(), type);
    }

    public void Send(XyzMessage message, int type = 0)
    {
        byte[] payload = new XyzMessageBuilder().Add(message).ToArray();
        this.Send(payload, type);
    }
}