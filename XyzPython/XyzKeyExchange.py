from XyzPython._diffie import _ECDiffieHellman

class PKError(Exception):
    '''
    Exception raised when wrong argument type was passed
    to a method or when the user tries to fetch the
    shared key before the exchange is finished.
    '''
    pass

class XyzKeyExchange:
    '''
    Class for simplifying Diffie-Hellman Curved Key Exchange (DHCKE)
    This class doesn't require any parameters.
    '''
    def __init__(self):
        '''
        Generates a random private key using the python 'secrets' module.
        Sets up a public key.
        '''
        self.diffie = _ECDiffieHellman()
        self.remote_public_key = None

    def getLocalPublicKey(self):
        '''
        Returns your public key as bytes.
        '''
        return self.diffie.getPublicKey()

    def setRemotePublicKey(self, remote_public_key: bytes):
        '''
        Sets the public key of the remote connection and switches state to ready.

        Parameters:
            remote_public_key\t- The public key of the remote connection. (Bytes)
        '''
        if not type(remote_public_key) == bytes or len(remote_public_key) != 140:
            raise PKError("Wrong public key data format.")
        self.remote_public_key = remote_public_key

    def getSharedSecretKey(self):
        '''
        Generates and returns a shared secret key out of your private key and the remote's
        public key as bytes.
        Will raise a XyzPython.XyzKeyExchange.PKError exception if the state is
        not ready.
        '''
        if self.remote_public_key == None:
            raise PKError("Remote public key is not set.")
        key = self.diffie.getSharedKey(self.remote_public_key)
        return key

    def isReady(self):
        '''
        Returns a bool whether the state is ready or not.
        '''
        return (self.diffie and self.remote_public_key)
