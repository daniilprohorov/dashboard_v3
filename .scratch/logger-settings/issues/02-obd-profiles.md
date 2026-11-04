# Restartable OBD and persisted dashboard profiles

Status: ready-for-agent
Blocked by: 01

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

main/obd.c, main/obd.h, new main/profile.c/h, additive main/CMakeLists.txt

## Change

Implement checked NVS profile storage and all four OBD profiles, real extended addressing, bounded safe stop/join, reset stale measurements/PID support. Read shared contract. UI and app caller migration belongs 05.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
