# CLI behavioral acceptance

Status: ready-for-agent
Blocked by: 04
Implementation: completed
Implementation commit: [c770af2](https://github.com/daniilprohorov/dashboard_v3/commit/c770af2)
Review repair: `impl/logger-review-fixes` (isolated Darwin PTY adapter, fail-fast peer, public timeout regression)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

tools/can_logger tests

## Change

Add deterministic public CLI subprocess/PTY acceptance scenarios across spec boundaries; no private-implementation tests. Cover strict boot/active stream handling, async command correlation, exact configuration, frame metadata, losses/unknown counts, timer/Ctrl-C/user_exit/link loss, repeat captures, invalid arguments, version/rejection and JSON exit codes. Serial-peer evidence never proves CAN/CH343/UI. Parent runs tests and smoke after final merge. Actual operator docs belong ticket 07, allowing tests to proceed independently of firmware/UI.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
