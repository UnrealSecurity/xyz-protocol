const XyzUtils = require('./XyzUtils.js');
const XyzMessage = require('./XyzMessage.js');

class XyzMessageBuilder {
    data = [];

    constructor() {

    }

    /**
     * Convert XyzMessageBuilder's data to Uint8Array
     * @param {XyzMessage} message
     */
    toArray() {
        return Uint8Array.from(this.data);
    }

    /**
     * Add an array of bytes
     * @param {Uint8Array} bytes
     */
    addBytes(bytes, type=0) {
        this.data.push(...Array.from(XyzUtils.Int32ToUint8Array(bytes.length)));
        this.data.push(Number(type));
        this.data.push(...Array.from(bytes));
    }

    /**
     * Add a single string
     * @param {String} string
     */
    addString(string, type=0) {
        this.addBytes(XyzUtils.TextEncoder.encode(XyzUtils.UTF8Encode(string)), type);
    }

    /**
     * Add an array of strings
     * @param {String[]} strings
     */
    addStrings(strings, type=0) {
        this.addBytes(XyzUtils.TextEncoder.encode(XyzUtils.UTF8Encode(strings.join('\0'))), type);
    }

    /**
     * Add 64 bit integer (long)
     * @param {Number} long
     */
    addLong(long, type=0) {
        this.addBytes(XyzUtils.Int64ToUint8Array(long), type);
    }

    /**
     * Add 32 bit integer (int)
     * @param {Number} int
     */
    addInt(int, type=0) {
        this.addBytes(XyzUtils.Int32ToUint8Array(int), type);
    }

    /**
     * Add XyzMessage object
     * @param {XyzMessage} message
     */
    addMessage(message) {
        this.addBytes(message.bytes, message.type);
    }

    /**
     * Add boolean values
     * @param {Boolean[]} booleans
     */
    addBooleans(booleans, type=0) {
        const bytes = new Uint8Array(booleans.length);
        for (let i=0; i<booleans.length; i++) {
            bytes[i] = booleans[i] == true ? 0x01 : 0x00;
        }
        this.addBytes(bytes, type);
    }
}

module.exports = XyzMessageBuilder;