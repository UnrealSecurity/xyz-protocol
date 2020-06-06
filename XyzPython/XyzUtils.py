from Crypto.Cipher import AES
from pbkdf2 import PBKDF2
import zlib, hashlib

class InflateError(Exception):
    pass

class DeflateError(Exception):
    pass

class CryptError(Exception):
    pass

class HashError(Exception):
    pass

class NoAddressError(Exception):
    pass

class SendError(Exception):
    pass

def _PKCS7P(data):
    return data+(chr(16-len(data)%16)*(16-len(data)%16)).encode()

def _PKCS7U(data):
    return data[:-data[-1]]

class XyzUtils:
    def deflate(message):
        if type(message) != bytes:
            raise DeflateError("Invalid message data type.")

        return zlib.compress(message)[2:-4]

    def inflate(message):
        if type(message) != bytes:
            raise InflateError("Invalid message data type.")
    
        try:
            _data = zlib.decompress(message, -15)
        except zlib.error:
            raise InflateError("Failed to inflate data.")
        
        return _data
    
    def _getAES(key):
        salt = bytes([0x44, 0x12, 0x08, 0x19, 0x4D, 0x51, 0x1B, 0x09])
        pdb = PBKDF2(key, salt, 2)
        return AES.new(pdb.read(32), AES.MODE_CBC, pdb.read(16))

    def encrypt(message, key):
        if type(message) != bytes:
            raise CryptError("Invalid message data type.")
        if type(key) != bytes:
            raise CryptError("Invalid key data type.")

        aes = XyzUtils._getAES(key)
        return aes.encrypt(_PKCS7P(message))
    
    def decrypt(message, key):
        if type(message) != bytes:
            raise CryptError("Invalid message data type.")
        if type(key) != bytes:
            raise CryptError("Invalid key data type.")

        aes = XyzUtils._getAES(key)
        return _PKCS7U(aes.decrypt(message))

    def md5(message):
        if type(message) != bytes:
            raise HashError("Invalid message data type.")
        return hashlib.md5(message).hexdigest()
    
    def sha256(message):
        if type(message) != bytes:
            raise HashError("Invalid message data type.")
        return hashlib.sha256(message).hexdigest()