const fs = require('fs');
let c = fs.readFileSync('src/engine/BlamEngine.cpp', 'utf8');

// 1. Add include
if (!c.includes('hooks/LobbyPatches.hpp')) {
    c = c.replace('#include "hooks/HookManager.hpp"', '#include "hooks/HookManager.hpp"\r\n#include "hooks/LobbyPatches.hpp"');
}

// 2. Add reset at start of LaunchMap
const targetReset = 'bool BlamEngine::LaunchMap(const std::string& mapName, const std::string& gameMode, const std::string& connectAddress, int expectedPeers) {';
if (!c.includes('LobbyPatches::ResetRemotePlayerJoined()')) {
    c = c.replace(targetReset, targetReset + '\r\n    noble::LobbyPatches::ResetRemotePlayerJoined();');
}

// 3. Update the wait before Command 11
const oldWaitStart = c.indexOf('if (peersToWaitFor > 0 && !holdMissionForPeers) {');
const oldWaitEnd = c.indexOf('NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");');

if (oldWaitStart !== -1 && oldWaitEnd !== -1) {
    const newWait = `const size_t targetPeers = (peersToWaitFor > 0) ? static_cast<size_t>(peersToWaitFor)
                                         : (joinedBeforeStart > 0 ? 1 : 0);
                if (targetPeers > 0) {
                    const ULONGLONG deadline = GetTickCount64() + 60000;
                    ULONGLONG lastLog = 0;
                    NOBLE_INFO("BlamEngine: waiting for {} remote player(s) to finish loading and join membership before starting simulation...",
                               targetPeers);
                    while (GetTickCount64() < deadline) {
                        const size_t playersJoined = LobbyPatches::GetRemotePlayerAddCount();
                        if (playersJoined >= targetPeers) {
                            NOBLE_INFO("BlamEngine: {}/{} remote player(s) joined session membership (msg 27); settling {} ms for world replication",
                                       playersJoined, targetPeers, settleLonger ? 4000 : 2000);
                            std::this_thread::sleep_for(std::chrono::milliseconds(settleLonger ? 4000 : 2000));
                            break;
                        }
                        const ULONGLONG now = GetTickCount64();
                        if (lastLog == 0 || now - lastLog >= 5000) {
                            lastLog = now;
                            NOBLE_INFO("BlamEngine: membership wait: {}/{} player(s) in session (engine peers: {})",
                                       playersJoined, targetPeers, NetworkTransport::Get().GetGamePeerCount());
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    if (LobbyPatches::GetRemotePlayerAddCount() < targetPeers) {
                        NOBLE_WARN("BlamEngine: membership wait timed out with {}/{} player(s); starting simulation anyway",
                                   LobbyPatches::GetRemotePlayerAddCount(), targetPeers);
                    }
                } else {
                    const int simDelay = isOnlineHost ? 5000 : 200;
                    std::this_thread::sleep_for(std::chrono::milliseconds(simDelay));
                }

                `;
    c = c.substring(0, oldWaitStart) + newWait + c.substring(oldWaitEnd);
    console.log('Replaced Command 11 wait successfully');
} else {
    console.log('Could not find oldWait block', oldWaitStart, oldWaitEnd);
}

fs.writeFileSync('src/engine/BlamEngine.cpp', c, 'utf8');
console.log('Saved src/engine/BlamEngine.cpp');

