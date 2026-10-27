/**
 * Project Noble Auth Routes
 * Handles user registration, login, profile, and armor customization sync.
 */

const crypto = require('node:crypto');
const db = require('../database');
const { hashPassword, verifyPassword, generateToken, calculateExpiry } = require('../auth');

function handleAuthRoutes(req, res, pathname, data, user) {
    // POST /api/auth/register
    if (req.method === 'POST' && pathname === '/api/auth/register') {
        const username = typeof data.username === 'string' ? data.username.trim() : '';
        const password = typeof data.password === 'string' ? data.password : '';
        const serviceTag = typeof data.service_tag === 'string' ? data.service_tag.trim().toUpperCase().slice(0, 4) : 'B312';
        const email = typeof data.email === 'string' ? data.email.trim() : null;

        if (!username || username.length < 3 || username.length > 20) {
            return res.json(400, { error: 'Username must be between 3 and 20 characters' });
        }
        if (!/^[a-zA-Z0-9_-]+$/.test(username)) {
            return res.json(400, { error: 'Username can only contain alphanumeric characters, underscores, and dashes' });
        }
        if (!password || password.length < 6) {
            return res.json(400, { error: 'Password must be at least 6 characters' });
        }

        const existing = db.getUserByUsername(username);
        if (existing) {
            return res.json(409, { error: 'Username is already taken' });
        }

        const { hash, salt } = hashPassword(password);
        const userId = 'usr_' + crypto.randomBytes(8).toString('hex');

        try {
            const newUser = db.createUser({
                id: userId,
                username,
                passwordHash: hash,
                salt,
                serviceTag: serviceTag || 'B312',
                email,
            });

            const token = generateToken();
            db.createAuthToken(token, userId, calculateExpiry());

            const stats = db.getPlayerStats(userId);

            return res.json(201, {
                success: true,
                token,
                user: {
                    id: newUser.id,
                    username: newUser.username,
                    service_tag: newUser.service_tag,
                    created_at: newUser.created_at,
                },
                stats,
            });
        } catch (err) {
            console.error('[auth] Registration error:', err);
            return res.json(500, { error: 'Internal registration error' });
        }
    }

    // POST /api/auth/login
    if (req.method === 'POST' && pathname === '/api/auth/login') {
        const username = typeof data.username === 'string' ? data.username.trim() : '';
        const password = typeof data.password === 'string' ? data.password : '';

        if (!username || !password) {
            return res.json(400, { error: 'Username and password are required' });
        }

        const existingUser = db.getUserByUsername(username);
        if (!existingUser) {
            return res.json(401, { error: 'Invalid username or password' });
        }

        const valid = verifyPassword(password, existingUser.password_hash, existingUser.salt);
        if (!valid) {
            return res.json(401, { error: 'Invalid username or password' });
        }

        db.updateLastLogin(existingUser.id);
        const token = generateToken();
        db.createAuthToken(token, existingUser.id, calculateExpiry());

        const stats = db.getPlayerStats(existingUser.id);

        let armorConfig = {};
        try {
            armorConfig = JSON.parse(existingUser.armor_config || '{}');
        } catch {}

        return res.json(200, {
            success: true,
            token,
            user: {
                id: existingUser.id,
                username: existingUser.username,
                service_tag: existingUser.service_tag,
                email: existingUser.email,
                last_login: Date.now(),
                armor_config: armorConfig,
            },
            stats,
        });
    }

    // GET /api/auth/me
    if (req.method === 'GET' && pathname === '/api/auth/me') {
        if (!user) {
            return res.json(401, { error: 'Authentication required' });
        }

        const stats = db.getPlayerStats(user.id);
        let armorConfig = {};
        try {
            armorConfig = JSON.parse(user.armor_config || '{}');
        } catch {}

        return res.json(200, {
            user: {
                id: user.id,
                username: user.username,
                service_tag: user.service_tag,
                email: user.email,
                armor_config: armorConfig,
            },
            stats,
        });
    }

    // PUT /api/auth/profile
    if (req.method === 'PUT' && pathname === '/api/auth/profile') {
        if (!user) {
            return res.json(401, { error: 'Authentication required' });
        }

        const updates = {};
        if (data.service_tag !== undefined) {
            updates.serviceTag = String(data.service_tag).trim().toUpperCase().slice(0, 4);
        }
        if (data.email !== undefined) {
            updates.email = String(data.email).trim();
        }

        db.updateProfile(user.id, updates);
        return res.json(200, { success: true, message: 'Profile updated' });
    }

    // PUT /api/auth/armor
    if (req.method === 'PUT' && pathname === '/api/auth/armor') {
        if (!user) {
            return res.json(401, { error: 'Authentication required' });
        }

        const armorString = typeof data === 'object' ? JSON.stringify(data) : '{}';
        db.updateArmorConfig(user.id, armorString);
        return res.json(200, { success: true, message: 'Armor customization saved' });
    }

    // POST /api/auth/logout
    if (req.method === 'POST' && pathname === '/api/auth/logout') {
        const authHeader = req.headers['authorization'] || '';
        const token = authHeader.replace(/^Bearer\s+/i, '').trim();
        if (token) {
            db.deleteAuthToken(token);
        }
        return res.json(200, { success: true });
    }

    return false; // Route not handled here
}

module.exports = {
    handleAuthRoutes,
};
