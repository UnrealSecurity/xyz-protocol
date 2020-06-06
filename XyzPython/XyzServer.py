from XyzPython.XyzClient import XyzClient, _XyzListener
from XyzPython.XyzUtils import XyzUtils, SendError, NoAddressError
import socket, threading

class _XyzConnectionHandler(threading.Thread):
    def __init__(self, server, serversock, ccall):
        super().__init__()
        self.server = server
        self.socket = serversock
        self.connect = ccall
        self.running = False
        self.finished = True
        self.clients = []
    
    def quit(self):
        self.running = False
        while not self.finished:
            pass
        for client in self.clients:
            client.disconnect(self.server)
    
    def run(self):
        self.running = True
        self.finished = False
        while self.running:
            try:
                connection, address = self.socket.accept()
            except Exception as e:
                continue
            client = XyzClient(*address, _sclient=True)
            client._client = connection
            client.connected = True
            self.connect(self.server, client)
            client._listener = _XyzListener(client, connection, client.onMessage, client.onDisconnect, server=self.server)
            client._listener.start()
            self.clients.append(client)

        self.finished = True

class XyzServer:
    '''
    Server for communicating over the Xyz protocol.
    The host and port parameters can be ignored in the initialization, however,
    you'll have to set them in bind method of this class if you decide to
    leave them away.

    The host parameter defaults to '0.0.0.0' which means the Server will
    listen on every network interface.

    Parameters:
        host\t- The host to bind to. Can be a domain or IPv4 Address. (String)
        port\t- The port on which the Xyz server should listen on.    (Integer)
    '''
    def __init__(self, host: str="0.0.0.0", port: int=None):
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._listener = None
        self.address = (host, port)
        self.running = False
        self.finished = True

        self.onConnect = lambda server, connection: None
        self.onClose = lambda server: None
        self.onOpen = lambda server: None

    def bind(self, host: str="0.0.0.0", port: int=None):
        '''
        Binds the XyzServer to a host and port. The host and port only
        have to be set if you didn't do so when initializing the
        XyzServer object.
        
        You can completely leave the host parameter away which will lead
        to the server listening on '0.0.0.0' which means the Server
        will isten on every network interface.

        In case you specify host and/or port in this method but also
        supplied the parameters when initializing the object, the
        server will listen on the parameters specified in this method.

        The object's onOpen method, which you can override, is being called
        when binding is finished.
        The XyzServer object is being passed to this onOpen function.

        The object's onClose method, which you can override, is being called
        when the server is closed.
        The XyzServer object is being passed to this onClose function.

        The object's onConnect method, which you can override, is being called
        whenever a client connects to this server.
        The XyzServer object and a XyzPython.XyzClient.XyzClient representing
        the connected client will be passed to this onConnect function.

        This method will raise XyzPython.XyzUtils.NoAddressError if you did not
        supply the client object with a host and port argument.

        Parameters:
            host\t- The host to bind to. Can be a doimain or IPv4 Address. (String)
            port\t- The port on which the Xyz server should listen on.     (Integer)
        '''
        if not (self.address[1]) and not (port):
            raise NoAddressError("No Server Address supplied.")
        self.address = (host, port) if (port) else self.address
        self._server.bind(self.address)
        self._server.settimeout(5)
        self._server.listen(1024)
        self._listener = _XyzConnectionHandler(self, self._server, self.onConnect)
        self._listener.start()
        self.running = True
        self.finished = False
        self.onOpen(self)
    
    def unbind(self):
        '''
        Closes the server and kicks all the connected clients. Then proceeds to call
        the onClose function which you can override before starting the server.
        '''
        self.onClose(self)
        self._listener.quit()
        self.running = False
