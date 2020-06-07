from XyzPython.XyzUtils import XyzUtils, SendError, NoAddressError
from XyzPython.XyzMessage import XyzMessage
from XyzPython.XyzMessageDecoder import XyzMessageDecoder
import socket, threading, struct

class RunningError(Exception):
    '''
    An exception that is raised whenever the user
    tries to connect to a server or bind to an address
    when the XyzUdpClient is already busy.
    '''
    pass

class ClientError(Exception):
    '''
    An exception that is raised whenever the user
    tries to disconnect or unbind a XyzUdpClient
    object that is not active or doesn't support
    the action.
    '''
    pass

class _XyzUdpListener(threading.Thread):
    def __init__(self, server, socket, callback, dccallback, isserver=False):
        super().__init__()
        self.socket = socket
        self.server = server
        self.callback = callback
        self.disconnect = dccallback
        self.running = False
        self.finished = True
        self.isserver = isserver
    
    def quit(self):
        self.running = False
        while not self.finished:
            pass
        self.disconnect(self.server)

    def run(self):
        self.finished = False
        self.running = True
        while self.running:
            try:
                payload, sender = self.socket.recvfrom(65536)
            except Exception as e:
                continue
            
            messages = XyzMessageDecoder(payload).decode()
            for message in messages:
                if self.isserver:
                    client = XyzUdpClient(*sender)
                    client.running = True
                    self.callback(self.server, client, message.data, message.msgid)
                else:
                    self.callback(self.server, message.data, message.msgid)

        self.finished = True


class XyzUdpClient:
    '''
    A class that acts as both, a server AND a client.
    Whether the class acts as a server or a client is being determined when
    either the bind or the connect method is executed.

    The host and port parameters can be ignored in the initialization, however,
    you'll have to set them in connect/bind method of this class if you decide to
    leave them away.

    Parameters:
        host\t- The host to connect/bind to. Can be a domain or IPv4 Address.      (String)
        port\t- The port on which the Xyz server is listening on/to listen on.     (Integer)
    '''
    def __init__(self, host: str=None, port: int=None):
        self._client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._listener = None
        self._server = False
        self.running = False
        self.address = (host, port)
        self.onOpen = lambda client: None
        self.onMessage = None
        self.onClose = lambda client: None

    def bind(self, host: str=None, port: int=None):
        '''
        When this method is called, the XyzUdpClient becomes a Server. If it is already
        bound or connected, the RunningError exception will be raised.
        
        You can completely leave the host parameter away which will lead
        to the server listening on '0.0.0.0' which means the Server
        will isten on every network interface.

        In case you specify host and/or port in this method but also
        supplied the parameters when initializing the object, the
        server will listen on the parameters specified in this method.

        The object's onOpen method, which you can override, is being called
        when binding is finished.
        The XyzUdpClient object is being passed to this onOpen function.

        The object's onClose method, which you can override, is being called
        when the server is closed down.
        The XyzUdpClient object is being passed to this onClose function.

        The object's onMessage method, which you can override, is being called
        whenever a client sends a packet to this server.
        This XyzUdpClient object and another XyzUdpClient object that acts
        as a "connection" to the client. You can call the 'send' method of
        this client object in order to send data to the client.

        This method will raise XyzPython.XyzUtils.NoAddressError if you did not
        supply the client object with a host and port argument.

        Parameters:
            host\t- The host to bind to. Can be a doimain or IPv4 Address.      (String)
            port\t- The port on which the XyzUdpClient server should listen on. (Integer)
        '''
        if self.running:
            raise RunningError("Failed to bind UDP server: XyzUdpClient already active.")
        if (host == None and port == None) and (self.address[0] == None and self.address[1] == None):
            raise NoAddressError("No Server Address supplied.")
        if not (host == None and port == None):
            self.address = (host, port)
        
        if not self.onMessage:
            self.onMessage = lambda server, client, message, messageID: None

        self._client.bind(self.address)
        self._client.settimeout(5)

        self._listener = _XyzUdpListener(self, self._client, self.onMessage, self.onClose, isserver=True)
        self._listener.start()

        self.onOpen(self)

        self._server = True
        self.running = True
    
    def connect(self, host: str=None, port: int=None):
        '''
        When this method is called, the XyzUdpClient becomes a Client that can connect to a
        XyzUdpClient server. If it is already bound or connected, the RunningError exception
        will be raised. The host and port parameters only have to be set in case you didn't
        specify them when creating the object.

        In case you specify host and/or port in this method but also
        supplied the parameters when initializing the object, the
        client will connect using the parameters specified in this method.

        The object's onOpen method, which you can override, is being called
        when setting up the client is finished.
        The XyzUdpClient object is being passed to this onOpen function.

        The object's onClose method, which you can override, is being called
        when the client is closed down.
        The XyzUdpClient object is being passed to this onClose function.

        The object's onMessage method, which you can override, is being called
        whenever you receive a message from the XyzUdpClient Server.
        The XyzUdpClient object, the Message (Bytes) and the Message ID (int)
        are being passed to this onMessage function.

        This method will raise XyzPython.XyzUtils.NoAddressError if you did not
        supply the client object with a host and port argument.

        Parameters:
            host\t- The host to "connect" to. Can be a domain or IPv4 Address.  (String)
            port\t- The port on which the XyzUdpClient server is listening on.  (Integer)
        '''
        if self.running:
            raise RunningError("Failed to connect UDP client: XyzUdpClient already active.")
        if (host == None and port == None) and (self.address[0] == None and self.address[1] == None):
            raise NoAddressError("No Server Address supplied.")
        if not (host == None and port == None):
            self.address = (host, port)
        
        if not self.onMessage:
            self.onMessage = lambda server, message, messageID: None
        
        self._listener = _XyzUdpListener(self, self._client, self.onMessage, self.onClose)
        self._listener.start()
        self.onOpen(self)

        self.running = True
    
    def unbind(self):
        '''
        Unbinds the XyzUdpClient or closes the clients "connection" varying from
        state of this object. Should be called instead of disconnect whenever
        the XyzUdpClient object is acting as a server.
        
        The RunningError exception will be raised when the object is not active.
        
        The ClientError exception will be raised when the object is not active. 
        '''
        if not self.running:
            raise RunningError("Failed to unbind server. Not running.")
        if self._listener == None:
            raise ClientError("Cannot unbind client.")
        threading.Thread(target=self._listener.quit).start()
        self.running = False

    def disconnect(self):
        '''
        The same as the XyzUdpClient.unbind method. Renamed in order
        to make it easier to make sense of the action. Should be
        called instead of unbind whenever the XyzUdpClient object
        is acting as a client.
        '''
        if not self.running:
            raise RunningError("Failed to disconnect. Not running.")
        if self._listener == None:
            raise ClientError("Cannot disconnect server client on UDP socket.")

        threading.Thread(target=self._listener.quit).start()
        self.running = False

    def send(self, messages=None, data=None, msgid=None):
        '''
        Sends a message packet. In case the XyzUdpClient object is not active,
        the RunningError exception is raised.
        You can choose whether you want to send already created XyzMessages or
        let the function create them for you.

        That means you either have to specify messages or (data AND msgid).
        In case both are specified, the (data AND msgid) will be used.

        Parameters:
        1:
            messages\t- Message(s) to be sent (XyzMessage or list of XyzMessages)
        2:
            data\t- Raw message to be sent (Bytes)
            msgid\t- Message ID             (Integer)
        '''
        if not self.running:
            raise RunningError("Failed to send packet(s). Not running.")
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
            msg = message.get()
            if len(msg) > 65536:
                raise SendError("Max. UDP datagram size is 65536 bytes. Your packet currently has {} bytes.".format(len(msg)))
            self._client.sendto(msg, self.address)
