/**
 * Project Noble Backend Server Configuration
 */

const path = require('node:path');

const PORT = Number(process.env.PORT || 8080);
const HOST = process.env.HOST || '0.0.0.0';
const TRUST_PROXY = process.env.TRUST_PROXY === '1';

// Paths
const DATA_DIR = process.env.DATA_DIR || path.join(__dirname, '..', 'data');
const DB_PATH = process.env.DB_PATH || path.join(DATA_DIR, 'noble.db');

// Session & Heartbeat timeouts
const SESSION_TIMEOUT_MS = Number(process.env.SESSION_TIMEOUT_MS || 30000); // 30s timeout
const MATCHMAKING_TICKET_TIMEOUT_MS = Number(process.env.MATCHMAKING_TICKET_TIMEOUT_MS || 120000); // 2 minutes

// Security & Limits
const MAX_BODY_BYTES = 1024 * 1024; // 1 MB (supports full match stat records)
const MAX_SESSIONS_PER_IP = 8;
const MAX_SESSIONS_TOTAL = 2000;
const MAX_REQUESTS_PER_MINUTE = 240;
const TOKEN_EXPIRY_DAYS = 30;

module.exports = {
    PORT,
    HOST,
    TRUST_PROXY,
    DATA_DIR,
    DB_PATH,
    SESSION_TIMEOUT_MS,
    MATCHMAKING_TICKET_TIMEOUT_MS,
    MAX_BODY_BYTES,
    MAX_SESSIONS_PER_IP,
    MAX_SESSIONS_TOTAL,
    MAX_REQUESTS_PER_MINUTE,
    TOKEN_EXPIRY_DAYS,
};
