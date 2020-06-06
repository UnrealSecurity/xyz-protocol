from XyzPython.XyzUtils import XyzUtils
import struct

class DataError(Exception):
    '''
    Exception raised when there is not enough
    data in the XyzMessage buffer to generate
    a requested data type.
    '''
    pass

class XyzMessage:
    '''
    A class for generating/unpacking valid Messages for the Xyz protocol.

    Parameters:
        data\t- The decrypted, inflated data of the packet (Bytes)
        msgid\t- The Message ID of the packet.
    '''
    def __init__(self, data: bytes, msgid: int=0):
        self.data = data
        self.msgid = msgid

    def get(self):
        '''
        Deflates and packs the packet data/id and returns bytes that can be sent
        as-is to the server/client.
        '''
        data = XyzUtils.deflate(self.data)
        return struct.pack("IB", len(data), self.msgid)+data

    def bools(self):
        '''
        Unpacks the data to a list of booleans and returns this list.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        if len(self.data) == 0:
            raise DataError("No data.")

        nbools = len(self.data)
        dtype = "{}?".format(nbools)
        return list(struct.unpack(dtype, self.data))

    def ints(self):
        '''
        Unpacks the data to a list of integers and returns this list.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        if len(self.data) == 0:
            raise DataError("No data.")
        if len(self.data)%4 != 0:
            raise DataError("Size of Data must be a multiple of 4.")

        nints = len(self.data) // 4
        dtype = "{}i".format(nints)
        return list(struct.unpack(dtype, self.data))
    
    def floats(self):
        '''
        Unpacks the data to a list of floats and returns this list.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        if len(self.data) == 0:
            raise DataError("No data.")
        if len(self.data)%4 != 0:
            raise DataError("Size of Data must be a multiple of 4.")

        nfloats = len(self.data) // 4
        dtype = "{}f".format(nfloats)
        return list(struct.unpack(dtype, self.data))

    def longs(self):
        '''
        Unpacks the data to a list of longs and returns this list.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        if len(self.data) == 0:
            raise DataError("No data.")
        if len(self.data)%8 != 0:
            raise DataError("Size of Data must be a multiple of 8.")

        nlongs = len(self.data) // 8
        dtype = "{}q".format(nlongs)
        return list(struct.unpack(dtype, self.data))

    def string(self):
        '''
        UTF-8 decodes the data to a string and returns this string.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        return self.data.decode("utf8")

    def strings(self):
        '''
        Splits the data at NULL-Bytes and returns UTF-8 decoded strings.
        Raises XyzPython.XyzMessage.DataError if the size in the buffer
        is not sufficient for the operation.
        '''
        return self.data.decode("utf8").split("\x00")