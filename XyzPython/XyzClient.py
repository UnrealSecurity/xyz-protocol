from XyzPython.XyzUtils import XyzUtils, SendError, NoAddressError
from XyzPython.XyzMessage import XyzMessage
import socket, threading, struct

class NotConnectedError(Exception):
    '''
    Exception raised when a client who is not connected
    is trying to execute methods that require a connection.
    '''
    pass

class _XyzListener(threading.Thread):
    def __init__(self, client, connection, callback, disconnectCB, server=False):
        super().__init__()
        self.callback = callback
        self.disconnect = disconnectCB
        self.connection = connection
        self.server = server
        self.client = client
        self.running = False
        self.finished = True

    def quit(self, _server):
        self.running = False
        while not self.finished:
            pass
        if _server:
            self.disconnect(_server, self.client)
        else:
            self.disconnect(self.client)
        self.connection.close()

    def run(self):
        self.running = True
        self.finished = False
        while self.running:
            pkthdr = self.recvall(5)
            if not pkthdr:
                break
            length, ID = struct.unpack("IB", pkthdr)
            data = self.recvall(length)
            if not data:
                break
            if self.server:
                self.callback(self.server, self.client, XyzUtils.inflate(data), ID)
            else:
                self.callback(self.client, XyzUtils.inflate(data), ID)
        self.finished = True

    def recvall(self, size):
        data = b""
        while size-len(data) > 0 and self.running:
            try:
                data += self.connection.recv(size-len(data))
            except Exception as e:
                continue
        if not self.running:
            return None
        return data


class XyzClient:
    '''
    Client for communicating over the Xyz protocol.
    The host and port parameters can be ignored in the initialization, however,
    you'll have to set them in connect method of this class if you decide to
    leave them away.

    Parameters:
        host\t- The host to connect to. Can be a domain or IPv4 Address. (String)
        port\t- The port on which the Xyz server is listening on.        (Integer)
    '''
    def __init__(self, host: str=None, port: int=None, _sclient=False):

        self._client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._listener = None
        self._sclient = _sclient
        self.address = (host, port)
        if _sclient:
            self.onConnect = lambda client, server: None
            self.onMessage = lambda server, client, msgData, msgType: None
            self.onDisconnect = lambda server, client: None
        else:
            self.onConnect = lambda client: None
            self.onMessage = lambda client, msgData, msgType: None
            self.onDisconnect = lambda client: None
        self.connected = False

    def connect(self, host: str=None, port: int=None):
        '''
        Connects the client to a Xyz server. The host and port parameters only
        have to be set in case you didn't specify them when creating the object.

        The object's onConnect method, which you can override, is being called
        when the connection is established.
        The XyzClient object is being passed to this onConnect function.

        The object's onDisconnect method, which you can override, is being called
        when the connection is closed.
        The XyzClient object is being passed to this onDisconnect function.

        The object's onMessage method, which you can override, is being called
        whenever the client receives a packet from the server.
        The XyzClient object,

        This method will raise XyzPython.XyzUtils.NoAddressError if you did not
        supply the client object with a host and port argument.

        Parameters:
            host\t- The host to connect to. Can be a domain or IPv4 Address. (String)
            port\t- The port on which the Xyz server is listening on.        (Integer)
        '''
        if not (self.address[0] and self.address[1]) and not (host and port):
            raise NoAddressError("No Server Address supplied.")

        if (host and port):
            self.address = (host, port)

        self._client.connect(self.address)
        if not self._sclient:
            self._client.settimeout(2)
            self._listener = _XyzListener(self, self._client, self.onMessage, self.onDisconnect)
        else:
            self._listener = _XyzListener(self, self._client, self.onMessage, self.onDisconnect, server=self._sclient)
        self._listener.start()
        self.connected = True
        self.onConnect(self)

    def disconnect(self, _server=None):
        '''
        Disconnects the client from the connected server. In case no connection is
        established, the XyzPython.XyzClient.NotConnectedError exception is raised.
        '''
        if not self.connected:
            raise NotConnectedError("Client is not connected.")
        threading.Thread(target=self._listener.quit, args=(_server,)).start()
        self.connected = False

    def send(self, messages: (XyzMessage,list)=None, data: bytes=None, msgid: int=None):
        '''
        Sends a message packet to the server. In case no connection is established,
        the XyzPython.XyzClient.NotConnectedError exception is raised.
        You can choose whether you want to send already created XyzMessages or
        let the function create them for you.

        That means you either have to specify messages or (data AND msgid).
        In case both are specified, the (data AND msgid) will be used.

        Parameters:
        1:
            messages\t- Message(s) to be sent (XyzMessage or list of XyzMessages)
        2:
            data\t- Raw message to be sent (Bytes)
            msgid\t- Message ID            (Integer)
        '''
        if not self.connected:
            raise NotConnectedError("Client is not connected.")
        if not messages and (data == None and msgid == None):
            raise SendError("No message to send...")
        elif (data != None and msgid != None) and not 0 <= msgid <= 255:
            raise SendError("Invalid message ID")
        if (data != None and msgid != None):
            messages = [XyzMessage(data=data, msgid=msgid)]
        elif type(messages) == XyzMessage:
            messages = [messages]

        for message in messages:
            if message.msgid == None:
                if not msgid:
                    raise SendError("No message id specified.")
                message.msgid = msgid
            self._client.send(message.get())
