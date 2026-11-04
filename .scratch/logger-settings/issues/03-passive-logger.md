# Passive logger lifecycle and versioned UART service

Status: ready-for-agent
Blocked by: 01
Implementation: completed
Implementation commit: [f11a6b4](https://github.com/daniilprohorov/dashboard_v3/commit/f11a6b4)
Review repair: `impl/logger-review-fixes` (no RTOS calls under mux, joined stop and terminal ordering)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

new main/logger.c/h and helpers, sdkconfig.defaults, additive main/CMakeLists.txt

## Change

Implement the complete frozen UART v1 and passive capture in /tmp/dashboard-logger-contract.md. Manual logger entry only, ordered ownership release, exact config, filters before queue, drop-new accounting, hw overflow uncertainty, ACK/heartbeat, timeout, duration, exit, session identity, distinct TX/reader status. Board owns hardware access.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
