using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Compression;
using System.Security.Cryptography;

namespace XyzSharp
{
    public class XyzUtils
    {
        public static Random random = new Random();

        public static byte[] Deflate(byte[] bytes)
        {
            using (MemoryStream streamUncompressed = new MemoryStream(bytes))
            {
                using (MemoryStream streamCompressed = new MemoryStream())
                {
                    using (DeflateStream compressionStream = new DeflateStream(streamCompressed, CompressionMode.Compress))
                    {
                        streamUncompressed.CopyTo(compressionStream);
                    }

                    return streamCompressed.ToArray();
                }
            }
        }

        public static byte[] Inflate(byte[] bytes)
        {
            using (MemoryStream streamCompressed = new MemoryStream(bytes))
            {
                using (MemoryStream streamDecompressed = new MemoryStream())
                {
                    using (DeflateStream decompressionStream = new DeflateStream(streamCompressed, CompressionMode.Decompress))
                    {
                        decompressionStream.CopyTo(streamDecompressed);
                    }

                    return streamDecompressed.ToArray();
                }
            }
        }

        public static byte[] Encrypt(byte[] message, byte[] key)
        {
            AesManaged aes = new AesManaged();

            byte[] salt = new byte[] { 0x44, 0x12, 0x8, 0x19, 0x4D, 0x51, 0x1B, 0x9 };
            Rfc2898DeriveBytes pdb = new Rfc2898DeriveBytes(key, salt, 2);
            ICryptoTransform encryptor = aes.CreateEncryptor(pdb.GetBytes(32), pdb.GetBytes(16));
            MemoryStream ms = new MemoryStream();
            byte[] encrypted;

            using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))
            {
                using (BinaryWriter writer = new BinaryWriter(cs))
                    writer.Write(message);
                encrypted = ms.ToArray();
            }

            return encrypted;
        }

        public static byte[] Decrypt(byte[] message, byte[] key)
        {
            AesManaged aes = new AesManaged();

            byte[] salt = new byte[] { 0x44, 0x12, 0x8, 0x19, 0x4D, 0x51, 0x1B, 0x9 };
            Rfc2898DeriveBytes pdb = new Rfc2898DeriveBytes(key, salt, 2);
            ICryptoTransform decryptor = aes.CreateDecryptor(pdb.GetBytes(32), pdb.GetBytes(16));
            MemoryStream ms = new MemoryStream();
            byte[] decrypted;

            using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Write))
            {
                using (BinaryWriter writer = new BinaryWriter(cs))
                    writer.Write(message);
                decrypted = ms.ToArray();
            }

            return decrypted;
        }

        public static string MD5(byte[] input)
        {
            MD5 md5 = System.Security.Cryptography.MD5.Create();
            byte[] inputBytes = input;
            byte[] hash = md5.ComputeHash(inputBytes);
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < hash.Length; i++)
            {
                sb.Append(hash[i].ToString("x2"));
            }
            return sb.ToString().ToLower();
        }

        public static string MD5(string input)
        {
            return MD5(Encoding.UTF8.GetBytes(input));
        }

        public static string SHA256(byte[] input)
        {
            SHA256 sha256 = System.Security.Cryptography.SHA256.Create();
            byte[] inputBytes = input;
            byte[] hash = sha256.ComputeHash(inputBytes);
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < hash.Length; i++)
            {
                sb.Append(hash[i].ToString("x2"));
            }
            return sb.ToString().ToLower();
        }

        public static string SHA256(string input)
        {
            return SHA256(Encoding.UTF8.GetBytes(input));
        }
    }
}
