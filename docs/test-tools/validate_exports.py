# Validate every exported Discord json under D:\discord chat\full and print a summary table. Deletes nothing.
import json, os, sys
root = r"D:\discord chat\full"
rows = []
for dp, dn, fn in os.walk(root):
    for f in fn:
        if not f.lower().endswith('.json'): continue
        p = os.path.join(dp, f)
        size = os.path.getsize(p)
        try:
            with open(p, 'r', encoding='utf-8') as fh: j = json.load(fh)
            m = j.get('messages', [])
            ts = [x['timestamp'] for x in m] if m else []
            rows.append((os.path.relpath(p, root)[:80], size, 'OK', len(m), (min(ts)[:10] if ts else ''), (max(ts)[:10] if ts else '')))
        except Exception as ex:
            rows.append((os.path.relpath(p, root)[:80], size, 'BAD ' + str(ex)[:40], 0, '', ''))
rows.sort()
for r in rows: print('%-80s %10d  %-8s msgs=%-6d %s..%s' % r)
ok = [r for r in rows if r[2] == 'OK']
print('\nfiles: %d  ok: %d  bad: %d  messages: %d  MB: %.1f' % (len(rows), len(ok), len(rows)-len(ok), sum(r[3] for r in ok), sum(r[1] for r in rows)/1048576))
