from XyzPython.XyzClient import XyzClient
from XyzPython.XyzServer import XyzServer
from XyzPython.XyzUtils import NoAddressError
from XyzPython.XyzMessage import XyzMessage
import threading

class XyzProxyTunnel:
    '''
    A class for creating a Proxy Tunnel that forwards
    incoming packets to a server.

    The listen_host parameter defaults to '0.0.0.0' which means the Proxy will
    listen on every network interface.

    A XyzPython.XyzUtils.NoAddressError exception will be raised in case not
    all parameters are specified.

    Parameters:
        listen_host\t- The host the Proxy Server should bind to.   (String)
        listen_port\t- The port the Proxy Server should listen on. (Integer)
        forward_host\t- The host the Proxy Server should forward to. (String)
        forward_port\t- The port the Proxy Server should forward to. (Integer)
    '''
    def __init__(self, listen_host: str="0.0.0.0", listen_port: int=None, forward_host: str=None, forward_port: int=None):
        if not (listen_host and listen_port):
            raise NoAddressError("Listen address not specified.")
        if not (forward_host and forward_port):
            raise NoAddressError("Forward address not specified.")

        self.running = False
        self.listen_address = (listen_host, listen_port)
        self.forward_address = (forward_host, forward_port)

        self.onOpen             = lambda proxy:  None
        self.onClose            = lambda proxy:  None
        self.onClientConnect    = lambda server, client: None
        self.onClientDisconnect = lambda server, client: None
        self.onServerConnect    = lambda server: None
        self.onServerDisconnect = lambda server: None

        self.onClientMessage    = lambda server, client, msgData, msgID: XyzMessage(msgData, msgID)
        self.onServerMessage    = lambda client, msgData, msgID: XyzMessage(msgData, msgID)

        self.server             = None
        self.clients            = {}
        self.servers            = {}

    def _onClientMessage(self, server, client, msgData, msgID):
        remote_server = self.clients[client.address]
        data = self.onClientMessage(remote_server, client, msgData, msgID)
        remote_server.send(messages=data)

    def _onServerMessage(self, client, msgData, msgID):
        remote_client = self.servers[client]
        data = self.onServerMessage(remote_client, msgData, msgID)
        remote_client.send(messages=data)

    def _onClientDisconnect(self, server, client):
        self.onClientDisconnect(server, client)
        serv = self.clients[client.address]
        serv.disconnect()
        self.clients.pop(client.address)
        self.servers[serv].pop()

    def _onServerDisconnect(self, server):
        self.onServerDisconnect(server)
        client = self.servers[server]
        client.disconnect()
        self.servers.pop(server)


    def _handle_connection(self, server, client):
        self.onClientConnect(server, client)

        client.onDisconnect = self._onClientDisconnect
        client.onMessage    = self._onClientMessage

        remote_server              = XyzClient(*self.forward_address)
        remote_server.onConnect    = self.onServerConnect
        remote_server.onDisconnect = self._onServerDisconnect
        remote_server.onMessage    = self._onServerMessage

        try:
            remote_server.connect()
        except Exception as e:
            client.disconnect()
            return
        self.clients[client.address] = remote_server
        self.servers[remote_server] = client

    def start(self):
        '''
        When this method is called, the proxy Server will start accepting connections
        and will forward them. In case connecting to the target server failes, the
        connection is just dropped.

        Here is a list of callbacks:
            XyzProxyTunnel.onOpen                     - Called when the proxy opens up
                                            (proxy: XyzProxyTunnel)
            XyzProxyTunnel.onClose                    - Called when the proxy shuts down
                                            (proxy: XyzProxyTunnel)
            XyzProxyTunnel.onClientConnect            - Called when a client connects
                                            (server: XyzServer, client: XyzClient)
            XyzProxyTunnel.onClientDisconnect         - Called when a client disconnects
                                            (server: XyzServer, client: XyzClient)
            XyzProxyTunnel.onServerConnect            - Called when Server connection is established
                                            (server: XyzServer)
            XyzProxyTunnel.onServerConnect            - Called when Server connection is closed
                                            (server: XyzServer)



            XyzProxyTunnel.onClientMessage            - Called when the client sends a message
                                            (
                                                server: XyzServer,
                                                client: XyzClient,
                                                msgData: Bytes,
                                                msgID: Integer
                                            ) Return value (XyzMessage) is being forwarded to the server

            XyzProxyTunnel.onServerMessage            - Called when the server sends a message
                                            (
                                                client: XyzClient,
                                                msgData: Bytes,
                                                msgID: Integer
                                            ) Return value (XyzMessage) is being forwarded to the client
        '''
        self.server           = XyzServer(*self.listen_address)
        self.server.onOpen    = self.onOpen
        self.server.onClose   = self.onClose
        self.server.onConnect = self._handle_connection
        self.server.bind()
        self.running = True

    def stop(self):
        '''
        Completely shuts off the Proxy
        '''
        for client in self.clients.keys():
            self.server.close()
            server = self.clients[client]
            self.clients[client].close()
            self.servers[server].close()
            self.clients.pop(client)
            self.servers.pop(server)
            self.running = False