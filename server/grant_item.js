const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const fs = require('fs');

const targetUsername = process.argv[2];
let category = process.argv[3] ? process.argv[3].toLowerCase() : null;
const itemName = process.argv[4];

if (!targetUsername || !category) {
    console.log('\nUsage: node grant_item.js <username> <category> <item_name>');
    console.log('Categories: skin, cape, camo, title, card');
    console.log('Examples:');
    console.log('  node grant_item.js Playtester1 skin "Playtester Skin"');
    console.log('  node grant_item.js Playtester1 cape "Playtester Cape"');
    console.log('  node grant_item.js Playtester1 title "Alpha Tester"\n');
    process.exit(1);
}

const repoDbPath = path.join(__dirname, 'database.db');
const renderPersistentDir = '/var/data';
const defaultPersistentPath = fs.existsSync(renderPersistentDir) ? path.join(renderPersistentDir, 'database.db') : null;
const dbPath = process.env.DB_PATH || defaultPersistentPath || repoDbPath;

console.log(`[Database] Connecting to SQLite database at: ${dbPath}`);

const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error("Database connection error:", err.message);
        process.exit(1);
    }
});

// Map user-friendly category names to stats table columns
const categoryMap = {
    'skin': 'unlocked_skins',
    'player_skin': 'unlocked_skins',
    'cape': 'unlocked_capes',
    'player_cape': 'unlocked_capes',
    'camo': 'unlocked_camos',
    'weapon_camo': 'unlocked_camos',
    'title': 'unlocked_titles',
    'player_title': 'unlocked_titles',
    'card': 'unlocked_calling_cards',
    'calling_card': 'unlocked_calling_cards'
};

const targetColumn = categoryMap[category];
if (!targetColumn) {
    console.error(`\n❌ ERROR: Invalid category '${category}'. Valid categories are: skin, cape, camo, title, card`);
    db.close();
    process.exit(1);
}

if (!itemName) {
    console.error(`\n❌ ERROR: Please specify an item name to grant to '${targetUsername}'.`);
    db.close();
    process.exit(1);
}

db.serialize(() => {
    db.get(`SELECT id, username FROM users WHERE username = ?`, [targetUsername], (err, user) => {
        if (err) {
            console.error("Error querying user:", err.message);
            db.close();
            process.exit(1);
        }
        if (!user) {
            console.error(`\n❌ ERROR: User '${targetUsername}' not found in database!`);
            console.log("Please register the account first in game or through the server.\n");
            db.close();
            process.exit(1);
        }

        db.get(`SELECT * FROM stats WHERE user_id = ?`, [user.id], (sErr, stats) => {
            if (sErr) {
                console.error("Error querying stats:", sErr.message);
                db.close();
                process.exit(1);
            }

            const safeParse = (str, def) => { try { return str ? JSON.parse(str) : def; } catch(e) { return def; } };
            let rawList = stats ? stats[targetColumn] : null;
            let items = safeParse(rawList, []);

            if (!Array.isArray(items)) {
                items = [];
            }

            if (!items.includes(itemName)) {
                items.push(itemName);
            }

            const updatedStr = JSON.stringify(items);

            if (stats) {
                db.run(`UPDATE stats SET ${targetColumn} = ? WHERE user_id = ?`, [updatedStr, user.id], (upErr) => {
                    if (upErr) {
                        console.error("Error updating stats:", upErr.message);
                    } else {
                        console.log(`\n✅ SUCCESS: Granted '${itemName}' (${category}) to account '${targetUsername}'!`);
                    }
                    // Insert into unlocked_items table as well
                    db.run(`INSERT OR IGNORE INTO unlocked_items (user_id, item_id) VALUES (?, ?)`, [user.id, itemName], () => {
                        db.close();
                    });
                });
            } else {
                console.error(`\n❌ ERROR: No stats record found for user '${targetUsername}'. Log in with the account once first.`);
                db.close();
            }
        });
    });
});
