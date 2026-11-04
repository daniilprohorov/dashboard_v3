# Exact CAN timing and UART board primitives

Status: ready-for-agent
Blocked by: none

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

components/board

## Change

Provide exact bitrate validation/creation for 1000..1000000, explicit listen-only, and fixed UART0 2M8N1 board primitives. Preserve physical pin routing and lifecycle errors. Read /tmp/dashboard-logger-contract.md Board contract. Existing caller migration belongs 02.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
