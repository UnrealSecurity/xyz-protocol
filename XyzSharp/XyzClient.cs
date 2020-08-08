using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace XyzSharp
{
    public class XyzClient
    {
        private TcpClient client;
        private bool disconnected = false;
        NetworkStream stream;
        byte[] buffer = new byte[4];

        private enum State
        {
            Length,
            Type,
            Message,
        }

        State state = State.Length;
        int length = 0;
        int type = 0;

        public TcpClient Client { get { return this.client; } }

        private Action on_Connect = null;
        public Action OnConnect { get { return on_Connect; } set { on_Connect = value; } }

        private Action on_Disconnect = null;
        public Action OnDisconnect { get { return on_Disconnect; } set { on_Disconnect = value; } }

        private Action<byte[], int> on_Message = null;
        public Action<byte[], int> OnMessage { get { return on_Message; } set { on_Message = value; } }

        public XyzClient()
        {
            this.client = new TcpClient();
            this.client.NoDelay = true;
        }

        public XyzClient(string host, int port)
        {
            this.client = new TcpClient();
            this.client.NoDelay = true;
            this.Connect(host, port);
        }

        public XyzClient(TcpClient client)
        {
            client.NoDelay = true;
            this.client = client;
            this.stream = client.GetStream();
        }

        public void Read()
        {
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
                this.Disconnect(true);
            }
        }

        public void Disconnect(bool forced = false)
        {
            if (forced || this.client.Connected)
            {
                if (disconnected)
                {
                    return;
                }
                else
                {
                    disconnected = true;
                }

                this.client.Client.Close();
                this.on_Disconnect?.Invoke();
            }
        }

        public bool Connect(string host, int port)
        {
            try
            {
                this.client.Connect(host, port);

                this.stream = client.GetStream();
                this.stream.BeginRead(this.buffer, 0, 4, DataReceived, this.client);

                this.OnConnect?.Invoke();

                return true;
            }
            catch (Exception)
            {
                this.OnDisconnect?.Invoke();
            }
            return false;
        }

        public bool Connected()
        {
            bool a = this.client.Client.Poll(1000, SelectMode.SelectRead);
            bool b = this.client.Client.Available == 0;

            if (a && b)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        public void Send(byte[] data, int type = 0)
        {
            try
            {
                byte[] payload = new XyzMessageBuilder().Add(XyzUtils.Deflate(data), type).ToArray();
                this.stream.BeginWrite(payload, 0, payload.Length, null, this.client);
            }
            catch (Exception)
            {
            }
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

        public void SendSync(byte[] data, int type = 0)
        {
            try
            {
                byte[] payload = new XyzMessageBuilder().Add(XyzUtils.Deflate(data), type).ToArray();
                this.stream.Write(payload, 0, payload.Length);
            }
            catch (Exception)
            {
            }
        }

        public void SendSync(string data, int type = 0)
        {
            this.SendSync(Encoding.UTF8.GetBytes(data), type);
        }

        public void SendSync(XyzMessage[] messages, int type = 0)
        {
            XyzMessageBuilder builder = new XyzMessageBuilder();
            foreach (XyzMessage message in messages)
            {
                builder.Add(message);
            }
            this.SendSync(builder.ToArray(), type);
        }

        public void SendSync(XyzMessage message, int type = 0)
        {
            byte[] payload = new XyzMessageBuilder().Add(message).ToArray();
            this.SendSync(payload, type);
        }
    }
}