# Passive logger lifecycle and versioned UART service

Status: ready-for-agent
Blocked by: 01

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

new main/logger.c/h and helpers, sdkconfig.defaults, additive main/CMakeLists.txt

## Change

Implement the complete frozen UART v1 and passive capture in /tmp/dashboard-logger-contract.md. Manual logger entry only, ordered ownership release, exact config, filters before queue, drop-new accounting, hw overflow uncertainty, ACK/heartbeat, timeout, duration, exit, session identity, distinct TX/reader status. Board owns hardware access.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
