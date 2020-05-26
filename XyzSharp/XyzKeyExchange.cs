//using System.Numerics;
using System.Security.Cryptography;

namespace XyzSharp
{
    class XyzKeyExchange
    {
        byte[] public_key = null;
        byte[] remote_public_key = null;
        ECDiffieHellmanCng ecdh;

        public XyzKeyExchange()
        {
            this.ecdh = new ECDiffieHellmanCng();
            this.ecdh.KeyDerivationFunction = ECDiffieHellmanKeyDerivationFunction.Hash;
            this.ecdh.HashAlgorithm = CngAlgorithm.Sha256;
            this.public_key = this.ecdh.PublicKey.ToByteArray();
        }

        public byte[] GetLocalPublicKey()
        {
            return this.public_key;
        }

        public void SetRemotePublicKey(byte[] remote_public_key)
        {
            this.remote_public_key = remote_public_key;
        }

        public byte[] GetSharedSecretKey()
        {
            return this.ecdh.DeriveKeyMaterial(CngKey.Import(remote_public_key, CngKeyBlobFormat.EccPublicBlob));
        }
    }
}
