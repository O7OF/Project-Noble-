const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const fs = require('fs');

const repoDbPath = path.join(__dirname, 'database.db');
const renderPersistentDir = '/var/data';
const defaultPersistentPath = fs.existsSync(renderPersistentDir) ? path.join(renderPersistentDir, 'database.db') : null;
const dbPath = process.env.DB_PATH || defaultPersistentPath || repoDbPath;

console.log('======================================================');
console.log('       Noble / Outbreak-Z Database Reset Tool         ');
console.log('======================================================');
console.log(`Target database: ${dbPath}`);

// If file exists, back it up and remove old database file
if (fs.existsSync(dbPath)) {
    const backupPath = `${dbPath}.bak.${Date.now()}`;
    try {
        fs.copyFileSync(dbPath, backupPath);
        console.log(`[Backup] Existing database backed up to: ${backupPath}`);
        fs.unlinkSync(dbPath);
        console.log(`[Clean] Old database file removed.`);
    } catch (err) {
        console.error(`[Error] Failed to rotate existing database: ${err.message}`);
    }
}

const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error("Database connection error:", err.message);
        process.exit(1);
    }
});

db.serialize(() => {
    console.log('[Schema] Creating fresh tables...');

    // Users table
    db.run(`CREATE TABLE users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE,
        password TEXT,
        is_developer INTEGER DEFAULT 0
    )`);

    // Stats table
    db.run(`CREATE TABLE stats (
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
    )`);

    // Unlocked items table
    db.run(`CREATE TABLE unlocked_items (
        user_id INTEGER,
        item_id TEXT,
        PRIMARY KEY(user_id, item_id),
        FOREIGN KEY(user_id) REFERENCES users(id)
    )`);

    // Selected cosmetics table
    db.run(`CREATE TABLE cosmetics (
        user_id INTEGER PRIMARY KEY,
        selected_skin TEXT DEFAULT 'Default',
        selected_title TEXT DEFAULT 'Survivor',
        selected_calling_card TEXT DEFAULT 'Default',
        selected_cape TEXT DEFAULT 'Default',
        selected_camo TEXT DEFAULT 'Default',
        FOREIGN KEY(user_id) REFERENCES users(id)
    )`);

    // Friends table
    db.run(`CREATE TABLE friends (
        user_id INTEGER,
        friend_id INTEGER,
        status TEXT DEFAULT 'pending',
        PRIMARY KEY(user_id, friend_id),
        FOREIGN KEY(user_id) REFERENCES users(id),
        FOREIGN KEY(friend_id) REFERENCES users(id)
    )`, (err) => {
        if (err) {
            console.error('[Error] Failed creating schema:', err.message);
        } else {
            console.log('✅ Database schema initialized successfully!');
            console.log('Tip: Use "node set_dev.js <username>" to promote an account to developer.');
        }
        db.close();
    });
});

