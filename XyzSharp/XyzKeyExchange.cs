using System.Numerics;

namespace XyzSharp
{
    class XyzKeyExchange
    {
        int p = 0, g = 0;
        byte[] private_key = null;
        byte[] remote_public_key = null;

        public XyzKeyExchange(int p = 23, int g = 5)
        {
            this.p = p;
            this.g = g;
        }

        public void GeneratePrivateKey(int length = 128)
        {
            this.private_key = new byte[length];
            XyzUtils.random.NextBytes(this.private_key);
        }

        public byte[] GetLocalPublicKey()
        {
            byte[] public_key = new byte[this.private_key.Length];

            for (int i = 0; i < this.private_key.Length; i++)
            {
                public_key[i] = (byte)(int)BigInteger.ModPow(this.g, this.private_key[i], this.p);
            }

            return public_key;
        }

        public void SetRemotePublicKey(byte[] remote_public_key)
        {
            this.remote_public_key = remote_public_key;
        }

        public byte[] GetSharedSecretKey()
        {
            byte[] shared_key = new byte[this.private_key.Length];

            for (int i = 0; i < this.private_key.Length; i++)
            {
                shared_key[i] = (byte)(int)BigInteger.ModPow(this.remote_public_key[i], this.private_key[i], this.p);
            }

            return shared_key;
        }
    }
}
