# Settings, Logger screen and exclusive app orchestration

Status: ready-for-agent
Blocked by: 02,03
Implementation: completed
Implementation commit: [cb508cd](https://github.com/daniilprohorov/dashboard_v3/commit/cb508cd)
Review repair: `impl/logger-review-fixes` (LVGL 9 event-bubble setter)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).

## Target

main/main.c, main/dashboard.c/h, new app controller and screen sources, main/CMakeLists.txt

## Change

Integrate all four applied/saved/draft profiles and logger lifecycle via background app controller outside LVGL lock. Preserve main UI/error text, OBD during settings, explicit Apply/error display, manual Logger only, Exit-only logger screen with full effective filter/state/ACK/loss/reason. Use retained live widgets/timers safely. Boot saved dashboard profile only. Read shared contract and upstream headers.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
