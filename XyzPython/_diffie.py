from tinyec import registry, ec
import secrets, struct, binascii, hashlib

class _ECDiffieHellman:
    def __init__(self):
        self.curve = registry.get_curve('secp521r1')
        self.private_key = secrets.randbelow(self.curve.field.n)
        self.public_key  = self.private_key * self.curve.g

    def getPublicKey(self):
        keyX = hex(self.public_key.x)[2:]
        keyX = (binascii.unhexlify('0'+keyX) if len(keyX)%2 else binascii.unhexlify(keyX)).rjust(66, b'\x00')
        keyY = hex(self.public_key.y)[2:]
        keyY = (binascii.unhexlify('0'+keyY) if len(keyY)%2 else binascii.unhexlify(keyY)).rjust(66, b'\x00')
        return b'ECK5B\x00\x00\x00'+keyX+keyY

    def getSharedKey(self, remotePublicKey):
        keyX = int('0x'+binascii.hexlify(remotePublicKey[8:8+66]).decode(), 16)
        keyY = int('0x'+binascii.hexlify(remotePublicKey[8+66:8+66+66]).decode(), 16)
        point = ec.Point(self.curve, keyX, keyY)
        shared = self.private_key*point
        key = hex(shared.x)[2:]
        key = (binascii.unhexlify('0'+key) if len(key)%2 else binascii.unhexlify(key)).rjust(66, b'\x00')
        return hashlib.sha256(key).digest()
