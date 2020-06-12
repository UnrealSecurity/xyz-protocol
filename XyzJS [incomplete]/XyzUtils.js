const zlib = require('zlib');
const utf8 = require('utf8');
const util = require('util');

class XyzUtils {
    static TextEncoder = new util.TextEncoder();
    static TextDecoder = new util.TextDecoder('utf-8', {ignoreBOM: true});

    static UTF8Encode(input) {
        return utf8.encode(input);
    }
    
    static UTF8Decode(input) {
        return utf8.decode(input);
    }

    static Deflate(bytes) {
        return zlib.deflateRawSync(bytes);
    }

    static Inflate(bytes) {
        return zlib.inflateRawSync(bytes);
    }

    static Int32ToUint8Array(int) {
        const bytes = new Uint8Array(4);
        for (let i=0; i<bytes.length; i++) {
            const byte = int & 0xff;
            bytes[i] = byte;
            int = (int-byte)/256;
        }
        return bytes;
    }
    
    static Uint8ArrayToInt32(bytes) {
        if (bytes.length != 4) return 0;

        let int = 0;
        for (let i = bytes.length-1; i>=0; i--) {
            int = (int * 256) + bytes[i];
        }
        return int;
    }

    static Int64ToUint8Array(long) {
        const bytes = new Uint8Array(8);
        for (let i=0; i<bytes.length; i++) {
            const byte = long & 0xff;
            bytes[i] = byte;
            long = (long-byte)/256;
        }
        return bytes;
    }
    
    static Uint8ArrayToInt64(bytes) {
        if (bytes.length != 8) return 0;

        let long = 0;
        for (let i = bytes.length-1; i>=0; i--) {
            long = (long * 256) + bytes[i];
        }
        return long;
    }
}

module.exports = XyzUtils;