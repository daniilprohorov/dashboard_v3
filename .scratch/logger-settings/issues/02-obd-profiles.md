# Restartable OBD and persisted dashboard profiles

Status: ready-for-agent
Blocked by: 01
Implementation: completed
Implementation commit: [1184e4d](https://github.com/daniilprohorov/dashboard_v3/commit/1184e4d)
Review repair: `impl/logger-review-fixes` (shared CAN release helper)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

main/obd.c, main/obd.h, new main/profile.c/h, additive main/CMakeLists.txt

## Change

Implement checked NVS profile storage and all four OBD profiles, real extended addressing, bounded safe stop/join, reset stale measurements/PID support. Read shared contract. UI and app caller migration belongs 05.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
