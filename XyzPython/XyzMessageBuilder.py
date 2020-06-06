from XyzPython.XyzMessage import XyzMessage
from XyzPython.XyzMessageDecoder import XyzMessageDecoder
from XyzPython.XyzUtils import XyzUtils
import struct

_inbetween = lambda _min, _max, _val: (_min <= _val <= _max) 

def _getminmax(dtype):
    sz = struct.calcsize(dtype)
    _min = b"\x80" + b"\x00"*(sz-1)
    _max = b"\x7f" + b"\xff"*(sz-1)
    return struct.unpack("!2{}".format(dtype), _min + _max)

def _numtype(number):
    if type(number) == float:
        return "f"
    elif _inbetween(*_getminmax("q"), number):
        return "q"
    elif _inbetween(*_getminmax("i"), number):
        return "i"
    else:
        return

def _isstrarr(array):
    if type(array) != list:
        return False
    oString = bool(len(array))
    for elem in array:
        if type(elem) != str:
            oString = False
    return oString

def _isbarr(array):
    if type(array) != list:
        return False
    oBool = bool(len(array))
    for elem in array:
        if type(elem) != bool:
            oBool = False
    return oBool

def _istnarr(array, dtype):
    if type(array) != list:
        return False
    oType = bool(len(array))
    for elem in array:
        if not type(elem) in [int, float] or _numtype(elem) != dtype:
            oType = False
    return oType


def _XyzBuildMessage(data, dtype):
    data = XyzUtils.deflate(data)
    return struct.pack("IB", len(data), dtype)+data

class XyzMessageBuilder:
    '''
    Class for generating packets. Adds options to directly pack different
    datatypes and convert them to a list of XyzPython.XyzMessage.XyzMessage
    objects.
    '''
    def __init__(self):
        self.data = b""
    
    def add(self, data: (XyzMessage, list, str, float, int, bytes, bool), dtype: int=0):
        '''
        Adds a new packet to the XyzMessageBuilder.
        
        Parameters:
            data\t - Data to pack into the packet. (
                XyzMessage or
                List of Strings or
                List of Floats or
                List of Integers or
                List of Longs or
                List of Booleans or
                String or
                Float or
                Int or
                Long or
                Boolean or
                Bytes)
            
            dtype\t - The Message ID of the packet. 0 if not set. (Int)
        '''
        if type(data) == XyzMessage:
            self.data += data.get()
        elif type(data) == bytes:
            self.data += _XyzBuildMessage(data, dtype)
        elif _isstrarr(data):
            self.data += _XyzBuildMessage(b"\x00".join(string.encode("utf8") for string in data), dtype)
        elif _istnarr(data, "f"):
            self.data += _XyzBuildMessage(struct.pack("{}f".format(len(data)), *data), dtype)
        elif _istnarr(data, "i"):
            self.data += _XyzBuildMessage(struct.pack("{}i".format(len(data)), *data), dtype)
        elif _istnarr(data, "q"):
            self.data += _XyzBuildMessage(struct.pack("{}q".format(len(data)), *data), dtype)
        elif _isbarr(data):
            self.data += _XyzBuildMessage(struct.pack("{}?".format(len(data)), *data), dtype)
        elif type(data) == str:
            self.data += _XyzBuildMessage(data.encode("utf8"), dtype)
        elif type(data) == float:
            self.data += _XyzBuildMessage(struct.pack("f", data), dtype)
        elif type(data) == int:
            if _inbetween(*_getminmax("i"), data):
                self.data += _XyzBuildMessage(struct.pack("i", data), dtype)
            elif _inbetween(*_getminmax("q"), number):
                self.data += _XyzBuildMessage(struct.pack("q", data), dtype)
            else:
                raise ValueError("Number type not supported.")
        elif type(data) == bool:
            self.data += _XyzBuildMessage(b"\x01" if data else b"\x00", dtype)
        else:
            raise ValueError("Data type not supported.")
    
    def get(self):
        '''
        Returns the data of all the packets.
        '''
        return self.data

    def getMessages(self):
        '''
        Returns a list of XyzPython.XyzMessage.XyzMessage objects, each
        one packet.
        '''
        return XyzMessageDecoder(self.get()).decode()
    
    def compileMessages(self, msgid: int=0):
        '''
        Compiles all the messages to a big one.
        Returns a XyzMessage object.

        Parameters:
            msgid\t- Message ID of the packet
        '''
        
        return XyzMessage(data=self.data, msgid=msgid)