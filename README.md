# Xyz Network Message Protocol
Xyz protocol is a protocol built on top of TCP/IP and this repository 
contains bunch of libraries for different programming languages to get you started with Xyz.

Xyz library is currently available for `C++`, `C#` and `Python`.
All messages are automatically compressed with Deflate before sending and decompressed upon receiving. 
Xyz library comes with all the things needed to establish a secure connection between two connected parties. 

## Example key exchange in C#
```csharp
/* Initialize Alice & Bob */
XyzKeyExchange alice = new XyzKeyExchange();
XyzKeyExchange bob = new XyzKeyExchange();

/* Alice & Bob exchange their public keys */
alice.SetRemotePublicKey( bob.GetLocalPublicKey() );
bob.SetRemotePublicKey( alice.GetLocalPublicKey() );

/* Alice & Bob now share the same secret keys that can 
   be used to encrypt messages */
byte[] aliceSharedSecret = alice.GetSharedSecretKey();
byte[] bobSharedSecret = bob.GetSharedSecretKey();
```

## Special thanks to
#### [Format_HDD](https://github.com/FormatHDD) for XyzPython
#### [MinusGix](https://github.com/MinusGix) for XyzCpp
