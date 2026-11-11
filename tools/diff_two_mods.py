
import os
ws = r"C:\Program Files (x86)\Steam\steamapps\workshop\content\976730"
a = os.path.join(ws, '2899383598', 'maps', 'forge_halo_normal.map')   # works
b = os.path.join(ws, '2899728214', 'maps', 'cex_ff_halo_normal.map')  # fails
N = 0x2000
def head(p):
    with open(p, 'rb') as f: return f.read(N)
A, B = head(a), head(b)
diffs = [(i, A[i], B[i]) for i in range(N) if A[i] != B[i]]
print(f"differing bytes in first 0x{N:X}: {len(diffs)}")
for i, x, y in diffs[:40]:
    print(f"0x{i:04X}: works=0x{x:02X} fails=0x{y:02X}")
