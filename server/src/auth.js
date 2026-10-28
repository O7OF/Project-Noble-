/**
 * Project Noble Authentication Utilities
 * Cryptographically secure password hashing (scrypt) and session token generation.
 */

const crypto = require('node:crypto');
const { TOKEN_EXPIRY_DAYS } = require('./config');

const SCRYPT_KEYLEN = 64;

function generateSalt() {
    return crypto.randomBytes(16).toString('hex');
}

function hashPassword(password, salt) {
    if (!salt) salt = generateSalt();
    const hash = crypto.scryptSync(password, salt, SCRYPT_KEYLEN).toString('hex');
    return { hash, salt };
}

function verifyPassword(password, hash, salt) {
    if (!password || !hash || !salt) return false;
    const testHash = crypto.scryptSync(password, salt, SCRYPT_KEYLEN).toString('hex');
    if (testHash.length !== hash.length) return false;
    return crypto.timingSafeEqual(Buffer.from(testHash), Buffer.from(hash));
}

function generateToken() {
    return crypto.randomBytes(32).toString('hex');
}

function calculateExpiry() {
    return Date.now() + (TOKEN_EXPIRY_DAYS * 24 * 60 * 60 * 1000);
}

module.exports = {
    generateSalt,
    hashPassword,
    verifyPassword,
    generateToken,
    calculateExpiry,
};
