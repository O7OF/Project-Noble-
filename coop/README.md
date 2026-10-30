# coop/ - the shared bus for two-machine co-op tests

Do not hand-edit anything here except `REPORT.md`.

| File | Who writes it | Why |
|---|---|---|
| `request.json` | the side asking for a test | pins the commit both machines must run |
| `endpoint.txt` | `tools\coop-test.ps1 -Role host` | the address the guest dials |
| `host-<stamp>.log` | `tools\coop-test.ps1 -Role host` | filtered host evidence: peer barrier, Command 10/11, aborts |
| `guest-<stamp>.log` | `tools\coop-test.ps1 -Role guest` | filtered guest evidence: connect, session start, aborts |
| `result-<role>-<stamp>.json` | both | status + commit + endpoint + log path |
| `REPORT.md` | whoever analyses | the conclusion, with the deciding lines quoted verbatim |

Protocol: `docs/COOP-TEST-PROTOCOL.md`.

Never commit a full `noble-client.log` here - those are megabytes. Only the filtered artefacts.
