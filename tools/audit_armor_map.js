const fs = require('fs');
const path = require('path');
const objPath = process.argv[2];
const pmcgPath = process.argv[3];

const perms = new Set();
const lines = fs.readFileSync(objPath, 'utf8').split(/\r?\n/);
for (const line of lines) {
  if (line.startsWith('g -')) perms.add(line.slice(3).trim());
}

const text = fs.readFileSync(pmcgPath, 'latin1');
const items = new Map(); // list -> id -> [{region,perm}]
let list = null, item = null;
for (const raw of text.split(/\r?\n/)) {
  const m = raw.match(/^(L|S|C)\s+(.+)$/);
  if (!m) continue;
  const kind = m[1];
  const parts = m[2].trim().split(/\s+/);
  if (kind === 'L') list = parts[0];
  else if (kind === 'S') { item = parts[0]; if (!items.has(list)) items.set(list, new Map()); items.get(list).set(item, []); }
  else if (kind === 'C' && list && item) items.get(list).get(item).push({ region: parts[0], perm: parts[1] });
}

const resolve = (name) => {
  if (perms.has(name)) return name;
  if (perms.has('player_' + name)) return 'player_' + name;
  return null;
};

const want = ['head', 'flair_head_a', 'flair_head_b', 'flair_head_c'];
let bad = 0, total = 0;
for (const [listName, byItem] of items) {
  if (!['20'].includes(listName)) continue;
  for (const [id, changes] of byItem) {
    for (const c of changes) {
      if (!want.includes(c.region)) continue;
      total++;
      const hit = resolve(c.perm);
      if (!hit) {
        bad++;
        // near matches: same suffix/prefix
        const near = [...perms].filter(p => p.includes(c.perm) || c.perm.includes(p.replace('player_',''))).slice(0, 6);
        console.log('MISS ' + listName + ' ' + id + ' ' + c.region + '=' + c.perm + '  near: ' + near.join(', '));
      }
    }
  }
}
console.log('checked ' + total + ' head changes, ' + bad + ' unresolved');
console.log('obj perms: ' + perms.size);
