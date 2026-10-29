const express = require('express');
const sqlite3 = require('sqlite3').verbose();
const bcrypt = require('bcryptjs');
const crypto = require('crypto');
const net = require('net');
const { WebSocketServer } = require('ws');
const path = require('path');
const fs = require('fs');

const app = express();
const TRUST_PROXY = process.env.TRUST_PROXY !== '0';
app.set('trust proxy', 1);

const MASTER_SESSION_TIMEOUT_MS = 30 * 1000;
const MASTER_MAX_SESSIONS = 2000;
const MASTER_MAX_SESSIONS_PER_IP = 4;
const MASTER_MAX_REQUESTS_PER_MINUTE = 120;
const MASTER_MAX_PLAYERS = 64;
const masterSessions = new Map();
const masterRequestCounts = new Map();

function isPrivateIp(ip) {
    if (!ip || !net.isIP(ip)) return true;
    if (ip === '127.0.0.1' || ip === '::1' || ip === 'localhost') return true;
    if (ip.startsWith('10.') || ip.startsWith('192.168.') || ip.startsWith('169.254.')) return true;
    const m = ip.match(/^172\.(\d+)\./);
    if (m) {
        const second = parseInt(m[1], 10);
        if (second >= 16 && second <= 31) return true;
    }
    return false;
}

function masterClientIp(req) {
    const xff = req.headers['x-forwarded-for'];
    let address = '';
    if (typeof xff === 'string' && xff.trim()) {
        address = xff.split(',')[0].trim().replace(/^::ffff:/, '');
    }
    if (!address) {
        address = String(req.ip || (req.socket && req.socket.remoteAddress) || '').replace(/^::ffff:/, '');
    }
    if (net.isIP(address)) return address;
    return '127.0.0.1';
}

function masterCleanText(value, fallback, maxLength) {
    if (typeof value !== 'string') return fallback;
    const text = value.replace(/[\u0000-\u001f\u007f]/g, '').trim().slice(0, maxLength);
    return text || fallback;
}

function masterCleanInt(value, fallback, min, max) {
    const number = Number(value);
    return Number.isInteger(number) ? Math.min(max, Math.max(min, number)) : fallback;
}

function masterRateLimited(ip) {
    const now = Date.now();
    let entry = masterRequestCounts.get(ip);
    if (!entry || now - entry.windowStart >= 60_000) {
        entry = { count: 0, windowStart: now };
        masterRequestCounts.set(ip, entry);
    }
    entry.count += 1;
    return entry.count > MASTER_MAX_REQUESTS_PER_MINUTE;
}

function masterTokenMatches(candidate, token) {
    if (typeof candidate !== 'string' || !/^[a-f0-9]{48}$/i.test(candidate)) return false;
    return crypto.timingSafeEqual(Buffer.from(candidate, 'hex'), Buffer.from(token, 'hex'));
}

function masterPublicSession(session) {
    return {
        id: session.id,
        name: session.name,
        address: session.address,
        port: session.port,
        player_count: session.playerCount,
        max_players: session.maxPlayers,
        map: session.map,
        mode: session.mode,
        variant: session.variant || '',
        difficulty: session.difficulty !== undefined ? session.difficulty : 1,
        in_progress: session.inProgress === true,
        has_password: session.hasPassword,
        build: session.build,
        is_mod: session.isMod === true,
        roster: session.roster || []
    };
}

setInterval(() => {
    const now = Date.now();
    for (const [id, session] of masterSessions) {
        if (now - session.lastHeartbeat > MASTER_SESSION_TIMEOUT_MS) {
            broadcastLobbyEvent(id, { type: 'lobby_closed', reason: 'Session timed out' });
            masterSessions.delete(id);
        }
    }
    for (const [ip, entry] of masterRequestCounts) {
        if (now - entry.windowStart > 120_000) masterRequestCounts.delete(ip);
    }
}, 5000).unref();

// ── Active Session Store ──────────────────────────────────────────────────────
// Maps userId (string) → { token, username, lastSeen }
// A user can only have ONE valid session at a time.
const activeSessions = {};

function generateSessionToken() {
    return crypto.randomBytes(32).toString('hex');
}

// Invalidate all sessions for a userId (called on re-login or logout)
function invalidateSession(userId) {
    delete activeSessions[String(userId)];
}

// Validate that req.body contains a userId + session_token matching activeSessions.
// Returns true and continues if valid. Sends 401 and returns false if invalid.
function validateSession(req, res) {
    const userId = String(req.body.userId || '');
    const token  = String(req.body.session_token || '');
    if (!userId || !token) {
        res.status(401).json({ error: "Session token required" });
        return false;
    }
    const session = activeSessions[userId];
    if (!session || session.token !== token) {
        res.status(401).json({ error: "Invalid or expired session. Please log in again." });
        return false;
    }
    // Refresh last-seen on every valid request
    session.lastSeen = Date.now();
    return true;
}

// Clean up sessions idle for more than 5 minutes (covers crash/disconnect recovery)
// The client sends a ping every 60s, so a live session stays active.
const SESSION_IDLE_MS = 5 * 60 * 1000; // 5 minutes idle timeout
setInterval(() => {
    const cutoff = Date.now() - SESSION_IDLE_MS;
    for (const uid in activeSessions) {
        if (activeSessions[uid].lastSeen < cutoff) {
            delete activeSessions[uid];
        }
    }
}, 15 * 1000); // Check every 15 seconds

// Helper to safely parse JSON strings with fallbacks
function safeParse(str, defaultVal) {
    if (!str) return defaultVal;
    try {
        return typeof str === 'string' ? JSON.parse(str) : str;
    } catch (e) {
        return defaultVal;
    }
}

// ── CORS ─────────────────────────────────────────────────────────────────────
// Required for HTML5 (browser) exports — browsers block cross-origin requests
// unless the server explicitly allows them.
app.use((req, res, next) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization, X-Session-Token');
    if (req.method === 'OPTIONS') {
        return res.sendStatus(204); // Respond to preflight immediately
    }
    next();
});

app.use('/api/sessions', express.json({ limit: '4kb' }));
app.use(express.json({ limit: '10mb' }));

// Check for Render Persistent Disk (/var/data/database.db), DB_PATH env var, or fallback to repository database.db
const repoDbPath = path.join(__dirname, 'database.db');
let renderPersistentDir = '/var/data';
let defaultPersistentPath = fs.existsSync(renderPersistentDir) ? path.join(renderPersistentDir, 'database.db') : null;

let dbPath = process.env.DB_PATH || defaultPersistentPath || repoDbPath;

// If using external persistent disk (e.g. /var/data/database.db on Render) and it doesn't exist yet, seed it from repo copy
if (dbPath !== repoDbPath && !fs.existsSync(dbPath)) {
    try {
        const dir = path.dirname(dbPath);
        if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
        if (fs.existsSync(repoDbPath)) {
            fs.copyFileSync(repoDbPath, dbPath);
        }
    } catch (e) {
        console.error("[Database] Error seeding persistent database file:", e.message);
    }
}

const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error("Database connection error:", err.message);
        process.exit(1);
    } else {
        // Enable WAL for concurrent writes, auto_vacuum to reclaim free pages, and efficient page size
        db.run("PRAGMA journal_mode = WAL;");
        db.run("PRAGMA busy_timeout = 5000;");
        db.run("PRAGMA auto_vacuum = INCREMENTAL;"); // reclaim deleted row space automatically
        db.run("PRAGMA page_size = 4096;");          // efficient page size (applied on new DB)
        db.run("PRAGMA cache_size = -2000;");         // 2MB page cache in memory
        initializeDatabase();
        // Checkpoint WAL every 5 minutes to keep .db file compact
        setInterval(() => { db.run("PRAGMA wal_checkpoint(PASSIVE);"); }, 5 * 60 * 1000);
    }
});

// ── Database Initialisation ──────────────────────────────────────────────────

function initializeDatabase() {
    db.serialize(() => {
        // Users table
        db.run(`CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE,
            password TEXT,
            is_developer INTEGER DEFAULT 0
        )`, (err) => { if (err) console.error("users table error:", err.message); });
        db.run(`ALTER TABLE users ADD COLUMN is_developer INTEGER DEFAULT 0`, () => {});

        // Stats table with full support for Zombies, PvP, Challenges, and Story Mode
        db.run(`CREATE TABLE IF NOT EXISTS stats (
            user_id INTEGER PRIMARY KEY,
            kills INTEGER DEFAULT 0,
            headshots INTEGER DEFAULT 0,
            rounds_survived INTEGER DEFAULT 0,
            downs INTEGER DEFAULT 0,
            revives INTEGER DEFAULT 0,
            revived_count INTEGER DEFAULT 0,
            downed_count INTEGER DEFAULT 0,
            highest_round INTEGER DEFAULT 0,
            highest_points INTEGER DEFAULT 0,
            level INTEGER DEFAULT 1,
            prestige INTEGER DEFAULT 0,
            xp INTEGER DEFAULT 0,
            boss_kills INTEGER DEFAULT 0,
            elemental_kills INTEGER DEFAULT 0,
            grenade_kills INTEGER DEFAULT 0,
            flags_captured INTEGER DEFAULT 0,
            players_killed INTEGER DEFAULT 0,
            domination_wins INTEGER DEFAULT 0,
            domination_losses INTEGER DEFAULT 0,
            tdm_wins INTEGER DEFAULT 0,
            tdm_losses INTEGER DEFAULT 0,
            ffa_wins INTEGER DEFAULT 0,
            ffa_losses INTEGER DEFAULT 0,
            story_missions_completed INTEGER DEFAULT 0,
            story_checkpoints TEXT DEFAULT '{}',
            completed_story_missions TEXT DEFAULT '[]',
            pvp_primary TEXT DEFAULT 'AK-47',
            pvp_secondary TEXT DEFAULT 'M1911',
            challenge_progress TEXT DEFAULT '{}',
            completed_challenges TEXT DEFAULT '[]',
            weapon_prestige TEXT DEFAULT '{}',
            weapon_xp TEXT DEFAULT '{}',
            weapon_levels TEXT DEFAULT '{}',
            unlocked_camos TEXT DEFAULT '["Default"]',
            unlocked_titles TEXT DEFAULT '["Survivor"]',
            unlocked_calling_cards TEXT DEFAULT '["Default"]',
            unlocked_skins TEXT DEFAULT '["Player Starter"]',
            unlocked_capes TEXT DEFAULT '["Default", "Golden Cape"]',
            weapon_attachments TEXT DEFAULT '{}',
            weapon_camos TEXT DEFAULT '{}',
            FOREIGN KEY(user_id) REFERENCES users(id)
        )`, (err) => { if (err) console.error("stats table error:", err.message); });

        // Safely add columns that may not exist in older DB files
        const addStatsColumn = (colName, colType, defaultVal = 0) => {
            db.run(`ALTER TABLE stats ADD COLUMN ${colName} ${colType} DEFAULT ${defaultVal}`, () => {});
        };
        addStatsColumn('downs',                    'INTEGER', 0);
        addStatsColumn('revives',                  'INTEGER', 0);
        addStatsColumn('revived_count',            'INTEGER', 0);
        addStatsColumn('downed_count',             'INTEGER', 0);
        addStatsColumn('highest_round',            'INTEGER', 0);
        addStatsColumn('highest_points',           'INTEGER', 0);
        addStatsColumn('level',                    'INTEGER', 1);
        addStatsColumn('prestige',                 'INTEGER', 0);
        addStatsColumn('xp',                       'INTEGER', 0);
        addStatsColumn('boss_kills',                'INTEGER', 0);
        addStatsColumn('elemental_kills',           'INTEGER', 0);
        addStatsColumn('grenade_kills',             'INTEGER', 0);
        addStatsColumn('flags_captured',            'INTEGER', 0);
        addStatsColumn('players_killed',            'INTEGER', 0);
        addStatsColumn('domination_wins',           'INTEGER', 0);
        addStatsColumn('domination_losses',         'INTEGER', 0);
        addStatsColumn('tdm_wins',                  'INTEGER', 0);
        addStatsColumn('tdm_losses',                'INTEGER', 0);
        addStatsColumn('ffa_wins',                  'INTEGER', 0);
        addStatsColumn('ffa_losses',                'INTEGER', 0);
        addStatsColumn('story_missions_completed',  'INTEGER', 0);
        addStatsColumn('story_checkpoints',         'TEXT', "'{}'");
        addStatsColumn('completed_story_missions',   'TEXT', "'[]'");
        addStatsColumn('pvp_primary',               'TEXT', "'AK-47'");
        addStatsColumn('pvp_secondary',             'TEXT', "'M1911'");
        addStatsColumn('challenge_progress',       'TEXT', "'{}'");
        addStatsColumn('completed_challenges',     'TEXT', "'[]'");
        addStatsColumn('weapon_prestige',          'TEXT', "'{}'");
        addStatsColumn('weapon_xp',                'TEXT', "'{}'");
        addStatsColumn('weapon_levels',            'TEXT', "'{}'");
        addStatsColumn('unlocked_camos',            'TEXT', "'[\"Default\"]'");
        addStatsColumn('unlocked_titles',           'TEXT', "'[\"Survivor\"]'");
        addStatsColumn('unlocked_calling_cards',    'TEXT', "'[\"Default\"]'");
        addStatsColumn('unlocked_skins',            'TEXT', "'[\"Player Starter\"]'");
        addStatsColumn('unlocked_capes',            'TEXT', "'[\"Default\", \"Golden Cape\"]'");
        addStatsColumn('weapon_attachments',        'TEXT', "'{}'");
        addStatsColumn('weapon_camos',              'TEXT', "'{}'");

        // Unlocked items table
        db.run(`CREATE TABLE IF NOT EXISTS unlocked_items (
            user_id INTEGER,
            item_id TEXT,
            PRIMARY KEY(user_id, item_id),
            FOREIGN KEY(user_id) REFERENCES users(id)
        )`, (err) => { if (err) console.error("unlocked_items table error:", err.message); });

        // Selected cosmetics table
        db.run(`CREATE TABLE IF NOT EXISTS cosmetics (
            user_id INTEGER PRIMARY KEY,
            selected_skin TEXT DEFAULT 'Default',
            selected_title TEXT DEFAULT 'Survivor',
            selected_calling_card TEXT DEFAULT 'Default',
            selected_cape TEXT DEFAULT 'Default',
            selected_camo TEXT DEFAULT 'Default',
            FOREIGN KEY(user_id) REFERENCES users(id)
        )`, (err) => { if (err) console.error("cosmetics table error:", err.message); });

        const addCosmeticsColumn = (colName, colType, defaultVal) => {
            db.run(`ALTER TABLE cosmetics ADD COLUMN ${colName} ${colType} DEFAULT '${defaultVal}'`, () => {});
        };
        addCosmeticsColumn('selected_cape', 'TEXT', 'Default');
        addCosmeticsColumn('selected_camo', 'TEXT', 'Default');

        // Friends table
        db.run(`CREATE TABLE IF NOT EXISTS friends (
            user_id INTEGER,
            friend_id INTEGER,
            status TEXT DEFAULT 'pending',
            PRIMARY KEY(user_id, friend_id),
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(friend_id) REFERENCES users(id)
        )`, (err) => { if (err) console.error("friends table error:", err.message); });

        // Performance index for high-speed leaderboard queries
        db.run(`CREATE INDEX IF NOT EXISTS idx_stats_leaderboard ON stats (prestige DESC, level DESC, xp DESC, kills DESC);`, () => {});
    });
}

// ── Root & Health Endpoints ──────────────────────────────────────────────────

app.get('/', (req, res) => {
    res.json({
        status: "online",
        service: "Outbreak-Z Dedicated Backend",
        version: "1.2.0",
        uptime: process.uptime(),
        timestamp: Date.now()
    });
});

app.get('/api/health', (req, res) => {
    res.json({
        status: "healthy",
        uptime: process.uptime(),
        activeSessions: Object.keys(activeSessions).length,
        activeLobbies: Object.keys(activeLobbies).length,
        masterSessions: masterSessions.size,
        timestamp: Date.now()
    });
});

// ── Noble Master Server API ───────────────────────────────────────────────────

app.get('/api/sessions', (req, res) => {
    const ip = masterClientIp(req) || '127.0.0.1';
    if (masterRateLimited(ip)) return res.status(429).json({ error: "Too many requests" });
    const now = Date.now();
    const sessions = [];
    for (const session of masterSessions.values()) {
        if (!session.isPrivate && (now - session.lastHeartbeat <= MASTER_SESSION_TIMEOUT_MS)) sessions.push(masterPublicSession(session));
    }
    res.set('Cache-Control', 'no-store').json(sessions);
});

app.post('/api/sessions', (req, res) => {
    let ip = masterClientIp(req);
    if (req.body && typeof req.body.address === 'string' && net.isIP(req.body.address.trim())) {
        const candidate = req.body.address.trim();
        if (!isPrivateIp(candidate)) {
            ip = candidate;
        } else if (isPrivateIp(ip)) {
            ip = candidate;
        }
    }
    if (!ip) return res.status(400).json({ error: "A valid client address is required" });
    if (masterRateLimited(ip)) return res.status(429).json({ error: "Too many requests" });
    if (masterSessions.size >= MASTER_MAX_SESSIONS) return res.status(503).json({ error: "Server list is full" });
    let sessionsFromAddress = 0;
    for (const session of masterSessions.values()) if (session.address === ip) sessionsFromAddress += 1;
    const reusingSession = (() => {
        const b0 = req.body || {};
        const port0 = masterCleanInt(b0.port, 11774, 1, 65535);
        for (const s0 of masterSessions.values()) if (s0.address === ip && (s0.port === port0 || s0.id === String(b0.session_id || ''))) return true;
        return false;
    })();
    if (!reusingSession && sessionsFromAddress >= MASTER_MAX_SESSIONS_PER_IP) {
        return res.status(429).json({ error: "Too many games from this address" });
    }

    const body = req.body || {};
    // Reuse the existing session for the same host address + port so a host that
    // re-registers (pre-game lobby -> match launch) keeps ONE session id. Otherwise
    // the advertised id, the relay room and the guests' lobby id all diverge and the
    // guests end up in a different session than the host.
    const reqPort = masterCleanInt(body.port, 11774, 1, 65535);
    let existing = null;
    if (body.session_id && masterSessions.has(String(body.session_id))) {
        const s0 = masterSessions.get(String(body.session_id));
        if (s0.address === ip) existing = s0;
    }
    if (!existing) {
        for (const s0 of masterSessions.values()) {
            if (s0.address === ip && s0.port === reqPort) { existing = s0; break; }
        }
    }
    const id = existing ? existing.id : crypto.randomBytes(8).toString('hex');
    const token = existing ? existing.token : crypto.randomBytes(24).toString('hex');

    const hostPeerId = body.host_peer_id || body.player_id || 1;
    const hostMachineId = body.host_machine_id || '0x484F535400000001';
    const hostUsername = masterCleanText(body.host_username || body.player_name, 'Host', 24);
    const hostIsDev = body.host_is_dev === true;
    const hostTag = masterCleanText(body.host_service_tag, '117', 4);

    const initialRoster = Array.isArray(body.roster) && body.roster.length > 0 ? body.roster : (existing && Array.isArray(existing.roster) && existing.roster.length > 0) ? existing.roster : [
        {
            peerId: hostPeerId,
            machineId: hostMachineId,
            username: hostUsername,
            isHost: true,
            isDev: hostIsDev,
            serviceTag: hostTag,
            ping: 0,
            joinedAt: Date.now()
        }
    ];

    masterSessions.set(id, {
        id,
        token,
        address: ip,
        name: masterCleanText(body.name, 'Reach game', 48),
        port: masterCleanInt(body.port, 11774, 1, 65535),
        playerCount: masterCleanInt(body.player_count, initialRoster.length, 0, MASTER_MAX_PLAYERS),
        maxPlayers: masterCleanInt(body.max_players, 16, 1, MASTER_MAX_PLAYERS),
        map: masterCleanText(body.map, '20_sword_slayer', 48),
        mode: masterCleanText(body.mode, 'Multiplayer', 24),
        variant: masterCleanText(body.variant, '', 64),
        difficulty: masterCleanInt(body.difficulty, 1, 0, 3),
        skulls: String(body.skulls || '0'),
        inProgress: body.in_progress === true,
        isPrivate: body.is_private === true,
        lanAddress: masterCleanText(body.lan_address, '', 64),
        hasPassword: body.has_password === true,
        build: masterCleanText(body.build, '', 24),
        isMod: body.is_mod === true || String(body.name || '').includes('[Mod]') || String(body.mode || '').toLowerCase().includes('mod'),
        roster: initialRoster,
        launchData: (existing && body.in_progress === true) ? existing.launchData : null,
        lastHeartbeat: Date.now()
    });
    if (existing) console.log(`[Master] Host ${ip}:${reqPort} re-registered; reusing session ${id}`);
    res.status(existing ? 200 : 201).json({ success: true, session_id: id, token, reused: !!existing });
});

app.put('/api/sessions/heartbeat', (req, res) => {
    const ip = masterClientIp(req);
    // ip allowed
    if (masterRateLimited(ip)) return res.status(429).json({ error: "Too many requests" });
    const body = req.body || {};
    const session = masterSessions.get(String(body.session_id || ''));
    if (!session) return res.status(404).json({ error: "Session not found" });
    if (!masterTokenMatches(req.get('X-Session-Token'), session.token)) {
        return res.status(403).json({ error: "Wrong token" });
    }
    session.lastHeartbeat = Date.now();
    if (body.player_count !== undefined) {
        session.playerCount = masterCleanInt(body.player_count, session.playerCount, 0, MASTER_MAX_PLAYERS);
    }
    if (body.in_progress !== undefined) {
        session.inProgress = body.in_progress === true;
        if (!session.inProgress) session.launchData = null;
    }
    if (body.is_private !== undefined) session.isPrivate = body.is_private === true;
    if (body.lan_address !== undefined) session.lanAddress = masterCleanText(body.lan_address, session.lanAddress || '', 64);
    if (body.map !== undefined) session.map = masterCleanText(body.map, session.map, 48);
    if (body.mode !== undefined) session.mode = masterCleanText(body.mode, session.mode, 24);
    if (body.variant !== undefined) session.variant = masterCleanText(body.variant, session.variant, 64);
    if (body.difficulty !== undefined) session.difficulty = masterCleanInt(body.difficulty, session.difficulty, 0, 3);
    if (body.skulls !== undefined) session.skulls = String(body.skulls);
    if (body.roster && Array.isArray(body.roster)) {
        session.roster = body.roster;
        session.playerCount = session.roster.length;
    }

    const pendingJoins = session.pendingJoins || [];
    session.pendingJoins = [];
    const hostHeader = req.headers.host || 'outbreak-z-test-server.onrender.com';
    res.json({
        success: true,
        pending_joins: pendingJoins,
        roster: session.roster || [],
        launch_data: session.launchData,
        relay_url: `wss://${hostHeader}/ws/relay?session=${session.id}&role=host`
    });
});

app.post('/api/sessions/:id/join', (req, res) => {
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });

    const body = req.body || {};
    let guestIp = masterClientIp(req);
    if (typeof body.guest_address === 'string' && net.isIP(body.guest_address.trim())) {
        const cand = body.guest_address.trim();
        if (!isPrivateIp(cand)) guestIp = cand;
        else if (isPrivateIp(guestIp)) guestIp = cand;
    }
    const guestPort = masterCleanInt(body.guest_port, 11774, 1, 65535);
    const guestMachineId = String(body.guest_machine_id || '');

    if (!session.pendingJoins) session.pendingJoins = [];
    session.pendingJoins.push({
        ip: guestIp,
        port: guestPort,
        machine_id: guestMachineId,
        timestamp: Date.now()
    });
    if (session.pendingJoins.length > 32) session.pendingJoins.shift();

    const hostHeader = req.headers.host || 'outbreak-z-test-server.onrender.com';
    res.json({
        success: true,
        session_id: session.id,
        host_ip: (guestIp === session.address && session.lanAddress) ? session.lanAddress : session.address,
        host_lan_ip: session.lanAddress || '',
        is_lan: (guestIp === session.address && !!session.lanAddress),
        host_port: session.port,
        relay_url: `wss://${hostHeader}/ws/relay?session=${session.id}&role=guest&peerId=${guestMachineId}`
    });
});

// ── Pre-Game Lobby Signaling Endpoints ─────────────────────────────────────
app.get('/api/sessions/:id/lobby', (req, res) => {
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });
    const guestIp = masterClientIp(req);
    res.json({
        success: true,
        session_id: session.id,
        name: session.name,
        map: session.map,
        mode: session.mode,
        variant: session.variant || '',
        difficulty: session.difficulty !== undefined ? session.difficulty : 1,
        skulls: session.skulls || '0',
        in_progress: session.inProgress === true,
        player_count: session.playerCount,
        max_players: session.maxPlayers,
        host_ip: (guestIp === session.address && session.lanAddress) ? session.lanAddress : session.address,
        host_lan_ip: session.lanAddress || '',
        is_lan: (guestIp === session.address && !!session.lanAddress),
        host_port: session.port,
        roster: session.roster || [],
        launch_data: session.launchData
    });
});

app.post('/api/sessions/:id/lobby/join', (req, res) => {
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });
    const guestIp = masterClientIp(req);

    const body = req.body || {};
    const peerId = body.peer_id || body.player_id || (session.roster.length + 1);
    const username = masterCleanText(body.username, 'Spartan', 24);
    const isDev = body.is_dev === true;
    const serviceTag = masterCleanText(body.service_tag, '117', 4);
    const machineId = String(body.machine_id || '');
    const ping = masterCleanInt(body.ping, 25, 0, 999);

    if (!session.roster) session.roster = [];
    const existingIdx = session.roster.findIndex(p =>
        (machineId && p.machineId === machineId) ||
        (peerId != 2 && p.peerId == peerId) ||
        (username !== 'Spartan' && username !== 'Guest' && p.username.toLowerCase() === username.toLowerCase())
    );
    const playerEntry = {
        peerId,
        machineId,
        username,
        isHost: false,
        isDev,
        serviceTag,
        ping,
        joinedAt: Date.now()
    };
    if (existingIdx >= 0) {
        session.roster[existingIdx] = playerEntry;
    } else {
        if (session.roster.length < session.maxPlayers) {
            session.roster.push(playerEntry);
        }
    }
    session.playerCount = session.roster.length;
    const guestPort = masterCleanInt(body.guest_port, 11774, 1, 65535);
    if (!session.pendingJoins) session.pendingJoins = [];
    session.pendingJoins.push({
        ip: guestIp,
        port: guestPort,
        machine_id: machineId,
        timestamp: Date.now()
    });
    if (session.pendingJoins.length > 32) session.pendingJoins.shift();

    broadcastLobbyEvent(session.id, {
        type: 'lobby_roster',
        roster: session.roster
    });

    const hostHeader = req.headers.host || 'outbreak-z-test-server.onrender.com';
    res.json({
        success: true,
        session_id: session.id,
        host_ip: (guestIp === session.address && session.lanAddress) ? session.lanAddress : session.address,
        host_lan_ip: session.lanAddress || '',
        is_lan: (guestIp === session.address && !!session.lanAddress),
        host_port: session.port,
        roster: session.roster,
        map: session.map,
        mode: session.mode,
        variant: session.variant || '',
        difficulty: session.difficulty !== undefined ? session.difficulty : 1,
        in_progress: session.inProgress === true,
        relay_url: `wss://${hostHeader}/ws/relay?session=${session.id}&role=guest&peerId=${machineId}`
    });
});

app.post('/api/sessions/:id/lobby/leave', (req, res) => {
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });
    const guestIp = masterClientIp(req);
    const body = req.body || {};
    const peerId = body.peer_id;
    const username = body.username ? String(body.username).toLowerCase() : '';

    if (session.roster) {
        session.roster = session.roster.filter(p => p.peerId != peerId && p.username.toLowerCase() !== username);
        session.playerCount = session.roster.length;
    }

    broadcastLobbyEvent(session.id, {
        type: 'lobby_roster',
        roster: session.roster || []
    });
    res.json({ success: true, roster: session.roster || [] });
});

app.post('/api/sessions/:id/lobby/launch', (req, res) => {
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });
    const guestIp = masterClientIp(req);
    if (!masterTokenMatches(req.get('X-Session-Token'), session.token)) {
        return res.status(403).json({ error: "Wrong token" });
    }
    const body = req.body || {};
    session.inProgress = true;
    session.launchData = {
        map: masterCleanText(body.map, session.map, 48),
        mode: masterCleanText(body.mode, session.mode, 24),
        variant: masterCleanText(body.variant, session.variant || '', 64),
        difficulty: masterCleanInt(body.difficulty, session.difficulty || 1, 0, 3),
        skulls: String(body.skulls || session.skulls || '0'),
        host_ip: (guestIp === session.address && session.lanAddress) ? session.lanAddress : session.address,
        host_lan_ip: session.lanAddress || '',
        is_lan: (guestIp === session.address && !!session.lanAddress),
        host_port: session.port,
        session_id: session.id
    };

    broadcastLobbyEvent(session.id, {
        type: 'lobby_launch',
        launch: session.launchData
    });

    res.json({ success: true, launch_data: session.launchData });
});



// ── Relay & Direct Connect Lobby Signaling ──────────────────────────────────
const lobbyRelays = new Map();
const LOBBY_RELAY_TIMEOUT_MS = 60_000;
const localPlayerStatsCache = new Map();

app.post('/api/lobbies/broadcast', (req, res) => {
    const ip = masterClientIp(req);
    // ip allowed
    const body = req.body || {};
    const lobbyId = body.lobby_id || crypto.randomBytes(6).toString('hex');
    const port = masterCleanInt(body.port, 11774, 1, 65535);
    const entry = {
        lobbyId,
        hostIp: ip,
        port,
        name: masterCleanText(body.name, 'Reach Lobby', 48),
        map: masterCleanText(body.map, 'Unknown map', 48),
        mode: masterCleanText(body.mode, 'Custom Game', 24),
        playerCount: masterCleanInt(body.player_count, 1, 1, 64),
        maxPlayers: masterCleanInt(body.max_players, 16, 2, 64),
        isMod: body.is_mod === true || String(body.name || '').includes('[Mod]'),
        hasPassword: body.has_password === true,
        connectString: ip + ':' + port,
        lastSeen: Date.now()
    };
    lobbyRelays.set(lobbyId, entry);
    res.json({ success: true, lobby: entry });
});

app.get('/api/lobbies', (req, res) => {
    const now = Date.now();
    const active = [];
    for (const [id, lobby] of lobbyRelays.entries()) {
        if (now - lobby.lastSeen <= LOBBY_RELAY_TIMEOUT_MS) {
            active.push(lobby);
        } else {
            lobbyRelays.delete(id);
        }
    }
    res.set('Cache-Control', 'no-store').json(active);
});

// Local player stats share/load for other clients
app.post('/api/stats/local-sync', (req, res) => {
    const { playerName, serviceTag, stats } = req.body || {};
    if (!playerName) return res.status(400).json({ error: 'Player name required' });
    const key = String(playerName).toLowerCase().trim();
    localPlayerStatsCache.set(key, {
        playerName,
        serviceTag: serviceTag || 'NOBL',
        stats: stats || {},
        updatedAt: Date.now()
    });
    res.json({ success: true, message: 'Local stats synced for other clients' });
});

app.get('/api/stats/player/:playerName', (req, res) => {
    const key = String(req.params.playerName || '').toLowerCase().trim();
    const cached = localPlayerStatsCache.get(key);
    if (!cached) {
        return res.status(404).json({ error: 'Player stats not found' });
    }
    res.json({ success: true, player: cached });
});

app.delete('/api/sessions/:id', (req, res) => {
    const ip = masterClientIp(req);
    // ip allowed
    if (masterRateLimited(ip)) return res.status(429).json({ error: "Too many requests" });
    const session = masterSessions.get(String(req.params.id));
    if (!session) return res.status(404).json({ error: "Session not found" });
    if (!masterTokenMatches(req.get('X-Session-Token'), session.token)) {
        return res.status(403).json({ error: "Wrong token" });
    }
    broadcastLobbyEvent(session.id, { type: 'lobby_closed', reason: 'Host closed the lobby' });
    masterSessions.delete(session.id);
    res.json({ success: true });
});

// ── Auth Endpoints ───────────────────────────────────────────────────────────

app.post('/api/register', (req, res) => {
    const { username, password } = req.body;
    if (!username || !password) {
        return res.status(400).json({ error: "Username and password required" });
    }
    if (username.length > 14) {
        return res.status(400).json({ error: "Username must be 14 characters or fewer" });
    }

    bcrypt.hash(password, 10, (err, hash) => {
        if (err) {
            console.error("bcrypt.hash error:", err.message);
            return res.status(500).json({ error: "Failed to encrypt password" });
        }

        db.run(`INSERT INTO users (username, password) VALUES (?, ?)`, [username, hash], function(err) {
            if (err) {
                if (err.message.includes("UNIQUE")) {
                    return res.status(400).json({ error: "Username already taken" });
                }
                console.error("Register insert error:", err.message);
                return res.status(500).json({ error: err.message });
            }

            const userId = this.lastID;

            // Create default stats row
            db.run(`INSERT INTO stats (user_id) VALUES (?)`, [userId], (err) => {
                if (err) console.error("stats insert error for userId", userId, err.message);
            });
            // Create default cosmetics row
            db.run(`INSERT INTO cosmetics (user_id) VALUES (?)`, [userId], (err) => {
                if (err) console.error("cosmetics insert error for userId", userId, err.message);
            });
            // Unlock default item
            db.run(`INSERT INTO unlocked_items (user_id, item_id) VALUES (?, ?)`, [userId, 'Default'], (err) => {
                if (err) console.error("unlock_items insert error for userId", userId, err.message);
            });

            res.json({ message: "Registration successful!", userId });
        });
    });
});

app.post('/api/login', (req, res) => {
    const { username, password, force } = req.body;
    if (!username || !password) {
        return res.status(400).json({ error: "Username and password required" });
    }

    db.get(`SELECT * FROM users WHERE username = ?`, [username], (err, user) => {
        if (err) {
            console.error("Login DB error:", err.message);
            return res.status(500).json({ error: "Database error" });
        }
        if (!user) return res.status(401).json({ error: "Invalid username or password" });

        bcrypt.compare(password, user.password, (bcryptErr, match) => {
            if (bcryptErr) {
                console.error("bcrypt.compare error:", bcryptErr.message);
                return res.status(500).json({ error: "Authentication error" });
            }
            if (!match) return res.status(401).json({ error: "Invalid username or password" });

            // Fetch stats
            db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (statsErr, stats) => {
                if (statsErr) console.error("Login stats fetch error:", statsErr.message);

                if (stats) {
                    stats.challenge_progress = safeParse(stats.challenge_progress, {});
                    stats.completed_challenges = safeParse(stats.completed_challenges, []);
                    stats.weapon_prestige = safeParse(stats.weapon_prestige, {});
                    stats.weapon_xp = safeParse(stats.weapon_xp, {});
                    stats.weapon_levels = safeParse(stats.weapon_levels, {});
                    stats.unlocked_camos = safeParse(stats.unlocked_camos, ["Default"]);
                    stats.unlocked_titles = safeParse(stats.unlocked_titles, ["Survivor"]);
                    stats.unlocked_calling_cards = safeParse(stats.unlocked_calling_cards, ["Default"]);
                    stats.unlocked_skins = safeParse(stats.unlocked_skins, ["Player Starter"]);
                    stats.unlocked_capes = safeParse(stats.unlocked_capes, ["Default", "Golden Cape"]);
                    stats.weapon_attachments = safeParse(stats.weapon_attachments, {});
                    stats.weapon_camos = safeParse(stats.weapon_camos, {});
                    stats.story_checkpoints = safeParse(stats.story_checkpoints, {});
                    stats.completed_story_missions = safeParse(stats.completed_story_missions, []);
                    stats.story_missions_completed = Number(stats.story_missions_completed || (Array.isArray(stats.completed_story_missions) ? stats.completed_story_missions.length : 0));
                    stats.grenade_kills = Number(stats.grenade_kills || 0);
                    stats.pvp_primary = stats.pvp_primary || 'AK-47';
                    stats.pvp_secondary = stats.pvp_secondary || 'M1911';
                }

                // Fetch unlocks
                db.all(`SELECT item_id FROM unlocked_items WHERE user_id = ?`, [user.id], (unlocksErr, items) => {
                    if (unlocksErr) console.error("Login unlocks fetch error:", unlocksErr.message);

                    // Fetch cosmetics
                    db.get(`SELECT * FROM cosmetics WHERE user_id = ?`, [user.id], (cosmeticsErr, cosmetics) => {
                        if (cosmeticsErr) console.error("Login cosmetics fetch error:", cosmeticsErr.message);

                        if (user.is_developer === 1 && stats) {
                            const ensureUnlockedItem = (target, item) => {
                                if (!target) return;
                                if (Array.isArray(target)) {
                                    if (!target.includes(item)) target.push(item);
                                } else if (typeof target === 'object') {
                                    if (!target.global) target.global = ["Default"];
                                    if (Array.isArray(target.global) && !target.global.includes(item)) target.global.push(item);
                                }
                            };
                            ensureUnlockedItem(stats.unlocked_skins, "Developer");
                            ensureUnlockedItem(stats.unlocked_camos, "Developer Matrix");
                            ensureUnlockedItem(stats.unlocked_calling_cards, "Developer");
                            ensureUnlockedItem(stats.unlocked_capes, "Developer Cape");
                            ensureUnlockedItem(stats.unlocked_titles, "Developer");
                        }

                        // Smart active session check
                        // If session has not pinged in 75 seconds or force=true is passed, supersede it cleanly
                        if (activeSessions[String(user.id)]) {
                            const existing = activeSessions[String(user.id)];
                            const timeSinceLastSeen = Date.now() - existing.lastSeen;
                            if (timeSinceLastSeen > 75 * 1000 || force) {
                                delete activeSessions[String(user.id)];
                            } else {
                                return res.status(409).json({
                                    error: "This account is already logged in on another device. Log out on your other device to continue."
                                });
                            }
                        }

                        // Issue session token for this new login
                        const sessionToken = generateSessionToken();
                        activeSessions[String(user.id)] = {
                            token: sessionToken,
                            username: user.username,
                            lastSeen: Date.now()
                        };

                        res.json({
                            message: "Login successful!",
                            userId: user.id,
                            username: user.username,
                            session_token: sessionToken,
                            is_developer: Boolean(user.is_developer === 1),
                            stats: stats || {},
                            unlocks: (items || []).map(i => i.item_id),
                            cosmetics: {
                                selected_skin: (cosmetics && cosmetics.selected_skin) || 'Default',
                                selected_title: (cosmetics && cosmetics.selected_title) || 'Survivor',
                                selected_calling_card: (cosmetics && cosmetics.selected_calling_card) || 'Default',
                                selected_cape: (cosmetics && cosmetics.selected_cape) || 'Default',
                                selected_camo: (cosmetics && cosmetics.selected_camo) || 'Default'
                            }
                        });
                    });
                });
            });
        });
    });
});

// ── Developer Accounts API ───────────────────────────────────────────────────

app.post('/api/dev/set-developer', (req, res) => {
    const { username, isDeveloper } = req.body;
    if (!username) return res.status(400).json({ error: "Username required" });
    const devVal = isDeveloper ? 1 : 0;
    db.run(`UPDATE users SET is_developer = ? WHERE username = ?`, [devVal, username], function(err) {
        if (err) return res.status(500).json({ error: err.message });
        if (this.changes === 0) return res.status(404).json({ error: "User not found" });

        if (devVal === 1) {
            db.get(`SELECT id FROM users WHERE username = ?`, [username], (uErr, user) => {
                if (user) {
                    db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
                        if (stats) {
                            let skins = safeParse(stats.unlocked_skins, ["Player Starter"]);
                            let camos = safeParse(stats.unlocked_camos, ["Default"]);
                            let cards = safeParse(stats.unlocked_calling_cards, ["Default"]);
                            let capes = safeParse(stats.unlocked_capes, ["Default", "Golden Cape"]);
                            let titles = safeParse(stats.unlocked_titles, ["Survivor"]);

                            const pushDev = (target, val) => {
                                if (Array.isArray(target) && !target.includes(val)) target.push(val);
                                else if (typeof target === 'object' && target !== null) {
                                    if (!target.global) target.global = ["Default"];
                                    if (Array.isArray(target.global) && !target.global.includes(val)) target.global.push(val);
                                }
                            };
                            pushDev(skins, "Developer");
                            pushDev(camos, "Developer Matrix");
                            pushDev(cards, "Developer");
                            pushDev(capes, "Developer Cape");
                            pushDev(titles, "Developer");

                            db.run(`UPDATE stats SET unlocked_skins = ?, unlocked_camos = ?, unlocked_calling_cards = ?, unlocked_capes = ?, unlocked_titles = ? WHERE user_id = ?`,
                                [JSON.stringify(skins), JSON.stringify(camos), JSON.stringify(cards), JSON.stringify(capes), JSON.stringify(titles), user.id]);
                        }
                    });
                }
            });
        }
        res.json({ message: `Developer status for ${username} set to ${Boolean(devVal)}` });
    });
});

// ── Logout ───────────────────────────────────────────────────────────────────

app.post('/api/logout', (req, res) => {
    const userId = String(req.body.userId || '');
    if (userId) invalidateSession(userId);
    res.json({ message: "Logged out successfully" });
});

// ── Session Ping (keepalive) ──────────────────────────────────────────────────
// Client calls this every 60s while logged in to prevent idle expiry.

app.post('/api/session/ping', (req, res) => {
    const userId = String(req.body.userId || '');
    const token  = String(req.body.session_token || '');
    if (!userId || !token) return res.status(400).json({ error: "userId and session_token required" });
    const session = activeSessions[userId];
    if (!session || session.token !== token) {
        return res.status(401).json({ alive: false, error: "Session expired. Please log in again." });
    }
    session.lastSeen = Date.now();
    res.json({ alive: true });
});

app.post('/api/dev/grant-item', (req, res) => {
    const { username, category, itemName } = req.body;
    if (!username || !category || !itemName) {
        return res.status(400).json({ error: "username, category, and itemName are required" });
    }
    const catMap = {
        'skin': 'unlocked_skins', 'player_skin': 'unlocked_skins', 'skins': 'unlocked_skins',
        'cape': 'unlocked_capes', 'player_cape': 'unlocked_capes', 'capes': 'unlocked_capes',
        'camo': 'unlocked_camos', 'weapon_camo': 'unlocked_camos', 'camos': 'unlocked_camos',
        'title': 'unlocked_titles', 'player_title': 'unlocked_titles', 'titles': 'unlocked_titles',
        'card': 'unlocked_calling_cards', 'calling_card': 'unlocked_calling_cards', 'cards': 'unlocked_calling_cards', 'calling_cards': 'unlocked_calling_cards',
        'other': 'completed_challenges', 'challenge': 'completed_challenges', 'challenges': 'completed_challenges'
    };
    const colName = catMap[category.toLowerCase().trim()];
    if (!colName) return res.status(400).json({ error: "Invalid category" });

    db.get(`SELECT id, username FROM users WHERE LOWER(username) = LOWER(?)`, [username.trim()], (uErr, user) => {
        if (uErr || !user) return res.status(404).json({ error: `User '${username}' not found` });
        db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
            if (sErr || !stats) return res.status(404).json({ error: "Stats not found for user" });
            let rawVal = safeParse(stats[colName], []);
            
            if (colName === 'unlocked_camos' && typeof rawVal === 'object' && !Array.isArray(rawVal)) {
                if (!rawVal.global) rawVal.global = ["Default"];
                if (!rawVal.global.includes(itemName)) rawVal.global.push(itemName);
            } else {
                if (!Array.isArray(rawVal)) rawVal = [];
                if (!rawVal.includes(itemName)) rawVal.push(itemName);
            }

            db.run(`UPDATE stats SET ${colName} = ? WHERE user_id = ?`, [JSON.stringify(rawVal), user.id], (upErr) => {
                if (upErr) return res.status(500).json({ error: upErr.message });
                db.run(`INSERT OR IGNORE INTO unlocked_items (user_id, item_id) VALUES (?, ?)`, [user.id, itemName]);
                res.json({ message: `Successfully granted '${itemName}' (${category}) to ${user.username}` });
            });
        });
    });
});

app.post('/api/dev/grant-item-all', (req, res) => {
    const { category, itemName } = req.body;
    if (!category || !itemName) {
        return res.status(400).json({ error: "category and itemName are required" });
    }
    const catMap = {
        'skin': 'unlocked_skins', 'player_skin': 'unlocked_skins', 'skins': 'unlocked_skins',
        'cape': 'unlocked_capes', 'player_cape': 'unlocked_capes', 'capes': 'unlocked_capes',
        'camo': 'unlocked_camos', 'weapon_camo': 'unlocked_camos', 'camos': 'unlocked_camos',
        'title': 'unlocked_titles', 'player_title': 'unlocked_titles', 'titles': 'unlocked_titles',
        'card': 'unlocked_calling_cards', 'calling_card': 'unlocked_calling_cards', 'cards': 'unlocked_calling_cards', 'calling_cards': 'unlocked_calling_cards',
        'other': 'completed_challenges', 'challenge': 'completed_challenges', 'challenges': 'completed_challenges'
    };
    const colName = catMap[category.toLowerCase().trim()];
    if (!colName) return res.status(400).json({ error: "Invalid category" });

    db.all(`SELECT user_id, ${colName} FROM stats`, [], (err, rows) => {
        if (err) return res.status(500).json({ error: err.message });
        let count = 0;
        const stmt = db.prepare(`UPDATE stats SET ${colName} = ? WHERE user_id = ?`);
        const itemStmt = db.prepare(`INSERT OR IGNORE INTO unlocked_items (user_id, item_id) VALUES (?, ?)`);

        db.serialize(() => {
            rows.forEach((row) => {
                let rawVal = safeParse(row[colName], []);
                let modified = false;
                if (colName === 'unlocked_camos' && typeof rawVal === 'object' && !Array.isArray(rawVal)) {
                    if (!rawVal.global) rawVal.global = ["Default"];
                    if (!rawVal.global.includes(itemName)) {
                        rawVal.global.push(itemName);
                        modified = true;
                    }
                } else {
                    if (!Array.isArray(rawVal)) rawVal = [];
                    if (!rawVal.includes(itemName)) {
                        rawVal.push(itemName);
                        modified = true;
                    }
                }
                if (modified) {
                    stmt.run([JSON.stringify(rawVal), row.user_id]);
                    itemStmt.run([row.user_id, itemName]);
                    count++;
                }
            });
            stmt.finalize();
            itemStmt.finalize();
            res.json({ message: `Successfully unlocked '${itemName}' for all ${rows.length} players in database (${count} updated)` });
        });
    });
});

app.post('/api/dev/revoke-item', (req, res) => {
    const { username, category, itemName } = req.body;
    if (!username || !category || !itemName) {
        return res.status(400).json({ error: "username, category, and itemName are required" });
    }
    const catMap = {
        'skin': 'unlocked_skins', 'player_skin': 'unlocked_skins', 'skins': 'unlocked_skins',
        'cape': 'unlocked_capes', 'player_cape': 'unlocked_capes', 'capes': 'unlocked_capes',
        'camo': 'unlocked_camos', 'weapon_camo': 'unlocked_camos', 'camos': 'unlocked_camos',
        'title': 'unlocked_titles', 'player_title': 'unlocked_titles', 'titles': 'unlocked_titles',
        'card': 'unlocked_calling_cards', 'calling_card': 'unlocked_calling_cards', 'cards': 'unlocked_calling_cards', 'calling_cards': 'unlocked_calling_cards',
        'other': 'completed_challenges', 'challenge': 'completed_challenges', 'challenges': 'completed_challenges'
    };
    const colName = catMap[category.toLowerCase().trim()];
    if (!colName) return res.status(400).json({ error: "Invalid category" });

    db.get(`SELECT id, username FROM users WHERE LOWER(username) = LOWER(?)`, [username.trim()], (uErr, user) => {
        if (uErr || !user) return res.status(404).json({ error: `User '${username}' not found` });
        db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
            if (sErr || !stats) return res.status(404).json({ error: "Stats not found for user" });
            let rawVal = safeParse(stats[colName], []);
            if (colName === 'unlocked_camos' && typeof rawVal === 'object' && !Array.isArray(rawVal)) {
                if (Array.isArray(rawVal.global)) {
                    rawVal.global = rawVal.global.filter(i => i !== itemName);
                }
            } else if (Array.isArray(rawVal)) {
                rawVal = rawVal.filter(i => i !== itemName);
            }
            db.run(`UPDATE stats SET ${colName} = ? WHERE user_id = ?`, [JSON.stringify(rawVal), user.id], (upErr) => {
                if (upErr) return res.status(500).json({ error: upErr.message });
                db.run(`DELETE FROM unlocked_items WHERE user_id = ? AND item_id = ?`, [user.id, itemName]);
                res.json({ message: `Successfully revoked '${itemName}' from ${user.username}` });
            });
        });
    });
});

// Hardcoded fallback registry of developer and special items
const KNOWN_SPECIAL_AND_DEV_ITEMS = {
    skin: ["Developer"],
    player_skin: ["Developer"],
    cape: ["Developer Cape", "Froggy's Dev Cape", "FroggyDevCape", "Primitives", "Primitives Cape", "Playtester", "Playtester Cape", "Froggy Cape", "Sunflower", "Sunflower Cape"],
    player_cape: ["Developer Cape", "Froggy's Dev Cape", "FroggyDevCape", "Primitives", "Primitives Cape", "Playtester", "Playtester Cape", "Froggy Cape", "Sunflower", "Sunflower Cape"],
    camo: ["Developer Matrix", "Froggy's Dev", "FroggysDev", "Developer Checkerboard", "DeveloperCheckerboard", "Developer Camo", "DeveloperCamo", "Froggys", "Duckys"],
    weapon_camo: ["Developer Matrix", "Froggy's Dev", "FroggysDev", "Developer Checkerboard", "DeveloperCheckerboard", "Developer Camo", "DeveloperCamo", "Froggys", "Duckys"],
    card: ["Developer"],
    calling_card: ["Developer"],
    title: ["Developer", "Playtester"],
    player_title: ["Developer", "Playtester"]
};

// Helper to filter unlocked cosmetics, keeping default items and gifted special/dev items
function filterKeptCosmetics(existingRaw, category, isDev, clientSpecialMap) {
    const catLower = category.toLowerCase().trim();
    let defaultItem = "Default";
    let devDefault = "";

    switch (catLower) {
        case 'skin':
        case 'skins':
        case 'player_skin':
            defaultItem = "Player Starter";
            if (isDev) devDefault = "Developer";
            break;
        case 'cape':
        case 'capes':
        case 'player_cape':
            defaultItem = "Default";
            if (isDev) devDefault = "Developer Cape";
            break;
        case 'camo':
        case 'camos':
        case 'weapon_camo':
            defaultItem = "Default";
            if (isDev) devDefault = "Developer Matrix";
            break;
        case 'title':
        case 'titles':
        case 'player_title':
            defaultItem = "Survivor";
            if (isDev) devDefault = "Developer";
            break;
        case 'card':
        case 'cards':
        case 'calling_card':
        case 'calling_cards':
            defaultItem = "Default";
            if (isDev) devDefault = "Developer";
            break;
    }

    // Build set of special items to preserve for this category
    const specialSet = new Set();
    const knownList = KNOWN_SPECIAL_AND_DEV_ITEMS[catLower] || [];
    for (const item of knownList) {
        specialSet.add(item.toLowerCase().trim());
    }

    if (clientSpecialMap && typeof clientSpecialMap === 'object') {
        const clientList = clientSpecialMap[catLower] || clientSpecialMap[category] || [];
        if (Array.isArray(clientList)) {
            for (const item of clientList) {
                if (typeof item === 'string') specialSet.add(item.toLowerCase().trim());
            }
        }
    }

    const isSpecialOrDev = (itemName) => {
        if (!itemName || typeof itemName !== 'string') return false;
        const low = itemName.toLowerCase().trim();
        if (specialSet.has(low)) return true;
        if (low.includes("developer") || low.includes("dev_only") || low.includes("playtester") || low.includes("froggy")) {
            return true;
        }
        return false;
    };

    if ((catLower === 'camo' || catLower === 'weapon_camo') && typeof existingRaw === 'object' && !Array.isArray(existingRaw) && existingRaw !== null) {
        const resultDict = {};
        const glob = Array.isArray(existingRaw.global) ? existingRaw.global : ["Default"];
        const keptGlob = [defaultItem];
        if (devDefault && !keptGlob.includes(devDefault)) keptGlob.push(devDefault);

        for (const itm of glob) {
            if (typeof itm !== 'string') continue;
            if (itm === defaultItem || itm === devDefault) continue;
            if (isSpecialOrDev(itm)) {
                if (!keptGlob.includes(itm)) keptGlob.push(itm);
            }
        }
        resultDict.global = keptGlob;

        for (const [wKey, wVal] of Object.entries(existingRaw)) {
            if (wKey === 'global') continue;
            if (Array.isArray(wVal)) {
                const keptW = [];
                for (const itm of wVal) {
                    if (typeof itm !== 'string') continue;
                    if (isSpecialOrDev(itm)) {
                        if (!keptW.includes(itm)) keptW.push(itm);
                    }
                }
                if (keptW.length > 0) {
                    resultDict[wKey] = keptW;
                }
            }
        }
        return resultDict;
    }

    const itemsToCheck = Array.isArray(existingRaw) ? existingRaw : [];
    const kept = [defaultItem];
    if ((catLower === 'cape' || catLower === 'player_cape') && !kept.includes("Golden Cape")) {
        kept.push("Golden Cape");
    }
    if (devDefault && !kept.includes(devDefault)) {
        kept.push(devDefault);
    }

    for (const itm of itemsToCheck) {
        if (typeof itm !== 'string') continue;
        if (kept.includes(itm)) continue;
        if (isSpecialOrDev(itm)) {
            kept.push(itm);
        }
    }

    return kept;
}

// Internal helper to reset a single user while preserving special/dev items
function resetUserStatsInternal(dbInstance, user, currentStats, currentCosmetics, clientSpecialMap, callback) {
    const isDev = user.is_developer === 1;

    const rawSkins = safeParse(currentStats ? currentStats.unlocked_skins : null, isDev ? ["Player Starter", "Developer"] : ["Player Starter"]);
    const rawCapes = safeParse(currentStats ? currentStats.unlocked_capes : null, isDev ? ["Default", "Golden Cape", "Developer Cape"] : ["Default", "Golden Cape"]);
    const rawCamos = safeParse(currentStats ? currentStats.unlocked_camos : null, isDev ? ["Default", "Developer Matrix"] : ["Default"]);
    const rawTitles = safeParse(currentStats ? currentStats.unlocked_titles : null, isDev ? ["Survivor", "Developer"] : ["Survivor"]);
    const rawCards = safeParse(currentStats ? currentStats.unlocked_calling_cards : null, isDev ? ["Default", "Developer"] : ["Default"]);

    const keptSkins = filterKeptCosmetics(rawSkins, 'skin', isDev, clientSpecialMap);
    const keptCapes = filterKeptCosmetics(rawCapes, 'cape', isDev, clientSpecialMap);
    const keptCamos = filterKeptCosmetics(rawCamos, 'camo', isDev, clientSpecialMap);
    const keptTitles = filterKeptCosmetics(rawTitles, 'title', isDev, clientSpecialMap);
    const keptCards = filterKeptCosmetics(rawCards, 'card', isDev, clientSpecialMap);

    // Collect all kept items for unlocked_items table
    const allKeptItems = new Set(["Default"]);
    for (const s of keptSkins) allKeptItems.add(s);
    for (const c of keptCapes) allKeptItems.add(c);
    for (const t of keptTitles) allKeptItems.add(t);
    for (const cd of keptCards) allKeptItems.add(cd);

    if (Array.isArray(keptCamos)) {
        for (const cm of keptCamos) allKeptItems.add(cm);
    } else if (typeof keptCamos === 'object' && keptCamos !== null) {
        if (Array.isArray(keptCamos.global)) {
            for (const cm of keptCamos.global) allKeptItems.add(cm);
        }
        for (const [wKey, wArr] of Object.entries(keptCamos)) {
            if (wKey !== 'global' && Array.isArray(wArr)) {
                for (const cm of wArr) allKeptItems.add(cm);
            }
        }
    }

    // Determine safe equipped cosmetics
    let selSkin = currentCosmetics && currentCosmetics.selected_skin ? currentCosmetics.selected_skin : "Player Starter";
    if (!keptSkins.includes(selSkin)) selSkin = "Player Starter";

    let selCape = currentCosmetics && currentCosmetics.selected_cape ? currentCosmetics.selected_cape : "Golden Cape";
    if (!keptCapes.includes(selCape)) selCape = "Golden Cape";

    let selTitle = currentCosmetics && currentCosmetics.selected_title ? currentCosmetics.selected_title : "Survivor";
    if (!keptTitles.includes(selTitle)) selTitle = "Survivor";

    let selCard = currentCosmetics && currentCosmetics.selected_calling_card ? currentCosmetics.selected_calling_card : "Default";
    if (!keptCards.includes(selCard)) selCard = "Default";

    let selCamo = currentCosmetics && currentCosmetics.selected_camo ? currentCosmetics.selected_camo : "Default";
    let camoAvailable = false;
    if (Array.isArray(keptCamos)) {
        camoAvailable = keptCamos.includes(selCamo);
    } else if (typeof keptCamos === 'object' && keptCamos !== null && Array.isArray(keptCamos.global)) {
        camoAvailable = keptCamos.global.includes(selCamo);
    }
    if (!camoAvailable) selCamo = "Default";

    dbInstance.serialize(() => {
        dbInstance.run(`UPDATE stats SET 
            kills = 0,
            headshots = 0,
            rounds_survived = 0,
            downs = 0,
            revives = 0,
            revived_count = 0,
            downed_count = 0,
            highest_round = 0,
            highest_points = 0,
            level = 1,
            prestige = 0,
            xp = 0,
            boss_kills = 0,
            elemental_kills = 0,
            grenade_kills = 0,
            flags_captured = 0,
            players_killed = 0,
            domination_wins = 0,
            domination_losses = 0,
            tdm_wins = 0,
            tdm_losses = 0,
            ffa_wins = 0,
            ffa_losses = 0,
            story_missions_completed = 0,
            story_checkpoints = '{}',
            completed_story_missions = '[]',
            pvp_primary = 'AK-47',
            pvp_secondary = 'M1911',
            challenge_progress = '{}',
            completed_challenges = '[]',
            weapon_prestige = '{}',
            weapon_xp = '{}',
            weapon_levels = '{}',
            weapon_attachments = '{}',
            weapon_camos = '{}',
            unlocked_camos = ?,
            unlocked_titles = ?,
            unlocked_calling_cards = ?,
            unlocked_skins = ?,
            unlocked_capes = ?
            WHERE user_id = ?`,
            [
                JSON.stringify(keptCamos),
                JSON.stringify(keptTitles),
                JSON.stringify(keptCards),
                JSON.stringify(keptSkins),
                JSON.stringify(keptCapes),
                user.id
            ],
            (statErr) => {
                if (statErr) return callback(statErr);

                dbInstance.run(`UPDATE cosmetics SET
                    selected_skin = ?,
                    selected_title = ?,
                    selected_calling_card = ?,
                    selected_cape = ?,
                    selected_camo = ?
                    WHERE user_id = ?`,
                    [selSkin, selTitle, selCard, selCape, selCamo, user.id],
                    (cosErr) => {
                        if (cosErr) return callback(cosErr);

                        dbInstance.run(`DELETE FROM unlocked_items WHERE user_id = ?`, [user.id], (delErr) => {
                            if (delErr) return callback(delErr);

                            const insStmt = dbInstance.prepare(`INSERT OR IGNORE INTO unlocked_items (user_id, item_id) VALUES (?, ?)`);
                            allKeptItems.forEach((itm) => {
                                insStmt.run([user.id, itm]);
                            });
                            insStmt.finalize(callback);
                        });
                    }
                );
            }
        );
    });
}

// Reset stats for a single user (preserves special and dev items)
app.post('/api/dev/reset-user-stats', (req, res) => {
    const { username, userId, special_items, specialItems } = req.body;
    if (!username && !userId) {
        return res.status(400).json({ error: "username or userId is required" });
    }

    const clientSpecialMap = special_items || specialItems || null;

    let userQuery = "SELECT id, username, is_developer FROM users WHERE ";
    let userParam = [];
    if (userId) {
        userQuery += "id = ?";
        userParam.push(userId);
    } else {
        userQuery += "username = ?";
        userParam.push(username);
    }

    db.get(userQuery, userParam, (uErr, user) => {
        if (uErr || !user) return res.status(404).json({ error: "User not found" });

        db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
            db.get(`SELECT * FROM cosmetics WHERE user_id = ?`, [user.id], (cErr, cosmetics) => {
                resetUserStatsInternal(db, user, stats, cosmetics, clientSpecialMap, (err) => {
                    if (err) return res.status(500).json({ error: err.message });
                    res.json({ message: `Successfully reset weapon levels, rank & stats for '${user.username}' back to Level 1 while keeping special and dev items!` });
                });
            });
        });
    });
});

// Reset stats for ALL players in database (preserves special and dev items)
app.post('/api/dev/reset-all-players', (req, res) => {
    const { special_items, specialItems } = req.body || {};
    const clientSpecialMap = special_items || specialItems || null;

    db.all(`SELECT u.id, u.username, u.is_developer, s.unlocked_skins, s.unlocked_capes, s.unlocked_camos, s.unlocked_titles, s.unlocked_calling_cards, c.selected_skin, c.selected_cape, c.selected_title, c.selected_calling_card, c.selected_camo
            FROM users u
            LEFT JOIN stats s ON u.id = s.user_id
            LEFT JOIN cosmetics c ON u.id = c.user_id`, [], (err, rows) => {
        if (err) return res.status(500).json({ error: err.message });
        if (!rows || rows.length === 0) {
            return res.json({ message: "No registered players found in database to reset." });
        }

        let completed = 0;
        let hasError = false;

        rows.forEach((row) => {
            const user = { id: row.id, username: row.username, is_developer: row.is_developer };
            const stats = {
                unlocked_skins: row.unlocked_skins,
                unlocked_capes: row.unlocked_capes,
                unlocked_camos: row.unlocked_camos,
                unlocked_titles: row.unlocked_titles,
                unlocked_calling_cards: row.unlocked_calling_cards
            };
            const cosmetics = {
                selected_skin: row.selected_skin,
                selected_cape: row.selected_cape,
                selected_title: row.selected_title,
                selected_calling_card: row.selected_calling_card,
                selected_camo: row.selected_camo
            };

            resetUserStatsInternal(db, user, stats, cosmetics, clientSpecialMap, (uErr) => {
                if (hasError) return;
                if (uErr) {
                    hasError = true;
                    console.error(`Error resetting player ${user.username}:`, uErr.message);
                    return res.status(500).json({ error: `Error resetting player ${user.username}: ${uErr.message}` });
                }
                completed++;
                if (completed === rows.length) {
                    res.json({ message: `Successfully reset weapon levels, player ranks & leaderboard stats for all ${rows.length} players while keeping special and dev items!` });
                }
            });
        });
    });
});


app.post('/api/dev/check', (req, res) => {
    const { userId, username } = req.body;
    if (!userId && !username) return res.json({ is_developer: false });

    let query = "SELECT is_developer FROM users WHERE ";
    let params = [];
    if (userId) {
        query += "id = ?";
        params.push(userId);
    } else {
        query += "username = ?";
        params.push(username);
    }

    db.get(query, params, (err, user) => {
        if (err || !user) return res.json({ is_developer: false });
        res.json({ is_developer: Boolean(user.is_developer === 1) });
    });
});

// ── Stats Endpoints ──────────────────────────────────────────────────────────

app.post('/api/stats', (req, res) => {
    const {
        userId, kills, headshots, rounds_survived, downs, revives, revived_count, downed_count, highest_round, highest_points, level, prestige, xp,
        boss_kills, elemental_kills, flags_captured, players_killed,
        domination_wins, domination_losses, tdm_wins, tdm_losses, ffa_wins, ffa_losses,
        challenge_progress, completed_challenges, weapon_prestige, weapon_xp, weapon_levels,
        unlocked_camos, unlocked_titles, unlocked_calling_cards, unlocked_skins, unlocked_capes,
        weapon_attachments, weapon_camos,
        story_checkpoints, completed_story_missions, story_missions_completed, grenade_kills, pvp_primary, pvp_secondary
    } = req.body;

    if (!userId) return res.status(400).json({ error: "User ID required" });
    if (!validateSession(req, res)) return;

    const safeStr = (val, defaultVal = '{}') => {
        if (val === null || val === undefined) return defaultVal;
        if (typeof val === 'object') return JSON.stringify(val);
        return val;
    };

    const storyArr = Array.isArray(completed_story_missions)
        ? completed_story_missions
        : (typeof completed_story_missions === 'string' ? safeParse(completed_story_missions, []) : []);
    const resolvedStoryCount = Math.max(
        Number(story_missions_completed || 0),
        storyArr.length
    );

    db.run(`UPDATE stats SET 
        kills = MAX(kills, ?), 
        headshots = MAX(headshots, ?), 
        rounds_survived = MAX(rounds_survived, ?),
        downs = MAX(downs, ?),
        revives = MAX(revives, ?),
        revived_count = MAX(revived_count, ?),
        downed_count = MAX(downed_count, ?),
        highest_round = MAX(highest_round, ?),
        highest_points = MAX(highest_points, ?),
        level = ?,
        prestige = ?,
        xp = ?,
        boss_kills = MAX(boss_kills, ?),
        elemental_kills = MAX(elemental_kills, ?),
        flags_captured = MAX(flags_captured, ?),
        players_killed = MAX(players_killed, ?),
        domination_wins = MAX(domination_wins, ?),
        domination_losses = MAX(domination_losses, ?),
        tdm_wins = MAX(tdm_wins, ?),
        tdm_losses = MAX(tdm_losses, ?),
        ffa_wins = MAX(ffa_wins, ?),
        ffa_losses = MAX(ffa_losses, ?),
        grenade_kills = MAX(grenade_kills, ?),
        story_missions_completed = MAX(story_missions_completed, ?),
        challenge_progress = ?,
        completed_challenges = ?,
        weapon_prestige = ?,
        weapon_xp = ?,
        weapon_levels = ?,
        unlocked_camos = ?,
        unlocked_titles = ?,
        unlocked_calling_cards = ?,
        unlocked_skins = ?,
        unlocked_capes = ?,
        weapon_attachments = ?,
        weapon_camos = ?,
        story_checkpoints = ?,
        completed_story_missions = ?,
        pvp_primary = COALESCE(?, pvp_primary),
        pvp_secondary = COALESCE(?, pvp_secondary)
        WHERE user_id = ?`,
        [
            kills || 0,
            headshots || 0,
            rounds_survived || 0,
            downs || 0,
            revives || 0,
            revived_count || 0,
            downed_count || 0,
            highest_round || 0,
            highest_points || 0,
            level !== undefined ? level : 1,
            prestige || 0,
            xp || 0,
            boss_kills || 0,
            elemental_kills || 0,
            flags_captured || 0,
            players_killed || 0,
            domination_wins || 0,
            domination_losses || 0,
            tdm_wins || 0,
            tdm_losses || 0,
            ffa_wins || 0,
            ffa_losses || 0,
            grenade_kills || 0,
            resolvedStoryCount,
            safeStr(challenge_progress, '{}'),
            safeStr(completed_challenges, '[]'),
            safeStr(weapon_prestige, '{}'),
            safeStr(weapon_xp, '{}'),
            safeStr(weapon_levels, '{}'),
            safeStr(unlocked_camos, '["Default"]'),
            safeStr(unlocked_titles, '["Survivor"]'),
            safeStr(unlocked_calling_cards, '["Default"]'),
            safeStr(unlocked_skins, '["Player Starter"]'),
            safeStr(unlocked_capes, '["Default", "Golden Cape"]'),
            safeStr(weapon_attachments, '{}'),
            safeStr(weapon_camos, '{}'),
            safeStr(story_checkpoints, '{}'),
            safeStr(completed_story_missions, '[]'),
            pvp_primary || null,
            pvp_secondary || null,
            userId
        ],
        (err) => {
            if (err) {
                console.error("Stats update error:", err.message);
                return res.status(500).json({ error: err.message });
            }
            res.json({ message: "Stats saved successfully!" });
        }
    );
});

// ── Story Mode Dedicated Endpoints ───────────────────────────────────────────

// Record/update a story checkpoint reached
app.post('/api/story/checkpoint', (req, res) => {
    const { userId, missionId, checkpointId } = req.body;
    if (!userId || !missionId || !checkpointId) {
        return res.status(400).json({ error: "userId, missionId, and checkpointId are required" });
    }
    if (!validateSession(req, res)) return;

    db.get(`SELECT story_checkpoints FROM stats WHERE user_id = ?`, [userId], (err, row) => {
        if (err) return res.status(500).json({ error: err.message });
        let cps = safeParse(row ? row.story_checkpoints : null, {});
        cps[missionId] = checkpointId;

        db.run(`UPDATE stats SET story_checkpoints = ? WHERE user_id = ?`, [JSON.stringify(cps), userId], (upErr) => {
            if (upErr) return res.status(500).json({ error: upErr.message });
            res.json({ message: `Checkpoint '${checkpointId}' saved for mission '${missionId}'`, checkpoints: cps });
        });
    });
});

// Record a completed story mission
app.post('/api/story/complete', (req, res) => {
    const { userId, missionId } = req.body;
    if (!userId || !missionId) {
        return res.status(400).json({ error: "userId and missionId are required" });
    }
    if (!validateSession(req, res)) return;

    db.get(`SELECT completed_story_missions, story_missions_completed FROM stats WHERE user_id = ?`, [userId], (err, row) => {
        if (err) return res.status(500).json({ error: err.message });
        let completed = safeParse(row ? row.completed_story_missions : null, []);
        if (!Array.isArray(completed)) completed = [];
        if (!completed.includes(missionId)) {
            completed.push(missionId);
        }
        const count = completed.length;

        db.run(`UPDATE stats SET completed_story_missions = ?, story_missions_completed = MAX(story_missions_completed, ?) WHERE user_id = ?`,
            [JSON.stringify(completed), count, userId], (upErr) => {
                if (upErr) return res.status(500).json({ error: upErr.message });
                res.json({ message: `Story mission '${missionId}' marked as completed!`, completed_missions: completed, count });
            }
        );
    });
});

// Fetch story mode progression directly
app.get('/api/story/progress/:userId', (req, res) => {
    const userId = req.params.userId;
    if (!userId) return res.status(400).json({ error: "userId is required" });

    db.get(`SELECT story_checkpoints, completed_story_missions, story_missions_completed FROM stats WHERE user_id = ?`, [userId], (err, row) => {
        if (err) return res.status(500).json({ error: err.message });
        if (!row) return res.status(404).json({ error: "Stats not found" });

        res.json({
            story_checkpoints: safeParse(row.story_checkpoints, {}),
            completed_story_missions: safeParse(row.completed_story_missions, []),
            story_missions_completed: Number(row.story_missions_completed || 0)
        });
    });
});

// ── Unlocks Endpoints ────────────────────────────────────────────────────────

app.post('/api/unlock', (req, res) => {
    const { userId, itemId } = req.body;
    if (!userId || !itemId) return res.status(400).json({ error: "User ID and Item ID required" });
    if (!validateSession(req, res)) return;

    db.run(`INSERT OR IGNORE INTO unlocked_items (user_id, item_id) VALUES (?, ?)`, [userId, itemId], (err) => {
        if (err) {
            console.error("Unlock insert error:", err.message);
            return res.status(500).json({ error: err.message });
        }
        res.json({ message: `Item ${itemId} unlocked successfully!` });
    });
});

// ── Cosmetics Selection Endpoints ────────────────────────────────────────────

app.post('/api/cosmetics', (req, res) => {
    let { userId, selected_skin, selected_title, selected_calling_card, selected_cape, selected_camo } = req.body;
    if (!userId) return res.status(400).json({ error: "User ID required" });
    if (!validateSession(req, res)) return;

    db.get(`SELECT is_developer FROM users WHERE id = ?`, [userId], (uErr, user) => {
        db.get(`SELECT * FROM stats WHERE user_id = ?`, [userId], (sErr, stats) => {
            if (user && user.is_developer !== 1 && stats) {
                const skins = safeParse(stats.unlocked_skins, []);
                const titles = safeParse(stats.unlocked_titles, []);
                const cards = safeParse(stats.unlocked_calling_cards, []);
                const capes = safeParse(stats.unlocked_capes, []);
                const camos = safeParse(stats.unlocked_camos, []);

                if (selected_skin === "Developer" && !skins.includes("Developer")) selected_skin = "Player Starter";
                if (selected_title === "Developer" && !titles.includes("Developer")) selected_title = "Survivor";
                if (selected_calling_card === "Developer" && !cards.includes("Developer")) selected_calling_card = "Default";
                if (selected_cape === "Developer Cape" && !capes.includes("Developer Cape")) selected_cape = "Golden Cape";
                if (selected_camo === "Developer Matrix" && !camos.includes("Developer Matrix")) selected_camo = "Default";
            }

            db.run(`UPDATE cosmetics SET 
                selected_skin = COALESCE(?, selected_skin), 
                selected_title = COALESCE(?, selected_title), 
                selected_calling_card = COALESCE(?, selected_calling_card),
                selected_cape = COALESCE(?, selected_cape),
                selected_camo = COALESCE(?, selected_camo)
                WHERE user_id = ?`,
                [selected_skin, selected_title, selected_calling_card, selected_cape, selected_camo, userId],
                (err) => {
                    if (err) {
                        console.error("Cosmetics update error:", err.message);
                        return res.status(500).json({ error: err.message });
                    }
                    res.json({ message: "Cosmetics saved successfully!" });
                }
            );
        });
    });
});

// ── Transient Lobbies API ────────────────────────────────────────────────────
let activeLobbies = {}; // userId -> { userId, username, ip, port, map, players, playersList, inProgress, lastSeen }

// Clean up expired lobbies (no heartbeat for 25 seconds)
setInterval(() => {
    const now = Date.now();
    for (const userId in activeLobbies) {
        if (now - activeLobbies[userId].lastSeen > 25000) {
            delete activeLobbies[userId];
        }
    }
}, 5000);

app.post('/api/lobby/host', (req, res) => {
    const { userId, username, ip, port, map } = req.body;
    if (!userId || !username || !ip || !port) {
        return res.status(400).json({ error: "Missing required fields" });
    }
    activeLobbies[userId] = {
        userId,
        username,
        ip,
        port,
        map: map || "Unknown Map",
        players: 1,
        maxPlayers: 4,
        playersList: [username],
        inProgress: false,
        lastSeen: Date.now()
    };
    res.json({ message: "Lobby registered successfully", lobby: activeLobbies[userId] });
});

app.post('/api/lobby/heartbeat', (req, res) => {
    const { userId, players, inProgress, map, playersList } = req.body;
    if (!userId) {
        return res.status(400).json({ error: "User ID required" });
    }
    if (activeLobbies[userId]) {
        activeLobbies[userId].players = players !== undefined ? Math.min(players, 4) : activeLobbies[userId].players;
        activeLobbies[userId].inProgress = inProgress !== undefined ? inProgress : activeLobbies[userId].inProgress;
        activeLobbies[userId].map = map || activeLobbies[userId].map;
        if (Array.isArray(playersList)) {
            activeLobbies[userId].playersList = playersList.slice(0, 4);
        }
        activeLobbies[userId].lastSeen = Date.now();
        res.json({ message: "Heartbeat accepted", lobby: activeLobbies[userId] });
    } else {
        res.status(404).json({ error: "Lobby not found" });
    }
});

app.get('/api/lobby/list', (req, res) => {
    // In pure Zombies Co-Op, return lobbies that are not in progress and have space (< 4)
    const lobbies = Object.values(activeLobbies).filter(l => {
        return !l.inProgress && (l.players < 4);
    });
    res.json(lobbies);
});

// ── Leaderboard API ──────────────────────────────────────────────────────────

app.get('/api/leaderboard', (req, res) => {
    const sortBy = String(req.query.sort || 'rank').toLowerCase();
    const limit = Math.min(Math.max(parseInt(req.query.limit, 10) || 100, 1), 250);
    const offset = Math.max(parseInt(req.query.offset, 10) || 0, 0);

    let orderByClause = 'ORDER BY s.prestige DESC, s.level DESC, s.xp DESC, s.kills DESC';
    if (sortBy === 'kills' || sortBy === 'zombies') {
        orderByClause = 'ORDER BY s.kills DESC, s.headshots DESC, s.highest_round DESC';
    } else if (sortBy === 'rounds') {
        orderByClause = 'ORDER BY s.highest_round DESC, s.highest_points DESC, s.kills DESC';
    } else if (sortBy === 'boss') {
        orderByClause = 'ORDER BY s.boss_kills DESC, s.elemental_kills DESC, s.kills DESC';
    }

    const query = `
        SELECT 
            u.username,
            u.is_developer,
            s.kills, s.headshots, s.rounds_survived, s.downs, s.revives, s.revived_count, s.downed_count, 
            s.highest_round, s.highest_points, s.level, s.prestige, s.xp, 
            s.boss_kills, s.elemental_kills, s.grenade_kills, s.flags_captured, s.players_killed,
            s.domination_wins, s.domination_losses, s.tdm_wins, s.tdm_losses, s.ffa_wins, s.ffa_losses,
            s.story_missions_completed, s.completed_story_missions,
            s.pvp_primary, s.pvp_secondary,
            c.selected_skin, c.selected_title, c.selected_calling_card, c.selected_cape, c.selected_camo
        FROM users u
        LEFT JOIN stats s ON u.id = s.user_id
        LEFT JOIN cosmetics c ON u.id = c.user_id
        ${orderByClause}
        LIMIT ? OFFSET ?
    `;

    db.all(query, [limit, offset], (err, rows) => {
        if (err) {
            console.error("Leaderboard query error:", err.message);
            return res.status(500).json({ error: err.message });
        }
        const cleanedRows = (rows || []).map(r => ({
            ...r,
            is_developer: Boolean(r.is_developer === 1),
            completed_story_missions: safeParse(r.completed_story_missions, []),
            story_missions_completed: Number(r.story_missions_completed || 0)
        }));
        res.json(cleanedRows);
    });
});

// ── Social & Invites Endpoints ───────────────────────────────────────────────

const activeInvites = {}; // username -> array of invite objects

// Search users
app.post('/api/friends/search', (req, res) => {
    const { prefix } = req.body;
    if (!prefix) return res.json([]);
    const query = `SELECT id, username FROM users WHERE username LIKE ? LIMIT 10`;
    db.all(query, [`${prefix}%`], (err, rows) => {
        if (err) return res.status(500).json({ error: err.message });
        res.json(rows);
    });
});

// Send Friend Request
app.post('/api/friends/add', (req, res) => {
    const { userId, targetUsername } = req.body;
    db.get(`SELECT id FROM users WHERE username = ?`, [targetUsername], (err, target) => {
        if (err || !target) return res.status(404).json({ error: "User not found" });
        if (target.id === userId) return res.status(400).json({ error: "Cannot add yourself" });
        
        db.run(`INSERT OR IGNORE INTO friends (user_id, friend_id, status) VALUES (?, ?, 'pending')`, [userId, target.id], (err) => {
            if (err) return res.status(500).json({ error: err.message });
            res.json({ message: "Request sent" });
        });
    });
});

// Accept Friend Request
app.post('/api/friends/accept', (req, res) => {
    const { userId, senderUsername } = req.body;
    db.get(`SELECT id FROM users WHERE username = ?`, [senderUsername], (err, sender) => {
        if (err || !sender) return res.status(404).json({ error: "User not found" });
        
        db.serialize(() => {
            db.run(`UPDATE friends SET status = 'accepted' WHERE user_id = ? AND friend_id = ?`, [sender.id, userId]);
            db.run(`INSERT OR REPLACE INTO friends (user_id, friend_id, status) VALUES (?, ?, 'accepted')`, [userId, sender.id]);
            res.json({ message: "Request accepted" });
        });
    });
});

// Remove Friend / Decline
app.post('/api/friends/remove', (req, res) => {
    const { userId, targetUsername } = req.body;
    db.get(`SELECT id FROM users WHERE username = ?`, [targetUsername], (err, target) => {
        if (err || !target) return res.status(404).json({ error: "User not found" });
        db.run(`DELETE FROM friends WHERE (user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)`, [userId, target.id, target.id, userId], (err) => {
            if (err) return res.status(500).json({ error: err.message });
            res.json({ message: "Friend removed" });
        });
    });
});

// List Friends
app.post('/api/friends/list', (req, res) => {
    const { userId } = req.body;
    const query = `
        SELECT u.username, f.status, 'incoming' as dir
        FROM friends f JOIN users u ON f.user_id = u.id 
        WHERE f.friend_id = ? AND f.status = 'pending'
        UNION ALL
        SELECT u.username, f.status, 'outgoing' as dir
        FROM friends f JOIN users u ON f.friend_id = u.id 
        WHERE f.user_id = ? AND f.status = 'pending'
        UNION ALL
        SELECT u.username, f.status, 'accepted' as dir
        FROM friends f JOIN users u ON f.friend_id = u.id 
        WHERE f.user_id = ? AND f.status = 'accepted'
    `;
    db.all(query, [userId, userId, userId], (err, rows) => {
        if (err) return res.status(500).json({ error: err.message });
        res.json(rows);
    });
});

// Send Invite
app.post('/api/invites/send', (req, res) => {
    const { targetUsername, senderUsername, hostIp, hostPort } = req.body;
    if (!activeInvites[targetUsername]) activeInvites[targetUsername] = [];
    
    if (activeInvites[targetUsername].some(i => i.senderUsername === senderUsername)) {
        return res.json({ message: "Invite already sent" });
    }
    
    activeInvites[targetUsername].push({ senderUsername, hostIp, hostPort, timestamp: Date.now() });
    res.json({ message: "Invite sent" });
});

// Poll Invites
app.post('/api/invites/list', (req, res) => {
    const { username } = req.body;
    let invites = activeInvites[username] || [];
    
    const now = Date.now();
    invites = invites.filter(i => now - i.timestamp < 120000);
    activeInvites[username] = invites;
    
    res.json(invites);
});

// Clear Invite
app.post('/api/invites/clear', (req, res) => {
    const { username, senderUsername } = req.body;
    if (activeInvites[username]) {
        activeInvites[username] = activeInvites[username].filter(i => i.senderUsername !== senderUsername);
    }
    res.json({ message: "Invite cleared" });
});

// ── Global Error Handler ─────────────────────────────────────────────────────

// Catches any unhandled errors thrown inside route handlers
// eslint-disable-next-line no-unused-vars
app.use((err, req, res, next) => {
    console.error("Unhandled server error:", err.stack || err.message);
    const status = Number.isInteger(err.status) && err.status >= 400 && err.status < 500 ? err.status : 500;
    const message = status === 413 ? "Request body too large" : status === 400 ? "Invalid JSON body" : "Internal server error";
    res.status(status).json({ error: message });
});

// ── Start Server ─────────────────────────────────────────────────────────────

const PORT = process.env.PORT || 3000;
const server = app.listen(PORT, () => {
    console.log(`[Server] Outbreak-Z dedicated backend running on port ${PORT}`);
});

// ── WebSocket Netcode Packet Relay ──────────────────────────────────────────
const wss = new WebSocketServer({ noServer: true });
const relayRooms = new Map();

function broadcastLobbyEvent(sessionId, eventObj) {
    const room = relayRooms.get(sessionId);
    if (!room) return;
    const msgStr = typeof eventObj === 'string' ? eventObj : JSON.stringify(eventObj);
    if (room.hostWs && room.hostWs.readyState === 1) { // 1 = OPEN
        try { room.hostWs.send(msgStr); } catch (e) {}
    }
    for (const gws of room.guests.values()) {
        if (gws && gws.readyState === 1) {
            try { gws.send(msgStr); } catch (e) {}
        }
    }
} // sessionId -> { hostWs: WebSocket|null, guests: Map<string, WebSocket> }

server.on('upgrade', (request, socket, head) => {
    let reqUrl;
    try {
        reqUrl = new URL(request.url, 'http://' + (request.headers.host || 'localhost'));
    } catch (e) {
        socket.destroy();
        return;
    }

    if (reqUrl.pathname === '/ws/relay') {
        wss.handleUpgrade(request, socket, head, (ws) => {
            wss.emit('connection', ws, request, reqUrl);
        });
    } else {
        socket.destroy();
    }
});

wss.on('connection', (ws, req, reqUrl) => {
    const sessionId = reqUrl.searchParams.get('session') || '';
    const role = (reqUrl.searchParams.get('role') || 'guest').toLowerCase();
    const peerId = reqUrl.searchParams.get('peerId') || '';

    if (!sessionId) {
        ws.close(1008, 'Session ID required');
        return;
    }

    let room = relayRooms.get(sessionId);
    if (!room) {
        room = { hostWs: null, guests: new Map() };
        relayRooms.set(sessionId, room);
    }

    if (role === 'host') {
        room.hostWs = ws;
        console.log(`[Relay] Host connected for session ${sessionId}`);
    } else {
        room.guests.set(peerId, ws);
        console.log(`[Relay] Guest 0x${peerId} connected for session ${sessionId}`);
    }

    ws.on('message', (data, isBinary) => {
        // Check if message is a JSON control/lobby frame
        let isJson = false;
        let jsonMsg = null;
        if (!isBinary) {
            try {
                const str = (typeof data === 'string') ? data.trim() : data.toString('utf8').trim();
                if (str.startsWith('{') && str.endsWith('}')) {
                    jsonMsg = JSON.parse(str);
                    isJson = true;
                }
            } catch (e) {}
        }

        if (isJson && jsonMsg) {
            if (jsonMsg.type === 'lobby_launch') {
                const session = masterSessions.get(sessionId);
                if (session) {
                    session.inProgress = true;
                    session.launchData = jsonMsg.launch || jsonMsg;
                }
                broadcastLobbyEvent(sessionId, {
                    type: 'lobby_launch',
                    launch: session ? session.launchData : jsonMsg
                });
            } else if (jsonMsg.type === 'lobby_join') {
                const session = masterSessions.get(sessionId);
                if (session) {
                    if (!session.roster) session.roster = [];
                    const p = {
                        peerId: jsonMsg.peerId || peerId || (session.roster.length + 1),
                        machineId: jsonMsg.machineId || peerId,
                        username: jsonMsg.username || 'Spartan',
                        isHost: false,
                        isDev: jsonMsg.isDev === true,
                        serviceTag: jsonMsg.serviceTag || '117',
                        ping: jsonMsg.ping || 25,
                        joinedAt: Date.now()
                    };
                    const idx = session.roster.findIndex(x => x.peerId == p.peerId || x.username.toLowerCase() === p.username.toLowerCase());
                    if (idx >= 0) session.roster[idx] = p;
                    else session.roster.push(p);
                    session.playerCount = session.roster.length;
                    broadcastLobbyEvent(sessionId, {
                        type: 'lobby_roster',
                        roster: session.roster
                    });
                }
            } else if (jsonMsg.type === 'lobby_leave') {
                const session = masterSessions.get(sessionId);
                if (session && session.roster) {
                    session.roster = session.roster.filter(x => x.peerId != jsonMsg.peerId && x.peerId != peerId);
                    session.playerCount = session.roster.length;
                    broadcastLobbyEvent(sessionId, {
                        type: 'lobby_roster',
                        roster: session.roster
                    });
                }
            } else if (jsonMsg.type === 'ping') {
                try { ws.send(JSON.stringify({ type: 'pong' })); } catch(e) {}
            }
            return;
        }

        if (typeof data === 'string') {
            try { data = Buffer.from(data); } catch(e) { return; }
        }
        if (!Buffer.isBuffer(data) || data.length < 8) return;

        if (role === 'guest') {
            // Forward Guest packet directly to Host
            if (room.hostWs && room.hostWs.readyState === ws.OPEN) {
                room.hostWs.send(data, { binary: true });
            }
        } else if (role === 'host') {
            // Host sends packet to Guest(s)
            for (const [gid, gws] of room.guests.entries()) {
                if (gws.readyState === ws.OPEN) {
                    gws.send(data, { binary: true });
                }
            }
        }
    });

    ws.on('close', () => {
        if (role === 'host') {
            if (room.hostWs === ws) room.hostWs = null;
            console.log(`[Relay] Host disconnected for session ${sessionId}`);
        } else {
            room.guests.delete(peerId);
            console.log(`[Relay] Guest 0x${peerId} disconnected for session ${sessionId}`);
        }
        if (!room.hostWs && room.guests.size === 0) {
            relayRooms.delete(sessionId);
        }
    });

    ws.on('error', (err) => {
        console.error(`[Relay] Error on session ${sessionId}:`, err.message);
    });
});

// ── Graceful Shutdown ────────────────────────────────────────────────────────

function shutdown(signal) {
    console.log(`[Server] Received ${signal}. Gracefully shutting down...`);
    server.close(() => {
        db.run("PRAGMA wal_checkpoint(TRUNCATE);", () => {
            db.close((err) => {
                if (err) console.error("Error closing database:", err.message);
                process.exit(0);
            });
        });
    });
}
process.on('SIGINT',  () => shutdown('SIGINT'));
process.on('SIGTERM', () => shutdown('SIGTERM'));
