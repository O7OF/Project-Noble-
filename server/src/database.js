/**
 * Project Noble Database Layer
 * Uses Node.js native SQLite (node:sqlite DatabaseSync) with ACID safety,
 *WAL mode, and foreign keys.
 */

const fs = require('node:fs');
const path = require('node:path');
const { DB_PATH, DATA_DIR } = require('./config');
const { getRank } = require('./ranks');

let db = null;

function initDatabase() {
    if (db) return db;

    if (!fs.existsSync(DATA_DIR)) {
        fs.mkdirSync(DATA_DIR, { recursive: true });
    }

    try {
        const { DatabaseSync } = require('node:sqlite');
        db = new DatabaseSync(DB_PATH);
        // Enable WAL mode and foreign keys
        db.exec(`
            PRAGMA journal_mode = WAL;
            PRAGMA foreign_keys = ON;
        `);
        console.log(`[database] Initialized native SQLite database at: ${DB_PATH}`);
    } catch (err) {
        console.error('[database] Warning: Failed to load node:sqlite, using memory SQLite fallback:', err.message);
        const { DatabaseSync } = require('node:sqlite');
        db = new DatabaseSync(':memory:');
    }

    createTables();
    return db;
}

function createTables() {
    db.exec(`
        CREATE TABLE IF NOT EXISTS users (
            id TEXT PRIMARY KEY,
            username TEXT UNIQUE NOT NULL COLLATE NOCASE,
            password_hash TEXT NOT NULL,
            salt TEXT NOT NULL,
            service_tag TEXT DEFAULT 'B312',
            email TEXT,
            created_at INTEGER NOT NULL,
            last_login INTEGER,
            armor_config TEXT DEFAULT '{}'
        );

        CREATE TABLE IF NOT EXISTS auth_tokens (
            token TEXT PRIMARY KEY,
            user_id TEXT NOT NULL,
            expires_at INTEGER NOT NULL,
            created_at INTEGER NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS player_stats (
            user_id TEXT PRIMARY KEY,
            credits INTEGER DEFAULT 0,
            rank_index INTEGER DEFAULT 0,
            rank_name TEXT DEFAULT 'Recruit',
            games_played INTEGER DEFAULT 0,
            wins INTEGER DEFAULT 0,
            losses INTEGER DEFAULT 0,
            kills INTEGER DEFAULT 0,
            deaths INTEGER DEFAULT 0,
            assists INTEGER DEFAULT 0,
            medals INTEGER DEFAULT 0,
            headshots INTEGER DEFAULT 0,
            minutes_played REAL DEFAULT 0.0,
            updated_at INTEGER NOT NULL,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS matches (
            id TEXT PRIMARY KEY,
            server_id TEXT,
            server_name TEXT,
            map TEXT,
            mode TEXT,
            playlist TEXT,
            duration_seconds REAL DEFAULT 0.0,
            created_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS match_player_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            match_id TEXT NOT NULL,
            user_id TEXT,
            player_name TEXT NOT NULL,
            service_tag TEXT,
            team INTEGER DEFAULT 0,
            score INTEGER DEFAULT 0,
            kills INTEGER DEFAULT 0,
            deaths INTEGER DEFAULT 0,
            assists INTEGER DEFAULT 0,
            medals INTEGER DEFAULT 0,
            headshots INTEGER DEFAULT 0,
            credits_earned INTEGER DEFAULT 0,
            is_winner INTEGER DEFAULT 0,
            FOREIGN KEY(match_id) REFERENCES matches(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
        CREATE INDEX IF NOT EXISTS idx_tokens_token ON auth_tokens(token);
        CREATE INDEX IF NOT EXISTS idx_stats_credits ON player_stats(credits DESC);
        CREATE INDEX IF NOT EXISTS idx_stats_kills ON player_stats(kills DESC);
        CREATE INDEX IF NOT EXISTS idx_matches_created ON matches(created_at DESC);
        CREATE INDEX IF NOT EXISTS idx_match_players_user ON match_player_records(user_id);
    `);
}

// User operations
function createUser({ id, username, passwordHash, salt, serviceTag = 'B312', email = null }) {
    const now = Date.now();
    const stmt = db.prepare(`
        INSERT INTO users (id, username, password_hash, salt, service_tag, email, created_at, last_login)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    `);
    stmt.run(id, username, passwordHash, salt, serviceTag, email, now, now);

    // Initialise empty stats
    const statsStmt = db.prepare(`
        INSERT INTO player_stats (user_id, updated_at)
        VALUES (?, ?)
    `);
    statsStmt.run(id, now);

    return getUserById(id);
}

function getUserById(id) {
    const stmt = db.prepare(`SELECT * FROM users WHERE id = ?`);
    return stmt.get(id) || null;
}

function getUserByUsername(username) {
    const stmt = db.prepare(`SELECT * FROM users WHERE username = ? COLLATE NOCASE`);
    return stmt.get(username) || null;
}

function updateLastLogin(id) {
    const stmt = db.prepare(`UPDATE users SET last_login = ? WHERE id = ?`);
    stmt.run(Date.now(), id);
}

function updateArmorConfig(userId, armorConfigJson) {
    const stmt = db.prepare(`UPDATE users SET armor_config = ? WHERE id = ?`);
    stmt.run(armorConfigJson, userId);
}

function updateProfile(userId, { serviceTag, email }) {
    let sql = 'UPDATE users SET ';
    const params = [];
    const updates = [];
    if (serviceTag !== undefined) {
        updates.push('service_tag = ?');
        params.push(serviceTag);
    }
    if (email !== undefined) {
        updates.push('email = ?');
        params.push(email);
    }
    if (updates.length === 0) return;
    sql += updates.join(', ') + ' WHERE id = ?';
    params.push(userId);
    db.prepare(sql).run(...params);
}

// Auth tokens
function createAuthToken(token, userId, expiresAt) {
    const stmt = db.prepare(`
        INSERT INTO auth_tokens (token, user_id, expires_at, created_at)
        VALUES (?, ?, ?, ?)
    `);
    stmt.run(token, userId, expiresAt, Date.now());
}

function getUserByToken(token) {
    const now = Date.now();
    const stmt = db.prepare(`
        SELECT u.id, u.username, u.service_tag, u.email, u.created_at, u.last_login, u.armor_config
        FROM auth_tokens t
        JOIN users u ON t.user_id = u.id
        WHERE t.token = ? AND t.expires_at > ?
    `);
    return stmt.get(token, now) || null;
}

function deleteAuthToken(token) {
    const stmt = db.prepare(`DELETE FROM auth_tokens WHERE token = ?`);
    stmt.run(token);
}

function cleanupExpiredTokens() {
    const stmt = db.prepare(`DELETE FROM auth_tokens WHERE expires_at <= ?`);
    stmt.run(Date.now());
}

// Player stats
function getPlayerStats(userId) {
    const stmt = db.prepare(`
        SELECT s.*, u.username, u.service_tag
        FROM player_stats s
        JOIN users u ON s.user_id = u.id
        WHERE s.user_id = ?
    `);
    const stats = stmt.get(userId);
    if (!stats) return null;

    const rankInfo = getRank(stats.credits);
    return {
        ...stats,
        rank_index: rankInfo.index,
        rank_name: rankInfo.name,
        rank_details: rankInfo,
        kd_ratio: stats.deaths > 0 ? Number((stats.kills / stats.deaths).toFixed(2)) : stats.kills,
        win_rate: stats.games_played > 0 ? Number(((stats.wins / stats.games_played) * 100).toFixed(1)) : 0.0,
    };
}

function getPlayerStatsByUsername(username) {
    const user = getUserByUsername(username);
    if (!user) return null;
    return getPlayerStats(user.id);
}

function addPlayerMatchStats(userId, matchDeltas) {
    const current = db.prepare(`SELECT * FROM player_stats WHERE user_id = ?`).get(userId);
    if (!current) return null;

    const newCredits = current.credits + (matchDeltas.creditsEarned || 0);
    const rankInfo = getRank(newCredits);

    const stmt = db.prepare(`
        UPDATE player_stats SET
            credits = ?,
            rank_index = ?,
            rank_name = ?,
            games_played = games_played + 1,
            wins = wins + ?,
            losses = losses + ?,
            kills = kills + ?,
            deaths = deaths + ?,
            assists = assists + ?,
            medals = medals + ?,
            headshots = headshots + ?,
            minutes_played = minutes_played + ?,
            updated_at = ?
        WHERE user_id = ?
    `);

    stmt.run(
        newCredits,
        rankInfo.index,
        rankInfo.name,
        matchDeltas.isWinner ? 1 : 0,
        matchDeltas.isWinner ? 0 : 1,
        matchDeltas.kills || 0,
        matchDeltas.deaths || 0,
        matchDeltas.assists || 0,
        matchDeltas.medals || 0,
        matchDeltas.headshots || 0,
        matchDeltas.minutesPlayed || 0.0,
        Date.now(),
        userId
    );

    return getPlayerStats(userId);
}

function getLeaderboard({ sortBy = 'credits', limit = 50 }) {
    const safeLimit = Math.min(100, Math.max(1, Number(limit) || 50));
    let orderColumn = 's.credits';
    if (sortBy === 'kills') orderColumn = 's.kills';
    else if (sortBy === 'wins') orderColumn = 's.wins';
    else if (sortBy === 'games') orderColumn = 's.games_played';

    const stmt = db.prepare(`
        SELECT s.*, u.username, u.service_tag
        FROM player_stats s
        JOIN users u ON s.user_id = u.id
        ORDER BY ${orderColumn} DESC
        LIMIT ?
    `);

    const rows = stmt.all(safeLimit);
    return rows.map((row, idx) => {
        const rankInfo = getRank(row.credits);
        return {
            rank_position: idx + 1,
            ...row,
            rank_name: rankInfo.name,
            kd_ratio: row.deaths > 0 ? Number((row.kills / row.deaths).toFixed(2)) : row.kills,
            win_rate: row.games_played > 0 ? Number(((row.wins / row.games_played) * 100).toFixed(1)) : 0.0,
        };
    });
}

// Matches & History
function recordMatch({ id, serverId, serverName, map, mode, playlist, durationSeconds, players = [] }) {
    const now = Date.now();
    const matchStmt = db.prepare(`
        INSERT INTO matches (id, server_id, server_name, map, mode, playlist, duration_seconds, created_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    `);
    matchStmt.run(id, serverId || null, serverName || null, map || 'Unknown', mode || 'Multiplayer', playlist || 'Custom', durationSeconds || 0.0, now);

    const playerRecordStmt = db.prepare(`
        INSERT INTO match_player_records (
            match_id, user_id, player_name, service_tag, team, score, kills, deaths, assists, medals, headshots, credits_earned, is_winner
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    `);

    for (const p of players) {
        playerRecordStmt.run(
            id,
            p.user_id || null,
            p.player_name || 'Spartan',
            p.service_tag || 'B312',
            p.team || 0,
            p.score || 0,
            p.kills || 0,
            p.deaths || 0,
            p.assists || 0,
            p.medals || 0,
            p.headshots || 0,
            p.credits_earned || 0,
            p.is_winner ? 1 : 0
        );

        // Update player stats in career table if linked to registered user
        if (p.user_id) {
            addPlayerMatchStats(p.user_id, {
                creditsEarned: p.credits_earned,
                isWinner: p.is_winner,
                kills: p.kills,
                deaths: p.deaths,
                assists: p.assists,
                medals: p.medals,
                headshots: p.headshots,
                minutesPlayed: (durationSeconds || 0) / 60.0,
            });
        }
    }

    return getMatchById(id);
}

function getMatchById(id) {
    const match = db.prepare(`SELECT * FROM matches WHERE id = ?`).get(id);
    if (!match) return null;
    const players = db.prepare(`SELECT * FROM match_player_records WHERE match_id = ?`).all(id);
    return {
        ...match,
        players,
    };
}

function getRecentMatches({ limit = 20, userId = null } = {}) {
    const safeLimit = Math.min(50, Math.max(1, Number(limit) || 20));
    if (userId) {
        const stmt = db.prepare(`
            SELECT DISTINCT m.*
            FROM matches m
            JOIN match_player_records r ON m.id = r.match_id
            WHERE r.user_id = ?
            ORDER BY m.created_at DESC
            LIMIT ?
        `);
        return stmt.all(userId, safeLimit);
    }

    const stmt = db.prepare(`SELECT * FROM matches ORDER BY created_at DESC LIMIT ?`);
    return stmt.all(safeLimit);
}

module.exports = {
    initDatabase,
    createUser,
    getUserById,
    getUserByUsername,
    updateLastLogin,
    updateArmorConfig,
    updateProfile,
    createAuthToken,
    getUserByToken,
    deleteAuthToken,
    cleanupExpiredTokens,
    getPlayerStats,
    getPlayerStatsByUsername,
    addPlayerMatchStats,
    getLeaderboard,
    recordMatch,
    getMatchById,
    getRecentMatches,
};
