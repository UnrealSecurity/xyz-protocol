using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.IO.Compression;

namespace XyzSharp
{
    class XyzUtils
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
    }
}
