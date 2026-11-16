# Scheduler and object-job scratch layouts

## Shared scheduler

| Item | Stock | V16 |
|---|---|---|
| Jobs | 4,096 | 16,384 |
| Dependency records | 4,096 | 65,536 |
| Ready and free queues, each | 4,096 | 16,384 |
| Owner base RVA | `0x02D6F3D0` | `0x14EE5000` |
| Owner bytes | `0x8C0F8` | `0x2900F8` |
| Body offset from owner | `+0x88` | `+0x88` |

Offsets below are relative to the scheduler body, not the owner.

| Body field | Stock offset | V16 offset | Record bytes |
|---|---|---|---:|
| Metadata | `0x00000` | `0x000000` | 12 |
| Job records | `0x0C000` | `0x030000` | 96 |
| Dependency records | `0x6C000` | `0x1B0000` | 8 |
| Dependency header/locks | `0x74000` | `0x230000` | — |
| Ready queue | `0x7401C` | `0x23001C` | — |
| Free-job queue | `0x80038` | `0x260038` | — |
| Worker metadata | `0x8C054` | `0x290054` | — |

Each queue has two-dword links, then four-byte values, then a 28-byte header. The values offset changes 0x8000 → 0x20000; the header offset changes 0xC000 → 0x30000. The worker context pointer at owner+0x8C0F0 moves to owner+0x2900F0. Biased record arithmetic changes `(index+512)*96` → `(index+2048)*96`, with matching inverse conversions.

## Per-call object-job scratch

| Item / offset from context | Stock | V16 |
|---|---|---|
| Capacity | 2,048 | 16,384 |
| Eight-byte handles | `+0x0000` | `+0x00000` |
| Eight-byte node pointers | `+0x4000` | `+0x20000` |
| One-byte flags | `+0x8000` | `+0x40000` |
| Prepared count | `+0x8800` | `+0x44000` |
| Manager pointer | `+0x8808` | `+0x44008` |
| State byte | `+0x8810` | `+0x44010` |
| Caller stack reservation | `0x88A0` | `0x440A0` |
| GS/EH cookie metadata displacement | `0x8892` | `0x44092` |

The caller is at RVA 0x0046A55C. Helpers at 0x0068ADD0 (prepare), 0x0068AEB8 (bind), and 0x0068AD4C (cleanup) share this layout. Context remains at body RSP+0x70; EH state remains at RSP+0x60. Saved-register positions, stack restoration, unwind data and cookie operands move together.

## Scheduler pointer/counter corrections included in V16

| Operand RVA | File offset | Stock bytes | V16 bytes | Meaning |
|---|---|---|---|---|
| `0x0068AAFF` | `0x00689EFF` | `F0 C0 08 00` | `F0 00 29 00` | main-thread job wait worker-context pointer |
| `0x0000CB08` | `0x0000BF08` | `C0 00 08 00` | `C0 00 26 00` | shutdown free-job queue pointer |
| `0x0046C151` | `0x0046B551` | `B0 C0 00 00` | `B0 00 03 00` | owner-relative frame-fence pending counter |

The job counts and final-index operands are listed in `KEY_FIELDS.md`. All other scheduler/scratch instruction displacements, constructor sizes, helper operands and unwind changes are listed by file offset in `COORDINATED_OPERANDS.csv` and covered by the complete overwrite recipe.
