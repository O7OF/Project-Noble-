const fs = require('fs');
const path = require('path');

const lines = [];
lines.push('#include <windows.h>');
lines.push('');

function add(name, relPath) {
    // Relative to src/res
    lines.push(name + ' RCDATA "../../' + relPath + '"');
}

add('LAUNCH_BLOCK_1_BIN', 'data/launch_block_1.bin');
add('LB_M_C_1_BIN', 'data/lb_m_c_1.bin');
add('PROFILE_1_BIN', 'data/profile_1.bin');

const blocksDir = path.join(process.cwd(), 'data', 'blocks');
if (fs.existsSync(blocksDir)) {
    const files = fs.readdirSync(blocksDir).filter(f => f.endsWith('.bin'));
    for (const f of files) {
        const name = f.toUpperCase().replace(/\./g, '_');
        add(name, 'data/blocks/' + f);
    }
}

fs.writeFileSync('src/res/blocks.rc', lines.join('\r\n') + '\r\n');
console.log('Done! Generated src/res/blocks.rc with relative paths, count:', lines.length - 2);
