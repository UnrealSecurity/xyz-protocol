using System;
using System.Collections.Generic;
using System.Text;

namespace XyzSharp
{
    public class XyzProxyTunnel
    {
        private XyzServer server = null;

        private int listenPort;
        private string forwardHost;
        private int forwardPort;

        private Action<XyzClient, Source> on_Connect = null;
        public Action<XyzClient, Source> OnConnect { get { return on_Connect; } set { on_Connect = value; } }

        private Action<XyzClient, Source> on_Disconnect = null;
        public Action<XyzClient, Source> OnDisconnect { get { return on_Disconnect; } set { on_Disconnect = value; } }

        private Action<XyzClient, byte[], int, Source> on_Message = null;
        public Action<XyzClient, byte[], int, Source> OnMessage { get { return on_Message; } set { on_Message = value; } }

        public XyzProxyTunnel(int listenPort, string forwardHost, int forwardPort)
        {
            this.listenPort = listenPort;
            this.forwardHost = forwardHost;
            this.forwardPort = forwardPort;
        }

        public void Listen()
        {
            server = new XyzServer(listenPort);
            server.OnConnect = (XyzClient client) => {
                this.on_Connect?.Invoke(client, Source.Client);

                // backend
                XyzClient backend = new XyzClient();
                backend.OnConnect = ()=> {
                    this.on_Connect?.Invoke(backend, Source.Backend);
                };
                backend.OnDisconnect = () =>
                {
                    client.Disconnect();
                    this.on_Disconnect?.Invoke(backend, Source.Backend);
                };
                backend.OnMessage = (byte[] bytes, int type) =>
                {
                    client.Send(bytes, type);
                    this.on_Message?.Invoke(backend, bytes, type, Source.Backend);
                };
                backend.Connect(forwardHost, forwardPort);

                // frontend
                client.OnDisconnect = () =>
                {
                    backend.Disconnect();
                    this.on_Disconnect?.Invoke(client, Source.Client);
                };

                client.OnMessage = (byte[] bytes, int type) =>
                {
                    backend.Send(bytes, type);
                    this.on_Message?.Invoke(client, bytes, type, Source.Client);
                };

                client.Read();
            };
        }

        public enum Source
        {
            Client,
            Backend,
        }
    }
}
