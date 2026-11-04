# CLI behavioral acceptance and actual operator contracts

Status: ready-for-agent
Blocked by: 04,05

## Context

- Spec: [../spec.md](../spec.md) (normative, entire acceptance scope).
- Shared implementation contract: `/tmp/dashboard-logger-contract.md` (frozen cross-ticket APIs/wire/CSV).

## Target

tools/can_logger tests, docs/design.md, new CLI/UART/CSV guide and acceptance evidence, Makefile

## Change

Add deterministic public CLI subprocess/PTY acceptance scenarios across spec boundaries; no private-implementation tests. Document actual protocol/CSV/exit examples, installation/discovery/repeat workflow, timing/ACK/loss limitations, hardware bench acceptance and unverified criteria. Update project discoverability and obsolete display-only/11bit docs. No pretend hardware evidence. Parent runs build and smoke after final merge.

## Acceptance

Meet corresponding complete spec criteria and shared contract. Commit on own branch/worktree. Skip builds/tests/formatters mid-flight; final integration owner verifies the complete branch once. Report all limitations honestly.
