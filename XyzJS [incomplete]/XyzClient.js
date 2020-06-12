const XyzUtils = require('./XyzUtils.js');
const net = require('net');

class XyzClient {
    socket = new net.Socket();
    onConnect = null;
    onDisconnect = null;
    onMessage = null;
    data = [];

    constructor() {
        this.socket.on('connect', ()=>{
            if (this.onConnect) {
                this.onConnect();
            }
        });

        this.socket.on('close', ()=>{
            if (this.onDisconnect) {
                this.onDisconnect();
            }
        });

        this.socket.on('data', (data)=>{
            this.data.push(...Array.from(data));

            if (this.data.length >= 5) {
                const length = XyzUtils.Uint8ArrayToInt32(Uint8Array.from(this.data.slice(0, 4)));
                
                if (this.data.length >= length) {
                    const type = this.data[4];
                    const _message = Uint8Array.from(this.data.slice(5, 5+length));
                    this.data.splice(0, 5+length);

                    const message = XyzUtils.Inflate(_message);

                    if (this.onMessage) {
                        this.onMessage(message, type);
                    }
                }
            }
        });
    }

    connect(host, port) {
        this.socket.connect(port, host);
    }

    send(data, type) {
        const dataBytes = new Uint8Array(data.length);
        if (Array.isArray(data) || data instanceof Uint8Array) {
            dataBytes.set(data);
        } else if ('string' === typeof data) {
            dataBytes.set(XyzUtils.TextEncoder.encode(XyzUtils.UTF8Encode(data)));
        }

        const message = XyzUtils.Deflate(dataBytes);
        const length = new Uint8Array(4);
        length.set(XyzUtils.Int32ToUint8Array(message.length));
        const _type = new Uint8Array(1);
        _type.set([Number(type)]);

        const bytes = new Uint8Array(length.length + _type.length + message.length);
        bytes.set(length, 0);
        bytes.set(_type, 4);
        bytes.set(message, 5);

        this.socket.write(bytes);
    }
}

module.exports = XyzClient;