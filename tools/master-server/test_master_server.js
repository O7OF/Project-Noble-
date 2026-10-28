// Basic checks for master_server.js: register, list, heartbeat, token checks, removal.
// Run: node test_master_server.js   (starts the server on a free port and stops it afterwards)
const { spawn } = require('child_process');
const assert = require('assert');
const path = require('path');

const PORT = 18000 + Math.floor(Math.random() * 1000);
const base = `http://127.0.0.1:${PORT}`;

async function call(method, url, body, token) {
    const headers = { 'Content-Type': 'application/json' };
    if (token) headers['X-Session-Token'] = token;
    const res = await fetch(base + url, { method, headers, body: body === undefined ? undefined : JSON.stringify(body) });
    let json = null;
    try { json = await res.json(); } catch { /* empty */ }
    return { status: res.status, json };
}

(async () => {
    const server = spawn(process.execPath, [path.join(__dirname, 'master_server.js')], { env: { ...process.env, PORT: String(PORT) } });
    await new Promise(r => setTimeout(r, 500));
    try {
        let r = await call('POST', '/api/sessions', { name: 'Test\u0007 game', port: 11774, max_players: 99, map: 'Sword Base', mode: 'Multiplayer', public_ip: '8.8.8.8' });
        assert.strictEqual(r.status, 201);
        const { session_id: id, token } = r.json;
        assert.ok(id && token);

        r = await call('GET', '/api/sessions');
        assert.strictEqual(r.json.length, 1);
        assert.strictEqual(r.json[0].name, 'Test game');            // control character removed
        assert.strictEqual(r.json[0].max_players, 16);              // clamped
        assert.notStrictEqual(r.json[0].address, '8.8.8.8');        // body address ignored
        assert.ok(!('token' in r.json[0]));                         // token never listed

        r = await call('PUT', '/api/sessions/heartbeat', { session_id: id, player_count: 3 }, 'wrong');
        assert.strictEqual(r.status, 403);
        r = await call('PUT', '/api/sessions/heartbeat', { session_id: id, player_count: 3 }, token);
        assert.strictEqual(r.status, 200);
        r = await call('GET', '/api/sessions');
        assert.strictEqual(r.json[0].player_count, 3);

        r = await call('DELETE', `/api/sessions/${id}`, undefined, 'wrong');
        assert.strictEqual(r.status, 403);
        r = await call('DELETE', `/api/sessions/${id}`, undefined, token);
        assert.strictEqual(r.status, 200);
        r = await call('GET', '/api/sessions');
        assert.strictEqual(r.json.length, 0);

        for (let i = 0; i < 4; i++) assert.strictEqual((await call('POST', '/api/sessions', { name: `g${i}` })).status, 201);
        assert.strictEqual((await call('POST', '/api/sessions', { name: 'too many' })).status, 429);  // per-IP limit

        const big = await call('POST', '/api/sessions', { name: 'x'.repeat(10000) });
        assert.ok(big.status === 413 || big.status === 0 || big.status === 429);

        console.log('master server tests passed');
    } finally {
        server.kill();
    }
})().catch(err => {
    console.error(err);
    process.exit(1);
});
