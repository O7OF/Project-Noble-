const fs = require('fs');

let s = fs.readFileSync('src/engine/BlamEngine.cpp', 'utf8');
const isCrlf = s.includes('\r\n');
const toFileEol = (t) => isCrlf ? t.replace(/\r?\n/g, '\r\n') : t.replace(/\r\n/g, '\n');

const oldSnippet = `    // PostCommand(10) and PostCommand(11) via vtbl+0x18 after ~300 ms (verified from host-probe main.c lines 4306-4323)
    std::thread([this]() {
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

const newSnippet = `    const bool isGuest = !connectAddress.empty();
    if (!isGuest) {
        // PostCommand(10) and PostCommand(11) via vtbl+0x18 after ~300 ms for host / offline matches.
        // On a guest connecting to a remote host, posting command 10/11 forces local simulation before
        // session membership and world replication arrive from the host, causing an unrendered black screen.
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            if (m_engineInstance && m_fnProcessEvent) {
                NOBLE_INFO("BlamEngine: Posting Command 10 (Event: Start) via vtbl+0x18 after 300ms delay...");
                int32_t status10 = m_fnProcessEvent(m_engineInstance, 10, 0);
                NOBLE_INFO("BlamEngine: PostCommand(10) returned: 0x{:X}", status10);

                NOBLE_INFO("BlamEngine: Posting Command 11 via vtbl+0x18...");
                int32_t status11 = m_fnProcessEvent(m_engineInstance, 11, 0);
                NOBLE_INFO("BlamEngine: PostCommand(11) returned: 0x{:X}", status11);
            }
        }).detach();
    } else {
        NOBLE_INFO("BlamEngine: [GUEST] Remote join active - skipping manual PostCommand(10/11); waiting for host session sync.");
    }`;

if (!s.includes(toFileEol(oldSnippet))) {
    console.error('Target snippet not found in BlamEngine.cpp!');
    process.exit(1);
}
s = s.replace(toFileEol(oldSnippet), toFileEol(newSnippet));
fs.writeFileSync('src/engine/BlamEngine.cpp', s, 'utf8');
console.log('BlamEngine.cpp updated successfully!');

