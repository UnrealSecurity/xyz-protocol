const XyzUtils = require('./XyzUtils.js');
const secp256k1 = require('secp256k1');
const { randomBytes } = require('crypto');

/*
    const crypto = require('crypto');
    const alice = crypto.createECDH('secp256k1');
    const bob = crypto.createECDH('secp256k1');

    // Note: This is a shortcut way to specify one of Alice's previous private
    // keys. It would be unwise to use such a predictable private key in a real
    // application.
    alice.setPrivateKey(
    crypto.createHash('sha256').update('alice', 'utf8').digest()
    );

    // Bob uses a newly generated cryptographically strong
    // pseudorandom key pair bob.generateKeys();

    const alice_secret = alice.computeSecret(bob.getPublicKey(), null, 'hex');
    const bob_secret = bob.computeSecret(alice.getPublicKey(), null, 'hex');

    // alice_secret and bob_secret should be the same shared secret value
    console.log(alice_secret === bob_secret);
*/

function _generatePrivateKey() {
    while (true) {
        const key = randomBytes(32);
        if (secp256k1.privateKeyVerify(key)) return key;
    }
}

class XyzKeyExchange {
    privateKey = null;
    remotePublicKey = null;

    constructor() {
        this.privateKey = _generatePrivateKey();
    }

    getLocalPublicKey() {
        const localPublicKey = secp256k1.publicKeyCreate(this.privateKey);
        const hexstr = Buffer.from(localPublicKey).toString('hex');
        const bytes = XyzUtils.TextEncoder.encode(hexstr);
        return bytes;
    }

    setRemotePublicKey(bytes) {
        this.remotePublicKey = bytes;
    }

    getSharedSecretKey() {
        const secretSharedKey = secp256k1.ecdh(this.remotePublicKey, this.privateKey);
        const hexstr = Buffer.from(secretSharedKey).toString('hex');
        const bytes = XyzUtils.TextEncoder.encode(hexstr);
        return bytes;
    }
}

module.exports = XyzKeyExchange;