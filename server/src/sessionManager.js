/**
 * Project Noble Session Manager (Master Server Registry)
 * Maintains active game sessions, heartbeats, and filters.
 * 100% compatible with existing MasterServerClient.cpp.
 */

const crypto = require('node:crypto');
const {
    SESSION_TIMEOUT_MS,
    MAX_SESSIONS_PER_IP,
    MAX_SESSIONS_TOTAL,
} = require('./config');

class SessionManager {
    constructor() {
        this.sessions = new Map(); // id -> session
        this.cleanupInterval = null;
    }

    start() {
        if (this.cleanupInterval) return;
        this.cleanupInterval = setInterval(() => this.cleanup(), 5000);
    }

    stop() {
        if (this.cleanupInterval) {
            clearInterval(this.cleanupInterval);
            this.cleanupInterval = null;
        }
    }

    cleanup() {
        const now = Date.now();
        for (const [id, s] of this.sessions) {
            if (now - s.lastHeartbeat > SESSION_TIMEOUT_MS) {
                console.log(`[sessions] Session expired: ${id} "${s.name}"`);
                this.sessions.delete(id);
            }
        }
    }

    sessionsForIp(ip) {
        let count = 0;
        for (const s of this.sessions.values()) {
            if (s.address === ip) count++;
        }
        return count;
    }

    registerSession(clientIp, data) {
        if (this.sessions.size >= MAX_SESSIONS_TOTAL) {
            const err = new Error('Server list is full');
            err.statusCode = 503;
            throw err;
        }

        if (this.sessionsForIp(clientIp) >= MAX_SESSIONS_PER_IP) {
            const err = new Error('Too many games registered from this address');
            err.statusCode = 429;
            throw err;
        }

        const id = crypto.randomBytes(8).toString('hex');
        const token = crypto.randomBytes(24).toString('hex');

        const session = {
            id,
            token,
            address: clientIp,
            name: this.cleanText(data.name, 'Reach Game', 48),
            port: this.cleanInt(data.port, 11774, 1, 65535),
            playerCount: this.cleanInt(data.player_count, 1, 0, 64),
            maxPlayers: this.cleanInt(data.max_players, 16, 1, 64),
            map: this.cleanText(data.map, 'Unknown Map', 48),
            mode: this.cleanText(data.mode, 'Multiplayer', 32),
            playlist: this.cleanText(data.playlist, 'Custom', 32),
            inProgress: data.in_progress === true,
            hasPassword: data.has_password === true,
            build: this.cleanText(data.build, '', 32),
            isDedicated: data.dedicated === true,
            lastHeartbeat: Date.now(),
            registeredAt: Date.now(),
        };

        this.sessions.set(id, session);
        console.log(`[sessions] Registered ${id} "${session.name}" from ${clientIp}:${session.port}`);
        return { sessionId: id, token };
    }

    heartbeat(sessionId, token, updates = {}) {
        const session = this.sessions.get(sessionId);
        if (!session) return false;

        if (!this.tokensEqual(token, session.token)) {
            const err = new Error('Invalid session token');
            err.statusCode = 403;
            throw err;
        }

        session.lastHeartbeat = Date.now();
        if (updates.player_count !== undefined) {
            session.playerCount = this.cleanInt(updates.player_count, session.playerCount, 0, 64);
        }
        if (updates.in_progress !== undefined) {
            session.inProgress = updates.in_progress === true;
        }
        if (updates.map !== undefined) {
            session.map = this.cleanText(updates.map, session.map, 48);
        }
        if (updates.mode !== undefined) {
            session.mode = this.cleanText(updates.mode, session.mode, 32);
        }

        return true;
    }

    unregister(sessionId, token) {
        const session = this.sessions.get(sessionId);
        if (!session) return false;

        if (!this.tokensEqual(token, session.token)) {
            const err = new Error('Invalid session token');
            err.statusCode = 403;
            throw err;
        }

        this.sessions.delete(sessionId);
        console.log(`[sessions] Unregistered session ${sessionId}`);
        return true;
    }

    getPublicSession(id) {
        const s = this.sessions.get(id);
        if (!s) return null;
        return this.publicView(s);
    }

    listPublicSessions(filters = {}) {
        const now = Date.now();
        const results = [];

        for (const s of this.sessions.values()) {
            if (now - s.lastHeartbeat > SESSION_TIMEOUT_MS) continue;

            if (filters.mode && s.mode.toLowerCase() !== filters.mode.toLowerCase()) continue;
            if (filters.map && !s.map.toLowerCase().includes(filters.map.toLowerCase())) continue;
            if (filters.hide_full && s.playerCount >= s.maxPlayers) continue;
            if (filters.hide_empty && s.playerCount <= 0) continue;

            results.push(this.publicView(s));
        }

        return results;
    }

    findJoinableServers({ mode = 'Multiplayer', requiredSlots = 1, hasPassword = false } = {}) {
        const now = Date.now();
        const list = [];
        for (const s of this.sessions.values()) {
            if (now - s.lastHeartbeat > SESSION_TIMEOUT_MS) continue;
            if (hasPassword === false && s.hasPassword) continue;
            if (mode && s.mode.toLowerCase() !== mode.toLowerCase()) continue;
            if ((s.maxPlayers - s.playerCount) >= requiredSlots) {
                list.push(s);
            }
        }
        return list;
    }

    publicView(s) {
        return {
            id: s.id,
            name: s.name,
            address: s.address,
            port: s.port,
            player_count: s.playerCount,
            max_players: s.maxPlayers,
            map: s.map,
            mode: s.mode,
            playlist: s.playlist,
            in_progress: s.inProgress,
            has_password: s.hasPassword,
            build: s.build,
            is_dedicated: s.isDedicated,
        };
    }

    cleanText(val, fallback, maxLen) {
        if (typeof val !== 'string') return fallback;
        const clean = val.replace(/[\u0000-\u001f\u007f]/g, '').trim().slice(0, maxLen);
        return clean.length > 0 ? clean : fallback;
    }

    cleanInt(val, fallback, min, max) {
        const n = Number.parseInt(val, 10);
        if (!Number.isFinite(n)) return fallback;
        return Math.min(max, Math.max(min, n));
    }

    tokensEqual(a, b) {
        if (typeof a !== 'string' || typeof b !== 'string' || a.length !== b.length) return false;
        return crypto.timingSafeEqual(Buffer.from(a), Buffer.from(b));
    }

    size() {
        return this.sessions.size;
    }
}

const sessionManagerInstance = new SessionManager();

module.exports = {
    SessionManager,
    sessionManager: sessionManagerInstance,
};
