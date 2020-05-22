using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;

class XyzSession
{
    XyzClient client;
    NetworkStream stream;
    byte[] buffer = new byte[4];
    State state = State.Length;

    int length = 0;
    int type = 0;

    public XyzClient Client { get { return this.client; } }

    private Action<XyzClient> on_Connect = null;
    public Action<XyzClient> OnConnect { get { return on_Connect; } set { on_Connect = value; } }

    private Action<XyzClient> on_Disconnect = null;
    public Action<XyzClient> OnDisconnect { get { return on_Disconnect; } set { on_Disconnect = value; } }

    private Action<byte[], int> on_Message = null;
    public Action<byte[], int> OnMessage { get { return on_Message; } set { on_Message = value; } }


    public XyzSession(XyzClient client)
    {
        this.client = client;
        this.stream = client.Client.GetStream();

        this.stream.BeginRead(this.buffer, 0, 4, DataReceived, this.client);
    }

    private void DataReceived(IAsyncResult ar)
    {
        try
        {
            int read = this.stream.EndRead(ar);

            if (this.state == State.Length)
            {
                this.state = State.Type;

                this.length = BitConverter.ToInt32(this.buffer, 0);
                this.buffer = new byte[1];
                this.stream.BeginRead(this.buffer, 0, 1, DataReceived, this.client);
            }
            else if (this.state == State.Type)
            {
                this.state = State.Message;
                this.type = (int)this.buffer[0];
                this.buffer = new byte[this.length];
                this.stream.BeginRead(this.buffer, 0, this.buffer.Length, DataReceived, this.client);
            }
            else if (this.state == State.Message)
            {
                this.state = State.Length;

                byte[] message = XyzUtils.Inflate(buffer);
                this.OnMessage?.Invoke(message, this.type);
                this.buffer = new byte[4];
                this.stream.BeginRead(this.buffer, 0, 4, DataReceived, this.client);
            }
        }
        catch (Exception)
        {
            this.state = State.Length;
            this.client.Disconnect(true);
        }
    }

    public void Send(byte[] data, int type = 0)
    {
        byte[] payload = new XyzMessageBuilder().Add(XyzUtils.Deflate(data), type).ToArray();
        this.stream.BeginWrite(payload, 0, payload.Length, null, this.client);
    }

    public void Send(string data, int type = 0)
    {
        this.Send(Encoding.UTF8.GetBytes(data), type);
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

    private enum State
    {
        Length,
        Type,
        Message,
    }
}