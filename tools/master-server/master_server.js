/**
 * Project Noble master server: a small list of online Halo: Reach (Noble) games.
 *
 * Based on the Outbreak-Z session registry written by the project owner (O7OF), adapted for Noble:
 *   - up to 16 players, Noble's default port 11774, Reach map / mode names;
 *   - each registration gets a secret token; heartbeats and removal need it, so nobody can delete
 *     or take over someone else's listing;
 *   - the listed address is always the address the request came from (never one sent in the body),
 *     so a listing can't point players at a third party;
 *   - limits: request body size, sessions per IP, requests per IP per minute, text lengths.
 *
 * Endpoints:
 *   POST   /api/sessions            register   -> { session_id, token }
 *   PUT    /api/sessions/heartbeat  keep alive (header X-Session-Token)
 *   DELETE /api/sessions/:id        remove     (header X-Session-Token)
 *   GET    /api/sessions            list of active games
 *   GET    /health                  status
 *
 * Run:  node master_server.js        (PORT env var, default 8080)
 * Behind a reverse proxy set TRUST_PROXY=1 so X-Forwarded-For is used for the client address.
 */

const http = require('http');
const crypto = require('crypto');

const PORT = Number(process.env.PORT || 8080);
const TRUST_PROXY = process.env.TRUST_PROXY === '1';
const SESSION_TIMEOUT_MS = 30000;      // dropped 30 s after the last heartbeat
const MAX_BODY_BYTES = 4096;
const MAX_SESSIONS_PER_IP = 4;
const MAX_SESSIONS_TOTAL = 2000;
const MAX_REQUESTS_PER_MINUTE = 120;    // per IP
const MAX_PLAYERS = 16;
const DEFAULT_PORT = 11774;

const sessions = new Map();             // id -> session
const requestCounts = new Map();        // ip -> { count, windowStart }

function now() {
    return Date.now();
}

function clientIp(req) {
    let ip = req.socket.remoteAddress || '';
    if (TRUST_PROXY && req.headers['x-forwarded-for']) {
        ip = String(req.headers['x-forwarded-for']).split(',')[0].trim();
    }
    return ip.replace(/^::ffff:/, '');
}

function rateLimited(ip) {
    const t = now();
    let entry = requestCounts.get(ip);
    if (!entry || t - entry.windowStart > 60000) {
        entry = { count: 0, windowStart: t };
        requestCounts.set(ip, entry);
    }
    entry.count += 1;
    return entry.count > MAX_REQUESTS_PER_MINUTE;
}

// Plain text for the menu: no control characters, limited length.
function cleanText(value, fallback, maxLen) {
    if (typeof value !== 'string') return fallback;
    const text = value.replace(/[\u0000-\u001f\u007f]/g, '').trim().slice(0, maxLen);
    return text.length > 0 ? text : fallback;
}

function cleanInt(value, fallback, min, max) {
    const n = Number.parseInt(value, 10);
    if (!Number.isFinite(n)) return fallback;
    return Math.min(max, Math.max(min, n));
}

function tokensEqual(a, b) {
    if (typeof a !== 'string' || typeof b !== 'string' || a.length !== b.length) return false;
    return crypto.timingSafeEqual(Buffer.from(a), Buffer.from(b));
}

function sendJson(res, status, data) {
    res.writeHead(status, {
        'Content-Type': 'application/json',
        'Cache-Control': 'no-store',
        'Access-Control-Allow-Origin': '*',          // the list is public; writes need a token
        'Access-Control-Allow-Methods': 'GET, POST, PUT, DELETE, OPTIONS',
        'Access-Control-Allow-Headers': 'Content-Type, X-Session-Token',
    });
    res.end(JSON.stringify(data));
}

function readJson(req, callback) {
    let body = '';
    let tooLarge = false;
    req.on('data', chunk => {
        if (tooLarge) return;
        body += chunk;
        if (body.length > MAX_BODY_BYTES) {
            tooLarge = true;
            callback(new Error('too large'), null, 413);
            req.destroy();
        }
    });
    req.on('end', () => {
        if (tooLarge) return;
        if (!body) return callback(null, {});
        try {
            const data = JSON.parse(body);
            if (data === null || typeof data !== 'object' || Array.isArray(data)) return callback(new Error('not an object'), null, 400);
            callback(null, data);
        } catch (err) {
            callback(err, null, 400);
        }
    });
}

function publicView(s) {
    return {
        id: s.id,
        name: s.name,
        address: s.address,
        port: s.port,
        player_count: s.playerCount,
        max_players: s.maxPlayers,
        map: s.map,
        mode: s.mode,
        in_progress: s.inProgress,
        has_password: s.hasPassword,
        build: s.build,
    };
}

function sessionsForIp(ip) {
    let count = 0;
    for (const s of sessions.values()) if (s.address === ip) count += 1;
    return count;
}

// Drop sessions without a recent heartbeat, and old rate-limit windows.
setInterval(() => {
    const t = now();
    for (const [id, s] of sessions) {
        if (t - s.lastHeartbeat > SESSION_TIMEOUT_MS) sessions.delete(id);
    }
    for (const [ip, entry] of requestCounts) {
        if (t - entry.windowStart > 120000) requestCounts.delete(ip);
    }
}, 5000);

const server = http.createServer((req, res) => {
    if (req.method === 'OPTIONS') return sendJson(res, 204, {});

    const ip = clientIp(req);
    if (rateLimited(ip)) return sendJson(res, 429, { error: 'Too many requests' });

    let pathname;
    try {
        pathname = new URL(req.url, 'http://localhost').pathname;
    } catch {
        return sendJson(res, 400, { error: 'Bad URL' });
    }

    if (req.method === 'GET' && (pathname === '/health' || pathname === '/')) {
        return sendJson(res, 200, { service: 'Project Noble master server', status: 'online', active_sessions: sessions.size });
    }

    if (req.method === 'GET' && pathname === '/api/sessions') {
        const t = now();
        const list = [];
        for (const s of sessions.values()) {
            if (t - s.lastHeartbeat <= SESSION_TIMEOUT_MS) list.push(publicView(s));
        }
        return sendJson(res, 200, list);
    }

    if (req.method === 'POST' && pathname === '/api/sessions') {
        return readJson(req, (err, data, status) => {
            if (err) return sendJson(res, status || 400, { error: 'Invalid JSON body' });
            if (sessions.size >= MAX_SESSIONS_TOTAL) return sendJson(res, 503, { error: 'Server list is full' });
            if (sessionsForIp(ip) >= MAX_SESSIONS_PER_IP) return sendJson(res, 429, { error: 'Too many games from this address' });

            const id = crypto.randomBytes(8).toString('hex');
            const token = crypto.randomBytes(24).toString('hex');
            const session = {
                id,
                token,
                address: ip,
                name: cleanText(data.name, 'Reach game', 48),
                port: cleanInt(data.port, DEFAULT_PORT, 1, 65535),
                playerCount: cleanInt(data.player_count, 1, 0, MAX_PLAYERS),
                maxPlayers: cleanInt(data.max_players, MAX_PLAYERS, 1, MAX_PLAYERS),
                map: cleanText(data.map, 'Unknown map', 48),
                mode: cleanText(data.mode, 'Multiplayer', 24),
                inProgress: data.in_progress === true,
                hasPassword: data.has_password === true,
                build: cleanText(data.build, '', 24),
                lastHeartbeat: now(),
            };
            sessions.set(id, session);
            console.log(`[master] registered ${id} "${session.name}" from ${ip}:${session.port}`);
            return sendJson(res, 201, { success: true, session_id: id, token });
        });
    }

    if ((req.method === 'PUT' || req.method === 'POST') && pathname === '/api/sessions/heartbeat') {
        return readJson(req, (err, data, status) => {
            if (err) return sendJson(res, status || 400, { error: 'Invalid JSON body' });
            const session = sessions.get(String(data.session_id || ''));
            if (!session) return sendJson(res, 404, { error: 'Session not found' });  // client registers again
            if (!tokensEqual(req.headers['x-session-token'], session.token)) return sendJson(res, 403, { error: 'Wrong token' });
            session.lastHeartbeat = now();
            if (data.player_count !== undefined) session.playerCount = cleanInt(data.player_count, session.playerCount, 0, MAX_PLAYERS);
            if (data.in_progress !== undefined) session.inProgress = data.in_progress === true;
            if (data.map !== undefined) session.map = cleanText(data.map, session.map, 48);
            return sendJson(res, 200, { success: true });
        });
    }

    if (req.method === 'DELETE' && pathname.startsWith('/api/sessions/')) {
        const id = pathname.slice('/api/sessions/'.length);
        const session = sessions.get(id);
        if (!session) return sendJson(res, 404, { error: 'Session not found' });
        if (!tokensEqual(req.headers['x-session-token'], session.token)) return sendJson(res, 403, { error: 'Wrong token' });
        sessions.delete(id);
        console.log(`[master] removed ${id}`);
        return sendJson(res, 200, { success: true });
    }

    return sendJson(res, 404, { error: 'Endpoint not found' });
});

server.requestTimeout = 10000;
server.headersTimeout = 10000;
server.listen(PORT, () => {
    console.log(`[master] Project Noble master server on port ${PORT} (sessions expire after ${SESSION_TIMEOUT_MS / 1000} s)`);
});
