from XyzPython.XyzMessage import XyzMessage
from XyzPython.XyzUtils import XyzUtils
import struct

class XyzMessageDecoder:
    '''
    Class for decoding raw streams of messages.
    Will raise a ValueError exception if the data parameter
    is not of type 'bytes'.

    Parameters:
        data\t- Data that has to be decoded. (Bytes)
    '''
    def __init__(self, data: bytes):
        if type(data) != bytes:
            raise ValueError("Data parameter has to be of type 'bytes'.")
        self.data = data
    
    def decode(self):
        '''
        Decodes the data in the object and returns a list
        containing each packet in an XyzPython.XyzMessage.XyzMessage
        object.
        '''
        messages = []
        data = self.data
        while len(data) >= 5:
            pkthdr = data[0:5]
            data = data[5:]
            size, dtype = struct.unpack("IB", pkthdr)
            if len(data) < size:
                return messages
            pktdata = XyzUtils.inflate(data[0:size])
            data = data[size:]
            messages.append(XyzMessage(pktdata, dtype))
        return messages
