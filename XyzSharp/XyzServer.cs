using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Net;
using System.Net.Sockets;

class XyzServer
{
    TcpListener listener;

    private Action<XyzSession> on_Connect = null;
    public Action<XyzSession> OnConnect { get { return on_Connect; } set { on_Connect = value; } }

    public XyzServer(int port)
    {
        listener = new TcpListener(IPAddress.Any, port);
        listener.Start();
        listener.BeginAcceptTcpClient(Connection, listener);
    }

    private void Connection(IAsyncResult ar)
    {
        TcpClient client = this.listener.EndAcceptTcpClient(ar);
        this.listener.BeginAcceptTcpClient(Connection, this.listener);
        XyzSession session = new XyzSession(client);

        this.on_Connect?.Invoke(session);
    }
}