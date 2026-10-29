const fs = require('fs');
const file = 'src/engine/BlamEngine.cpp';
let code = fs.readFileSync(file, 'utf8');

const target1 = `        if (!isMainMenu && loadedPrebuilt) {
            if (isGuest) {
                uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
                if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
                    guestPlayerId = localMachineId | 0x504C520000000001ULL;
                }
                block[0] &= ~0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                block[0x2F0] = 0x00;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId;
                block[0x2BF14] = 0xF3;
                block[0x2BF15] = 0x01;
            } else {
                block[0] |= 0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                block[0x2F0] = 0x01;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
            }
        }`;

const repl1 = `        if (!isMainMenu && loadedPrebuilt) {
            if (isGuest) {
                uint64_t guestPlayerId = localMachineId ^ 0x3F59F3BE00000000ULL;
                if (guestPlayerId == 0 || guestPlayerId == 0x000000003F59F3BEULL) {
                    guestPlayerId = localMachineId | 0x504C520000000001ULL;
                }
                block[0] &= ~0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = 0;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = guestPlayerId;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                block[0x100] = 0x00;
                block[0x101] = 0x00;
                block[0x102] = 0x00;
                block[0x103] = 0x00;
                block[0x104] = 0x02;
                block[0x108] = 0x00;
                block[0x109] = 0x00;
                block[0x10A] = 0x00;
                block[0x10B] = 0x00;
                block[0x2F0] = 0x00;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0xFFFFFFFF;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = hostMachineId;
                block[0x2BF14] = 0xF3;
                block[0x2BF15] = 0x01;
            } else {
                block[0] |= 0x08;
                *reinterpret_cast<uint64_t*>(&block[0x60]) = localMachineId;
                block[0xE8] = 0x01;
                *reinterpret_cast<uint64_t*>(&block[0xF0]) = 0x000000003F59F3BEULL;
                *reinterpret_cast<uint64_t*>(&block[0xF8]) = localMachineId;
                block[0x100] = 0x00;
                block[0x101] = 0x00;
                block[0x102] = 0x00;
                block[0x103] = 0x00;
                block[0x104] = 0x02;
                block[0x108] = 0x00;
                block[0x109] = 0x00;
                block[0x10A] = 0x00;
                block[0x10B] = 0x00;
                block[0x2F0] = 0x01;
                *reinterpret_cast<uint32_t*>(&block[0x2F4]) = 0;
                *reinterpret_cast<uint64_t*>(&block[0x2F8]) = 0;
            }
        }`;

const target2 = `        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            if (m_engineInstance && m_fnProcessEvent) {
                NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after 300ms delay...");
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

                NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
            }
        }).detach();`;

const repl2 = `        // If hosting an online match / lobby with network transport active, give remote
        // clients 4000 ms to establish membership sync before posting Command 10/11.
        // For solo / offline matches, 300 ms is sufficient.
        const bool isOnlineHost = noble::NetworkTransport::Get().IsActive() && noble::NetworkTransport::Get().IsHost();
        const int delayMs = isOnlineHost ? 4000 : 300;
        std::thread([this, delayMs]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            if (m_engineInstance && m_fnProcessEvent) {
                NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after {}ms delay...", delayMs);
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

                NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
            }
        }).detach();`;

// Check CRLF vs LF
const t1_crlf = target1.replace(/\n/g, '\r\n');
const r1_crlf = repl1.replace(/\n/g, '\r\n');
const t2_crlf = target2.replace(/\n/g, '\r\n');
const r2_crlf = repl2.replace(/\n/g, '\r\n');

if (code.includes(t1_crlf)) {
    code = code.replace(t1_crlf, r1_crlf);
} else if (code.includes(target1)) {
    code = code.replace(target1, repl1);
} else {
    console.error('Target 1 not found');
}

if (code.includes(t2_crlf)) {
    code = code.replace(t2_crlf, r2_crlf);
} else if (code.includes(target2)) {
    code = code.replace(target2, repl2);
} else {
    console.error('Target 2 not found');
}

fs.writeFileSync(file, code, 'utf8');
console.log('BlamEngine.cpp patched successfully');

