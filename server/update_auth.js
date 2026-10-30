const fs = require('fs');
const path = require('path');

function updateServerFile(filePath) {
    console.log(`Processing: ${filePath}`);
    let code = fs.readFileSync(filePath, 'utf8');
    const isCRLF = code.includes('\r\n');
    const eol = isCRLF ? '\r\n' : '\n';

    // 1. Table schema
    code = code.replace(
        'username TEXT UNIQUE,' + eol + '            password TEXT,',
        'username TEXT UNIQUE COLLATE NOCASE,' + eol + '            password TEXT,'
    );

    // 2. Register check
    const oldRegPattern = /app\.post\('\/api\/register'[\s\S]*?bcrypt\.hash\(password, 10, \(err, hash\) => \{/;
    const newRegSnippet = `app.post('/api/register', (req, res) => {` + eol +
`    const { username, password } = req.body;` + eol +
`    if (!username || !password) {` + eol +
`        return res.status(400).json({ error: "Username and password required" });` + eol +
`    }` + eol +
eol +
`    db.get(\`SELECT id FROM users WHERE LOWER(username) = LOWER(?)\`, [username], (checkErr, existing) => {` + eol +
`        if (checkErr) {` + eol +
`            console.error("Register check error:", checkErr.message);` + eol +
`            return res.status(500).json({ error: "Database error" });` + eol +
`        }` + eol +
`        if (existing) {` + eol +
`            return res.status(400).json({ error: "Username already taken" });` + eol +
`        }` + eol +
eol +
`        bcrypt.hash(password, 10, (err, hash) => {`;

    if (oldRegPattern.test(code)) {
        code = code.replace(oldRegPattern, newRegSnippet);
        // Add closing parenthesis to register route
        const oldRegEnd = `            res.json({ message: "Registration successful!", userId });` + eol +
`        });` + eol +
`    });` + eol +
`});`;
        const newRegEnd = `            res.json({ message: "Registration successful!", userId });` + eol +
`        });` + eol +
`    });` + eol +
`    });` + eol +
`});`;
        code = code.replace(oldRegEnd, newRegEnd);
        console.log(' - Register route updated with case-insensitive uniqueness check.');
    } else {
        console.warn(' - Old register pattern not matched.');
    }

    // 3. Login user lookup
    const oldLoginPattern = /app\.post\('\/api\/login', \(req, res\) => \{[\s\S]*?db\.get\(`SELECT \* FROM users WHERE username = \?`, \[username\], \(err, user\) => \{/;
    const newLoginSnippet = `app.post('/api/login', (req, res) => {` + eol +
`    const { username, password } = req.body;` + eol +
`    if (!username || !password) {` + eol +
`        return res.status(400).json({ error: "Username and password required" });` + eol +
`    }` + eol +
eol +
`    db.get(\`SELECT * FROM users WHERE LOWER(username) = LOWER(?)\`, [username], (err, user) => {`;

    if (oldLoginPattern.test(code)) {
        code = code.replace(oldLoginPattern, newLoginSnippet);
        console.log(' - Login lookup updated with case-insensitive username match.');
    }

    // 4. Single-instance active session check
    const oldSessionPattern = /\/\/ Smart active session check[\s\S]*?if \(activeSessions\[String\(user\.id\)\]\) \{[\s\S]*?return res\.status\(409\)\.json\(\{[\s\S]*?\}\);[\s\S]*?\}[\s\S]*?\}/;
    const newSessionSnippet = `// Strict single-instance session check:` + eol +
`                        // Prevent logging into the same account on 2 instances simultaneously` + eol +
`                        // or having the same username active concurrently.` + eol +
`                        const activeKey = String(user.id);` + eol +
`                        const existingSessionEntry = Object.entries(activeSessions).find(([uid, sess]) => {` + eol +
`                            if (uid === activeKey) return true;` + eol +
`                            if (sess.username && sess.username.toLowerCase() === user.username.toLowerCase()) return true;` + eol +
`                            return false;` + eol +
`                        });` + eol +
eol +
`                        if (existingSessionEntry) {` + eol +
`                            const [existingUid, existing] = existingSessionEntry;` + eol +
`                            const timeSinceLastSeen = Date.now() - existing.lastSeen;` + eol +
`                            // Expire dead sessions after 120 seconds of inactivity` + eol +
`                            if (timeSinceLastSeen > 120 * 1000) {` + eol +
`                                delete activeSessions[existingUid];` + eol +
`                            } else {` + eol +
`                                return res.status(409).json({` + eol +
`                                    error: "Error Account Already Logged In Logout on the other device to proceed"` + eol +
`                                });` + eol +
`                            }` + eol +
`                        }`;

    if (oldSessionPattern.test(code)) {
        code = code.replace(oldSessionPattern, newSessionSnippet);
        console.log(' - Session check updated with strict multi-instance rejection.');
    } else {
        console.warn(' - Old session pattern not matched.');
    }

    // 5. Logout route
    const oldLogout = `app.post('/api/logout', (req, res) => {` + eol +
`    const userId = String(req.body.userId || '');` + eol +
`    if (userId) invalidateSession(userId);` + eol +
`    res.json({ message: "Logged out successfully" });` + eol +
`});`;
    const newLogout = `app.post('/api/logout', (req, res) => {` + eol +
`    const userId = String(req.body.userId || '');` + eol +
`    const username = String(req.body.username || '');` + eol +
`    if (userId) invalidateSession(userId);` + eol +
`    if (username) {` + eol +
`        for (const [uid, sess] of Object.entries(activeSessions)) {` + eol +
`            if (sess.username && sess.username.toLowerCase() === username.toLowerCase()) {` + eol +
`                delete activeSessions[uid];` + eol +
`            }` + eol +
`        }` + eol +
`    }` + eol +
`    res.json({ message: "Logged out successfully" });` + eol +
`});`;

    if (code.includes(oldLogout)) {
        code = code.replace(oldLogout, newLogout);
        console.log(' - Logout route updated.');
    }

    // 6. WebSocket Lobby join
    const oldLobbyJoin = `const idx = session.roster.findIndex(x => x.peerId == p.peerId || x.username.toLowerCase() === p.username.toLowerCase());` + eol +
`                    if (idx >= 0) session.roster[idx] = p;` + eol +
`                    else session.roster.push(p);`;

    const newLobbyJoin = `const nameConflict = session.roster.some(x =>` + eol +
`                        x.peerId != p.peerId &&` + eol +
`                        x.username &&` + eol +
`                        x.username.toLowerCase() === p.username.toLowerCase()` + eol +
`                    );` + eol +
`                    if (nameConflict) {` + eol +
`                        try {` + eol +
`                            ws.send(JSON.stringify({` + eol +
`                                type: 'lobby_error',` + eol +
`                                error: 'A player with this name is already in the lobby.'` + eol +
`                            }));` + eol +
`                        } catch(e) {}` + eol +
`                        return;` + eol +
`                    }` + eol +
`                    const idx = session.roster.findIndex(x => x.peerId == p.peerId);` + eol +
`                    if (idx >= 0) session.roster[idx] = p;` + eol +
`                    else session.roster.push(p);`;

    if (code.includes(oldLobbyJoin)) {
        code = code.replace(oldLobbyJoin, newLobbyJoin);
        console.log(' - WebSocket lobby join updated with duplicate name rejection.');
    }

    fs.writeFileSync(filePath, code, 'utf8');
    console.log(`Saved: ${filePath}`);
}

updateServerFile(path.join(__dirname, 'server.js'));
const deployPath = path.join(__dirname, '..', '..', 'ServerTestDeploy', 'server.js');
if (fs.existsSync(deployPath)) {
    updateServerFile(deployPath);
}

