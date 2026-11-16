# Security policy

## Scope

This is a private repository. Security reports are handled internally by the
maintainer, not through public issues or discussions.

## Reporting

Report a vulnerability directly to the maintainer (@O7OF) through a private
channel — GitHub private vulnerability reporting on this repository, or a
direct message. Do not open a public issue. Include:

- the affected version and target (client/dev/dedicated),
- reproduction steps,
- the impact (crash, memory corruption, credential exposure, ...),
- any logs or a minidump if available.

## Supported versions

Only the current private release line is supported. The frozen open-source
0.9.16.35 base no longer receives security fixes.

## Out of scope

Noble loads the Halo: Reach engine from a local MCC install and reads game
memory. Issues in MCC, Steam or the Halo engine itself are out of scope.
