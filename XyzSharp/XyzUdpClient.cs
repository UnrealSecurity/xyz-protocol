using System;
using System.Net;
using System.Text;
using System.Net.Sockets;
using System.IO;

namespace XyzSharp
{
    class XyzUdpClient
    {
        UdpClient client;
        public UdpClient Client { get { return this.client; } }

        private Action<IPEndPoint, byte[], int> on_Message = null;
        public Action<IPEndPoint, byte[], int> OnMessage { get { return on_Message; } set { on_Message = value; } }

        public XyzUdpClient()
        {
            client = new UdpClient();
        }

        public XyzUdpClient(IPEndPoint bindTo)
        {
            client = new UdpClient();
            this.Bind(bindTo);
            client.BeginReceive(DataReceived, client);
        }

        public XyzUdpClient(string address, int port)
        {
            client = new UdpClient();
            this.Bind(address, port);
            client.BeginReceive(DataReceived, client);
        }

        public XyzUdpClient(IPAddress address, int port)
        {
            client = new UdpClient();
            this.Bind(address, port);
            client.BeginReceive(DataReceived, client);
        }

        public void Bind(IPEndPoint bindTo)
        {
            client.Client.Bind(bindTo);
        }

        public void Bind(string address, int port)
        {
            client.Client.Bind(new IPEndPoint(IPAddress.Parse(address), port));
        }

        public void Bind(IPAddress address, int port)
        {
            client.Client.Bind(new IPEndPoint(address, port));
        }

        public void Close()
        {
            client.Close();
        }

        private void DataReceived(IAsyncResult ar)
        {
            IPEndPoint endpoint = null;
            byte[] bytes = client.EndReceive(ar, ref endpoint);
            XyzMessage[] messages = new XyzMessageDecoder(bytes).Decode();

            client.BeginReceive(DataReceived, client);

            on_Message?.Invoke(endpoint, XyzUtils.Inflate(messages[0].Bytes), messages[0].Type);
        }

        public void Send(IPEndPoint sendTo, byte[] data, int type = 0)
        {
            try
            {
                byte[] payload = new XyzMessageBuilder().Add(XyzUtils.Deflate(data), type).ToArray();
                this.client.BeginSend(payload, payload.Length, sendTo, null, this.client);
            }
            catch (Exception)
            {
            }
        }

        public void Send(IPEndPoint sendTo, string data, int type = 0)
        {
            this.Send(sendTo, Encoding.UTF8.GetBytes(data), type);
        }

        public void Send(IPEndPoint sendTo, XyzMessage[] messages, int type = 0)
        {
            XyzMessageBuilder builder = new XyzMessageBuilder();
            foreach (XyzMessage message in messages)
            {
                builder.Add(message);
            }
            this.Send(sendTo, builder.ToArray(), type);
        }

        public void Send(IPEndPoint sendTo, XyzMessage message, int type = 0)
        {
            byte[] payload = new XyzMessageBuilder().Add(message).ToArray();
            this.Send(sendTo, payload, type);
        }

        public void SendSync(IPEndPoint sendTo, byte[] data, int type = 0)
        {
            try
            {
                byte[] payload = new XyzMessageBuilder().Add(XyzUtils.Deflate(data), type).ToArray();
                this.client.Send(payload, payload.Length, sendTo);
            }
            catch (Exception)
            {
            }
        }

        public void SendSync(IPEndPoint sendTo, string data, int type = 0)
        {
            this.SendSync(sendTo, Encoding.UTF8.GetBytes(data), type);
        }

        public void SendSync(IPEndPoint sendTo, XyzMessage[] messages, int type = 0)
        {
            XyzMessageBuilder builder = new XyzMessageBuilder();
            foreach (XyzMessage message in messages)
            {
                builder.Add(message);
            }
            this.SendSync(sendTo, builder.ToArray(), type);
        }

        public void SendSync(IPEndPoint sendTo, XyzMessage message, int type = 0)
        {
            byte[] payload = new XyzMessageBuilder().Add(message).ToArray();
            this.SendSync(sendTo, payload, type);
        }
    }
}
