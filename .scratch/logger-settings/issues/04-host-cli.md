# macOS/Linux CLI and event CSV

Status: ready-for-agent
Blocked by: none
Implementation: completed
Implementation commit: [cbe8894](https://github.com/daniilprohorov/dashboard_v3/commit/cbe8894)
Review repair: `impl/logger-review-fixes` (USB-only automatic probing and distinct response_timeout)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

tools/can_logger

## Change

Implement public dashboard-can CLI and constant event CSV exactly per shared contract. Serial ownership, discovery without reset, strict active parsing, command correlation, errors/exit codes, timed and Ctrl-C capture, ACKs, user_exit/local termination, no invented final counts. Permanent tests/docs belong 06.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
