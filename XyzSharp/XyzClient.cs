using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace XyzSharp
{
    public class XyzClient
    {
        private TcpClient client;
        private bool disconnected = false;
        NetworkStream stream;

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
        }

        public XyzClient(string host, int port)
        {
            this.client = new TcpClient();
            this.Connect(host, port);
        }

        public XyzClient(TcpClient client)
        {
            this.client = client;
            this.stream = client.GetStream();
        }

        public void Read()
        {
            new Thread(Receiver).Start();
        }

        public byte[] ReadBytes(int length)
        {
            byte[] bytes = new byte[length];
            int received = 0;

            while (received < length)
            {
                int read = this.stream.Read(bytes, received, length - received);
                received += read;
            }

            return bytes;
        }

        public void Receiver()
        {
            while (client.Connected)
            {
                byte[] header = ReadBytes(5);

                int length = BitConverter.ToInt32(header, 0);
                int type = header[4];

                byte[] message_bytes = ReadBytes(length);

                this.OnMessage?.Invoke(XyzUtils.Inflate(message_bytes), type);
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

                new Thread(Receiver).Start();

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