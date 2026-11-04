# Actual operator contracts and acceptance record

Status: ready-for-agent
Blocked by: 05,06

## Context

- Spec: [../spec.md](../spec.md), complete normative requirements.
- Shared design contract: `/tmp/dashboard-logger-contract.md`; final code is source of factual details.
- Verification prerequisites: `/tmp/dashboard-logger-verification.md`.

## Target

docs/design.md, new CLI/UART/CSV guide and acceptance record, Makefile discoverability; no product code.

## Change

Document actual protocol/CSV/JSON/exit examples, CLI installation and board-vs-simulator discovery, manual Logger gate, bounded capture/repeat/exit workflow, timestamp and ACK/loss limitations. Explain passive ACK bench requirements, silent IMSB5010, overload, fixed 2M8N1 ownership and flashing. Update obsolete display-only/11bit design text. Describe every hardware acceptance scenario and honestly mark actual unavailable hardware and Linux checks unverified. Parent will supply executed build/CLI results after final integration; do not fabricate examples as captured device output. Distinguish illustrative valid syntax from actually observed serial-peer runs. Add discoverability through Makefile targets only for real commands.

## Acceptance

Complete discoverable instructions covering all spec operational and evidence requirements. Commit assigned docs/Makefile only in own worktree. No builds/tests/formatters mid-flight. Parent verifies final branch and updates evidence before PR readiness.
