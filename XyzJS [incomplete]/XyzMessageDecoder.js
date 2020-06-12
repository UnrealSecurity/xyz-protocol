const XyzUtils = require('./XyzUtils.js');
const XyzMessage = require('./XyzMessage.js');

class XyzMessageDecoder {
    bytes = null;

    constructor(bytes) {
        this.bytes = bytes;
    }

    decode() {
        const arr = [];

        let i = 0;
        while (i < this.bytes.length) {
            const length = XyzUtils.Uint8ArrayToInt32(this.bytes.slice(i, i+4));
            const type = Number(this.bytes[i+4]);
            const message = this.bytes.slice(i+5, i+5+length);
            arr.push(new XyzMessage(message, type))
            i += 5+message.length;
        }

        return arr;
    }
}

module.exports = XyzMessageDecoder;