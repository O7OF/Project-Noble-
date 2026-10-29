const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const fs = require('fs');

const firstArg = process.argv[2];
const secondArg = process.argv[3];

if (!firstArg || firstArg === '--help' || firstArg === '-h') {
    console.log('\n======================================================');
    console.log('       Noble / Outbreak-Z Developer Account Tool       ');
    console.log('======================================================');
    console.log('Usage:');
    console.log('  node set_dev.js list');
    console.log('  node set_dev.js <username>');
    console.log('  node set_dev.js <username> remove\n');
    console.log('Examples:');
    console.log('  node set_dev.js O7OF');
    console.log('  node set_dev.js Tyler');
    console.log('  node set_dev.js list\n');
    process.exit(0);
}

// Check for Render Persistent Disk (/var/data/database.db), DB_PATH env var, or fallback to repo database.db
const repoDbPath = path.join(__dirname, 'database.db');
const renderPersistentDir = '/var/data';
const defaultPersistentPath = fs.existsSync(renderPersistentDir) ? path.join(renderPersistentDir, 'database.db') : null;
const dbPath = process.env.DB_PATH || defaultPersistentPath || repoDbPath;

// If using external persistent disk and database.db doesn't exist yet, seed it from repo copy
if (dbPath !== repoDbPath && !fs.existsSync(dbPath)) {
    try {
        const dir = path.dirname(dbPath);
        if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
        if (fs.existsSync(repoDbPath)) {
            fs.copyFileSync(repoDbPath, dbPath);
            console.log(`[Database] Seeded persistent database to ${dbPath} from ${repoDbPath}`);
        }
    } catch (e) {
        console.error("[Database] Error seeding persistent database file:", e.message);
    }
}

console.log(`[Database] Connecting to SQLite database at: ${dbPath}`);

const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error("Database connection error:", err.message);
        process.exit(1);
    }
});

db.serialize(() => {
    // Ensure is_developer column exists
    db.run(`ALTER TABLE users ADD COLUMN is_developer INTEGER DEFAULT 0`, () => {});

    // Command: LIST all registered users
    if (firstArg.toLowerCase() === 'list' || firstArg.toLowerCase() === '--list') {
        db.all(`SELECT id, username, is_developer FROM users ORDER BY id ASC`, (err, rows) => {
            if (err) {
                console.error("Error listing users:", err.message);
                db.close();
                process.exit(1);
            }
            console.log('\n======================================================');
            console.log(`Registered Accounts (${rows.length} total):`);
            console.log('======================================================');
            if (rows.length === 0) {
                console.log(' (No registered users found in database)');
            } else {
                for (const u of rows) {
                    const devTag = u.is_developer === 1 ? ' [DEVELOPER]' : '            ';
                    console.log(` ID: ${String(u.id).padEnd(4)} | ${u.username.padEnd(16)} | Status:${devTag}`);
                }
            }
            console.log('======================================================\n');
            db.close();
        });
        return;
    }

    const targetUsername = firstArg;
    const action = secondArg ? secondArg.toLowerCase() : 'add';

    db.get(`SELECT id, username, is_developer FROM users WHERE LOWER(username) = LOWER(?)`, [targetUsername], (err, user) => {
        if (err) {
            console.error("Error querying user:", err.message);
            db.close();
            process.exit(1);
        }
        if (!user) {
            console.error(`\n❌ ERROR: User '${targetUsername}' not found in database!`);
            console.log("Please register the account first in game or through the server login/register screen.\n");
            db.all(`SELECT username FROM users LIMIT 10`, (lErr, list) => {
                if (list && list.length > 0) {
                    console.log("Existing accounts: " + list.map(u => u.username).join(', ') + "\n");
                }
                db.close();
                process.exit(1);
            });
            return;
        }

        const isDev = action !== 'remove' ? 1 : 0;

        db.run(`UPDATE users SET is_developer = ? WHERE id = ?`, [isDev, user.id], (upErr) => {
            if (upErr) {
                console.error("Error updating user:", upErr.message);
                db.close();
                process.exit(1);
            }

            if (isDev === 1) {
                db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
                    const safeParse = (str, def) => { try { return str ? JSON.parse(str) : def; } catch(e) { return def; } };
                    let skins = safeParse(stats && stats.unlocked_skins, ["Player Starter"]);
                    let camos = safeParse(stats && stats.unlocked_camos, ["Default"]);
                    let cards = safeParse(stats && stats.unlocked_calling_cards, ["Default"]);
                    let capes = safeParse(stats && stats.unlocked_capes, ["Default", "Golden Cape"]);
                    let titles = safeParse(stats && stats.unlocked_titles, ["Survivor"]);

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

                    if (stats) {
                        db.run(
                            `UPDATE stats SET unlocked_skins = ?, unlocked_camos = ?, unlocked_calling_cards = ?, unlocked_capes = ?, unlocked_titles = ? WHERE user_id = ?`,
                            [JSON.stringify(skins), JSON.stringify(camos), JSON.stringify(cards), JSON.stringify(capes), JSON.stringify(titles), user.id],
                            (stErr) => {
                                if (stErr) console.error("Error updating stats:", stErr.message);
                                console.log(`\n✅ SUCCESS: Account '${user.username}' (ID: ${user.id}) is now a DEVELOPER account!`);
                                console.log("Developer skins, camos, calling cards, capes, and titles have been unlocked.\n");
                                db.close();
                            }
                        );
                    } else {
                        // Create stats row with developer unlocks
                        db.run(
                            `INSERT INTO stats (user_id, unlocked_skins, unlocked_camos, unlocked_calling_cards, unlocked_capes, unlocked_titles) VALUES (?, ?, ?, ?, ?, ?)`,
                            [user.id, JSON.stringify(skins), JSON.stringify(camos), JSON.stringify(cards), JSON.stringify(capes), JSON.stringify(titles)],
                            (inErr) => {
                                if (inErr) console.error("Error inserting stats:", inErr.message);
                                console.log(`\n✅ SUCCESS: Account '${user.username}' (ID: ${user.id}) is now a DEVELOPER account!`);
                                db.close();
                            }
                        );
                    }
                });
            } else {
                console.log(`\n✅ SUCCESS: Developer status removed from account '${user.username}' (is_developer = 0)!\n`);
                db.close();
            }
        });
    });
});
