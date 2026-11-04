# Exact CAN timing and UART board primitives

Status: ready-for-agent
Blocked by: none
Implementation: completed
Implementation commit: [e488b6a](https://github.com/daniilprohorov/dashboard_v3/commit/e488b6a), [b443270](https://github.com/daniilprohorov/dashboard_v3/commit/b443270)
Review repair: `impl/logger-review-fixes` (shared CAN release helper and lifecycle fixes)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

components/board

## Change

Provide exact bitrate validation/creation for 1000..1000000, explicit listen-only, and fixed UART0 2M8N1 board primitives. Preserve physical pin routing and lifecycle errors. Read /tmp/dashboard-logger-contract.md Board contract. Existing caller migration belongs 02.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
