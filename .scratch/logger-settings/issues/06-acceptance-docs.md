# CLI behavioral acceptance

Status: ready-for-agent
Blocked by: 04

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

tools/can_logger tests

## Change

Add deterministic public CLI subprocess/PTY acceptance scenarios across spec boundaries; no private-implementation tests. Cover strict boot/active stream handling, async command correlation, exact configuration, frame metadata, losses/unknown counts, timer/Ctrl-C/user_exit/link loss, repeat captures, invalid arguments, version/rejection and JSON exit codes. Serial-peer evidence never proves CAN/CH343/UI. Parent runs tests and smoke after final merge. Actual operator docs belong ticket 07, allowing tests to proceed independently of firmware/UI.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
