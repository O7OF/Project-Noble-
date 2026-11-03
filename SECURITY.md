# Security policy

## Supported versions

The current release line on
[Project Noble releases](https://github.com/O7OF/Project-Noble-/releases) gets
security fixes. Older builds are not supported.

## Reporting a vulnerability

Do **not** open a public issue for security problems. Contact the maintainers
privately through GitHub (open a draft security advisory on the repository, or
message @O7OF). Include:

- the affected version and target (client/dev/dedicated),
- reproduction steps,
- the impact (crash, memory corruption, credential exposure, ...),
- any logs or a minidump if available.

We will acknowledge within a few days and coordinate a fix and disclosure.

## Scope

Noble loads the Halo: Reach engine from a local MCC install and reads game
memory. Reports about the client's own code, its network protocol, the
updater, or the dedicated server are in scope. Issues in MCC, Steam or the
Halo engine itself are out of scope and should go to Microsoft/343.
