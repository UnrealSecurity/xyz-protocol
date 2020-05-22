﻿using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace XyzSharp
{
    class XyzServer
    {
        TcpListener listener;

        private Action<XyzClient> on_Connect = null;
        public Action<XyzClient> OnConnect { get { return on_Connect; } set { on_Connect = value; } }


        public XyzServer(int port)
        {
            listener = new TcpListener(IPAddress.Any, port);
            listener.Start();
            listener.BeginAcceptTcpClient(Connection, listener);
        }

        private void Connection(IAsyncResult ar)
        {
            TcpClient _client = this.listener.EndAcceptTcpClient(ar);
            this.listener.BeginAcceptTcpClient(Connection, this.listener);
            XyzClient client = new XyzClient(_client);

            this.on_Connect?.Invoke(client);
        }
    }
}