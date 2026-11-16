# C12 step 2 review: the 58 remaining slot-address sites

Scope: docs/limits-players-rest.json (66 entries, produced by slot_rewrite.py from the Run 1-4 trace).
Sites whose containing function is one of Claude's loops (0xD6B8, 0x3915A8, 0x392C90, 0x39DD4C,
0x408DDC, 0x41F510) are excluded. Method: stock haloreach.dll disassembled with capstone, function
ranges from .pdata, ~30 instructions before each site plus function callers from an E8 scan.

Note: excluding the six functions leaves **58** sites, not 54. The four-site difference is function-
boundary attribution (the function at 0x390DB0 ends exactly at 0x3915A8, and 0x392DB0 follows 0x392C90);
all 58 are reviewed here, so nothing is dropped.

Classes: A = object field / non-slot pointer (no change); B = slot pointer from an already handled
imul/site (covered); C = slot pointer from an argument or memory (producer named); D = new shape needing
its own detour.

| # | site | function | derivation | class | evidence / producer |
|---:|---|---|---|---|---|
| 1 | sub@0x35c18 | - | sub    rdx,rax | **C** | generic UTF-16 helper (0x35BF4, no .pdata entry); slot pointer passed in rdx/rcx; no direct callers found (indirect) |
| 2 | load@0x35c2b | - | movzx  ecx,WORD PTR [rdx+rax*1] | **C** | same helper; reads the slot bytes at [rdx+rax] |
| 3 | other@0x9d648 | - | inc    r10 | **C** | byte-buffer helper at 0x9D624; pointer from rdx argument; no direct callers |
| 4 | other@0x14f4c9 | 0x14F430 | inc    eax | **A** | players-mask bit loop (bt [r8+0x173c]); scalar field, not a slot record |
| 5 | add@0x16ff21 | 0x16FE7C | add    rax,rdx | **B** | rax=rdi+0x60 (peers base) + imul 0x158 at 0x16FF10 |
| 6 | add@0x16ff66 | 0x16FE7C | add    rax,r11 | **B** | 0x80-byte block copy inside the peer record from 0x16FF21 |
| 7 | load@0x38dab8 | 0x38DA74 | mov    rax,QWORD PTR [rsi+rcx*8] | **C** | rax=[rsi+rcx*8], rsi=r9 argument; pointer array supplied by callers 0x39120D/0x391402 |
| 8 | lea@0x38f53b | 0x38F49C | lea    rbp,[rbx+0x30] | **A** | object field: rbp=rbx+0x30 |
| 9 | add@0x390ee4 | 0x390DB0 | add    eax,r14d | **A** | players-mask bit loop (bt [rbx+0x173c]) |
| 10 | add@0x391006 | 0x390DB0 | add    rdi,rcx | **B** | rdi=rbx+0x1760 (players base) + imul 0xB0 at 0x390FF5 |
| 11 | lea@0x3911c1 | 0x390DB0 | lea    rcx,[rbp+0x1d0] | **A** | object field: rcx=rbp+0x1D0 passed to 0x35C3C |
| 12 | add@0x391350 | 0x390DB0 | add    rax,rcx | **B** | rax=rbx+0x1760 + imul 0xB0 at 0x391342 |
| 13 | add@0x3913d6 | 0x390DB0 | add    rdx,rsi | **B** | rdx=rbx+0x1760 + imul 0xB0 at 0x3913B2 |
| 14 | lea@0x391a85 | 0x3919E4 | lea    r8,[rdx+0x8] | **A** | record field: r8=rdx+8 (word compare of a non-slot record) |
| 15 | lea@0x391b5d | 0x3919E4 | lea    rbx,[r8+r13*1] | **C** | r8+r13, r8 an argument base; callers 0x4095EF/0x40A4F1/0x4143B3 |
| 16 | lea@0x393112 | 0x392DB0 | lea    r12,[rdi+0x168] | **A** | object field: r12=rdi+0x168 |
| 17 | lea@0x393376 | 0x392DB0 | lea    r9,[rax+rdi*1] | **B** | rax=imul 0x158 at 0x393361; lea r9=[rax+rdi] |
| 18 | not@0x394c01 | 0x394BD8 | mov    rbx,rcx | **C** | entry argument copy rbx=rcx; caller 0x390D07 supplies the session object |
| 19 | lea@0x396cab | 0x396828 | lea    rbx,[rsi+0x2248] | **A** | object field: rbx=rsi+0x2248 |
| 20 | add@0x397432 | 0x3973BC | add    rax,rcx | **B** | rax=rdi+0x60 + imul 0x158 at 0x397424 |
| 21 | load@0x397449 | 0x3973BC | mov    r10,QWORD PTR [rsp+0x70] | **B** | reload of the slot pointer stored to [rsp+0x70] by the site at 0x397439 |
| 22 | not@0x3973e5 | 0x3973BC | mov    rdi,rcx | **C** | entry argument copy rdi=rcx; callers 0x392297/0x392E8B/0x393308/0x394A59 |
| 23 | lea@0x3982ef | 0x398200 | lea    r12,[rdx+r8*1] | **B** | rdx=imul 0x158 at 0x3982C8; lea r12=[rdx+r8] |
| 24 | not@0x39cd0f | 0x39CD00 | mov    rbx,rdx | **C** | entry argument copy rbx=rdx; callers 0x38FE5C/0x394ED1/0x394FEC |
| 25 | add@0x39d24f | 0x39D1F8 | add    rbx,rdx | **A** | object-owned 0x158-stride table at object+0x48 (not the session slot arrays) |
| 26 | add@0x39d297 | 0x39D1F8 | add    rax,r13 | **A** | copy inside the object+0x48 table, r13 record stride |
| 27 | add@0x39d496 | 0x39D1F8 | add    rbx,r13 | **A** | copy inside the object+0x48 table, r13 record stride |
| 28 | not@0x39d64c | 0x39D620 | movsxd rdi,DWORD PTR [rcx+r10*4+0x174] | **A** | 0x56-stride map table (imul 0x56 at 0x39D63F); not a player/peer record |
| 29 | add@0x39d8e8 | 0x39D8A8 | add    r11,r10 | **B** | imul 0xB0 at 0x39D8E1; r11 += index*0xB0 |
| 30 | add@0x39d91e | 0x39D8A8 | add    r8,rcx | **B** | imul 0x158 at 0x39D917; r8 += rcx base |
| 31 | add@0x39d95e | - | add    r10,rcx | **B** | imul 0xB0 at 0x39D957 (no .pdata entry for the function) |
| 32 | add@0x39d988 | - | add    rdx,rcx | **B** | imul 0x158 at 0x39D97A |
| 33 | add@0x39e586 | 0x39E47C | add    r14,rbx | **B** | imul 0x158 at 0x39E57F; r14 += rbx (session base) |
| 34 | lea@0x39e8e3 | 0x39E47C | lea    r12,[rbx+0x8] | **A** | object field: r12=rbx+8 |
| 35 | not@0x39edce | 0x39EDA8 | mov    DWORD PTR [rcx],eax | **A** | field write [rcx]=eax (output argument), no slot indexing |
| 36 | not@0x39edc3 | 0x39EDA8 | mov    rsi,rcx | **C** | entry argument copy rsi=rcx; callers 0x39039D/0x39D424/0x39E6E0 |
| 37 | lea@0x39ef92 | 0x39EDA8 | lea    rcx,[rsi+0x114] | **A** | object field: rcx=rsi+0x114 |
| 38 | add@0x39f05f | 0x39F02C | add    rcx,rax | **A** | object+0x48 0x158-stride table: rcx += rax (imul 0x158 at 0x39F04E) |
| 39 | add@0x39f0c7 | 0x39F02C | add    r11,rbx | **B** | imul 0x158 at 0x39F0C0; r11 += rbx (session base) |
| 40 | not@0x39f04b | 0x39F02C | mov    rbx,rcx | **C** | entry argument copy rbx=rcx; callers 0x39C96B/0x39D44F |
| 41 | not@0x39f447 | 0x39F420 | mov    eax,DWORD PTR [rdx] | **A** | field load [rdx] for an object compare |
| 42 | not@0x39f449 | 0x39F420 | cmp    DWORD PTR [rcx],eax | **A** | field compare [rcx] against eax |
| 43 | not@0x39f451 | 0x39F420 | mov    eax,DWORD PTR [rcx] | **A** | field load [rcx] |
| 44 | not@0x39f43f | 0x39F420 | mov    rbx,rcx | **C** | entry argument copy rbx=rcx; callers 0x390319/0x390494/0x39E002 |
| 45 | lea@0x39f4c9 | 0x39F420 | lea    rcx,[rbx+0x84] | **A** | object field: rcx=rbx+0x84 |
| 46 | lea@0x39f4d2 | 0x39F420 | lea    rax,[rdi+0x84] | **A** | object field: rax=rdi+0x84 |
| 47 | sub@0x39f4ff | 0x39F420 | sub    rcx,rax | **A** | relative compare of two +0x84 fields (rcx-rax) |
| 48 | lea@0x39f502 | 0x39F420 | lea    rdx,[rax+0x8] | **A** | object field: rdx=rax+8 |
| 49 | not@0x39f43c | 0x39F420 | mov    rdi,rdx | **C** | entry argument copy rdi=rdx; callers 0x390319/0x390494/0x39E002 |
| 50 | lea@0x39f7e1 | 0x39F420 | lea    rcx,[rdi+0x114] | **A** | object field: rcx=rdi+0x114 |
| 51 | sub@0x39f7eb | 0x39F420 | sub    r8,rdi | **A** | relative compare of +0x114 fields |
| 52 | sub@0x39f80e | 0x39F420 | sub    rdx,rsi | **A** | relative pointer for a field copy |
| 53 | add@0x39fb69 | 0x39FB34 | add    r9,rcx | **B** | imul 0x158 at 0x39FB62; r9 += rcx (session base) |
| 54 | load@0x3dc27a | 0x3DC214 | mov    rsi,QWORD PTR [rsp+0x28] | **C** | reload of the pointer written by call 0x38FB68 (out slot [rsp+0x28]); caller 0x3AE71B |
| 55 | add@0x3ed5e3 | 0x3ED3F4 | add    r11,r13 | **B** | imul 0x158 at 0x3ED5DC; r11 += r13 (session base) |
| 56 | not@0x3f6ff4 | 0x3F6FC0 | mov    r14,r8 | **C** | entry argument copy r14=r8; callers 0x39C55C/0x39D03F/0x3CD7AB/0x403271 |
| 57 | sub@0x408178 | 0x408068 | sub    rdx,r9 | **A** | relative compare of object fields (no direct callers) |
| 58 | not@0x44b1f5 | 0x44B1DC | mov    rbx,rcx | **C** | entry argument copy rbx=rcx; caller 0x42AAA4 |

**Totals: 26 A, 17 B, 15 C, 0 D** (58 sites).

## Summary

- **A (26)**: object fields, record copies, mask-bit loops and field compares. None of them index the
  session slot arrays, so they need no change.
- **B (17)**: slot pointers formed by the 0x158/0xB0 record multiply (or reloaded from a store fed by one).
  These are the same shapes the slot rewrite already covers; extending the loops needs no new detours.
- **C (15)**: the base comes from a function argument or from a pointer returned by a call. Producers are
  named in the table (caller RVAs / out-parameter calls). They pass the session object or an array from code
  already reached through the patched loops, so the pointers follow the same layout; no separate detour is
  required, but these are the sites to re-check if a caller ever passes a stack copy of a slot record.
- **D (0)**: no new derivation shape found.

Nothing here changes code; this is the patch inventory for the 54/58 sites Claude asked to review.