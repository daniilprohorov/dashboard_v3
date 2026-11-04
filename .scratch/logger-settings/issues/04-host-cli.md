# macOS/Linux CLI and event CSV

Status: ready-for-agent
Blocked by: none

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

tools/can_logger

## Change

Implement public dashboard-can CLI and constant event CSV exactly per shared contract. Serial ownership, discovery without reset, strict active parsing, command correlation, errors/exit codes, timed and Ctrl-C capture, ACKs, user_exit/local termination, no invented final counts. Permanent tests/docs belong 06.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
