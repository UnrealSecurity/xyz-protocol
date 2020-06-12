const XyzUtils = require('./XyzUtils.js');

class XyzMessage {
    _bytes = null;
    _type = 0;

    /**
     * XyzMessage object
     * @param {Uint8Array} bytes
     * @param {Number} type
     */
    constructor(bytes, type=0) {
        this._bytes = bytes;
        this._type = type;
    }

    get bytes() {
        return this._bytes;
    }

    get type() {
        return Number(this._type);
    }
    
    get int() {
        return XyzUtils.Uint8ArrayToInt32(this._bytes);
    }

    get long() {
        return XyzUtils.Uint8ArrayToInt64(this._bytes);
    }

    get string() {
        return XyzUtils.TextDecoder.decode(this._bytes);
    }

    get strings() {
        return XyzUtils.TextDecoder.decode(this._bytes).split('\0');
    }

    get booleans() {
        const arr = [];

        for (let i=0; i<this._bytes; i++) {
            arr.push(this._bytes[i] == 1 ? true : false);
        }

        return arr;
    }

    get ints() {
        const arr = [];

        for (let i=0; i<this._bytes-4; i+=4) {
            const buffer = new Uint8Array(4);

            buffer.push(this._bytes[i]);
            buffer.push(this._bytes[i+1]);
            buffer.push(this._bytes[i+2]);
            buffer.push(this._bytes[i+3]);

            arr.push(XyzUtils.Uint8ArrayToInt32(buffer));
        }

        return arr;
    }

    get longs() {
        const arr = [];

        for (let i=0; i<this._bytes-8; i+=8) {
            const buffer = new Uint8Array(8);

            buffer.push(this._bytes[i]);
            buffer.push(this._bytes[i+1]);
            buffer.push(this._bytes[i+2]);
            buffer.push(this._bytes[i+3]);
            buffer.push(this._bytes[i+4]);
            buffer.push(this._bytes[i+5]);
            buffer.push(this._bytes[i+6]);
            buffer.push(this._bytes[i+7]);

            arr.push(XyzUtils.Uint8ArrayToInt64(buffer));
        }
        
        return arr;
    }
}

module.exports = XyzMessage;