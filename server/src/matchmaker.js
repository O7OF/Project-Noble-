/**
 * Project Noble Matchmaking System
 * Handles playlist queues, party matching, ticket lifecycle, and server allocation.
 */

const crypto = require('node:crypto');
const { MATCHMAKING_TICKET_TIMEOUT_MS } = require('./config');

const PLAYLISTS = {
    slayer: { id: 'slayer', name: 'Team Slayer', minPlayers: 2, maxPlayers: 8, defaultMap: 'Sword Base' },
    swat: { id: 'swat', name: 'Team SWAT', minPlayers: 2, maxPlayers: 8, defaultMap: 'Reflection' },
    snipers: { id: 'snipers', name: 'Team Snipers', minPlayers: 2, maxPlayers: 8, defaultMap: 'Countdown' },
    btb: { id: 'btb', name: 'Big Team Battle', minPlayers: 4, maxPlayers: 16, defaultMap: 'Spire' },
    infection: { id: 'infection', name: 'Living Dead', minPlayers: 2, maxPlayers: 12, defaultMap: 'Zealot' },
    invasion: { id: 'invasion', name: 'Invasion', minPlayers: 2, maxPlayers: 12, defaultMap: 'Boneyard' },
    firefight: { id: 'firefight', name: 'Firefight', minPlayers: 1, maxPlayers: 4, defaultMap: 'Corvette' },
    custom: { id: 'custom', name: 'Custom Games', minPlayers: 1, maxPlayers: 16, defaultMap: 'Forge World' },
};

class Matchmaker {
    constructor() {
        this.tickets = new Map(); // ticketId -> ticket
        this.matches = new Map(); // matchId -> match
        this.interval = null;
        this.sessionManager = null; // injected from server
    }

    setSessionManager(sm) {
        this.sessionManager = sm;
    }

    start() {
        if (this.interval) return;
        this.interval = setInterval(() => this.processQueues(), 1500);
        console.log('[matchmaker] Background matchmaking loop started');
    }

    stop() {
        if (this.interval) {
            clearInterval(this.interval);
            this.interval = null;
        }
    }

    /**
     * Create a new matchmaking ticket.
     */
    queuePlayer({
        userId = null,
        playerName = 'Spartan',
        serviceTag = 'B312',
        playlist = 'slayer',
        region = 'auto',
        clientIp = '127.0.0.1',
    }) {
        const playlistKey = (playlist || 'slayer').toLowerCase();
        const playlistDef = PLAYLISTS[playlistKey] || PLAYLISTS.slayer;

        const ticketId = 'tkt_' + crypto.randomBytes(12).toString('hex');
        const now = Date.now();

        const ticket = {
            id: ticketId,
            userId,
            playerName,
            serviceTag,
            playlist: playlistDef.id,
            playlistName: playlistDef.name,
            region,
            clientIp,
            status: 'QUEUED', // QUEUED, MATCHED, EXPIRED, CANCELED
            queuedAt: now,
            matchId: null,
            matchResult: null,
        };

        this.tickets.set(ticketId, ticket);
        console.log(`[matchmaker] Queued ticket ${ticketId} for ${playerName} (${serviceTag}) in [${playlistDef.name}]`);
        return ticket;
    }

    /**
     * Get ticket status.
     */
    getTicket(ticketId) {
        return this.tickets.get(ticketId) || null;
    }

    /**
     * Cancel an existing ticket.
     */
    cancelTicket(ticketId) {
        const ticket = this.tickets.get(ticketId);
        if (!ticket) return false;
        ticket.status = 'CANCELED';
        this.tickets.delete(ticketId);
        console.log(`[matchmaker] Canceled ticket ${ticketId}`);
        return true;
    }

    /**
     * Process matchmaking queues.
     */
    processQueues() {
        const now = Date.now();

        // 1. Clean up expired tickets
        for (const [id, ticket] of this.tickets) {
            if (ticket.status === 'QUEUED' && (now - ticket.queuedAt) > MATCHMAKING_TICKET_TIMEOUT_MS) {
                ticket.status = 'EXPIRED';
                this.tickets.delete(id);
                console.log(`[matchmaker] Ticket ${id} expired after ${MATCHMAKING_TICKET_TIMEOUT_MS / 1000}s`);
            }
        }

        // 2. Group queued tickets by playlist
        const byPlaylist = new Map();
        for (const ticket of this.tickets.values()) {
            if (ticket.status !== 'QUEUED') continue;
            if (!byPlaylist.has(ticket.playlist)) {
                byPlaylist.set(ticket.playlist, []);
            }
            byPlaylist.get(ticket.playlist).push(ticket);
        }

        // 3. For each playlist, match tickets
        for (const [playlistId, queuedList] of byPlaylist) {
            const playlistDef = PLAYLISTS[playlistId] || PLAYLISTS.slayer;

            // Strategy A: Match to an available open registered dedicated server
            if (this.sessionManager) {
                const availableServers = this.sessionManager.findJoinableServers({
                    mode: 'Multiplayer',
                    requiredSlots: 1,
                    hasPassword: false,
                });

                if (availableServers.length > 0) {
                    const server = availableServers[0];
                    const matchedTicket = queuedList.shift();
                    if (matchedTicket) {
                        this.fulfillTicket(matchedTicket, {
                            matchId: 'srv_' + server.id,
                            serverAddress: server.address,
                            serverPort: server.port,
                            serverName: server.name,
                            map: server.map,
                            mode: server.mode,
                            playlist: playlistDef.name,
                        });
                        continue;
                    }
                }
            }

            // Strategy B: Match tickets together when quorum reached
            if (queuedList.length >= playlistDef.minPlayers) {
                const batch = queuedList.splice(0, playlistDef.maxPlayers);
                const matchId = 'm_' + crypto.randomBytes(8).toString('hex');
                // The first player or dedicated coordinator acts as host/broker
                const hostTicket = batch[0];
                const matchResult = {
                    matchId,
                    serverAddress: hostTicket.clientIp === '::1' ? '127.0.0.1' : hostTicket.clientIp,
                    serverPort: 11774,
                    serverName: `Noble ${playlistDef.name} Match`,
                    map: playlistDef.defaultMap,
                    mode: 'Multiplayer',
                    playlist: playlistDef.name,
                    players: batch.map(b => ({
                        id: b.userId,
                        name: b.playerName,
                        serviceTag: b.serviceTag,
                    })),
                };

                this.matches.set(matchId, matchResult);

                for (const t of batch) {
                    this.fulfillTicket(t, matchResult);
                }

                console.log(`[matchmaker] Created match ${matchId} for playlist [${playlistDef.name}] with ${batch.length} players`);
            }
        }
    }

    fulfillTicket(ticket, matchResult) {
        ticket.status = 'MATCHED';
        ticket.matchId = matchResult.matchId;
        ticket.matchResult = matchResult;
        console.log(`[matchmaker] Ticket ${ticket.id} MATCHED -> ${matchResult.serverAddress}:${matchResult.serverPort}`);
    }

    getStats() {
        let queuedCount = 0;
        const byPlaylist = {};
        for (const key of Object.keys(PLAYLISTS)) {
            byPlaylist[key] = 0;
        }

        for (const t of this.tickets.values()) {
            if (t.status === 'QUEUED') {
                queuedCount++;
                if (byPlaylist[t.playlist] !== undefined) {
                    byPlaylist[t.playlist]++;
                }
            }
        }

        return {
            totalQueued: queuedCount,
            activeMatches: this.matches.size,
            playlists: byPlaylist,
        };
    }
}

const matchmakerInstance = new Matchmaker();

module.exports = {
    PLAYLISTS,
    Matchmaker,
    matchmaker: matchmakerInstance,
};
