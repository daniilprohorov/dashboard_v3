# Actual operator contracts and acceptance record

Status: ready-for-agent
Blocked by: 05,06
Implementation: completed
Implementation commit: [943cf62](https://github.com/daniilprohorov/dashboard_v3/commit/943cf62)
Review repair: `impl/logger-review-fixes` (final discovery/error schema, truthful PTY evidence and limitations)
PR: [#1](https://github.com/daniilprohorov/dashboard_v3/pull/1)
Acceptance evidence: [host record and unverified hardware gates](../../../docs/logger-acceptance.md); final verification belongs to the integration owner.

## Context

- Spec: [../spec.md](../spec.md), complete normative requirements.
- Published implementation contracts: [UART/CSV](../../../docs/logger-protocol.md), [operator/JSON](../../../docs/logger.md).
- Verification procedures and observed evidence: [acceptance record](../../../docs/logger-acceptance.md).

## Target

docs/design.md, new CLI/UART/CSV guide and acceptance record, Makefile discoverability; no product code.

## Change

Document actual protocol/CSV/JSON/exit examples, CLI installation and board-vs-simulator discovery, manual Logger gate, bounded capture/repeat/exit workflow, timestamp and ACK/loss limitations. Explain passive ACK bench requirements, silent IMSB5010, overload, fixed 2M8N1 ownership and flashing. Update obsolete display-only/11bit design text. Describe every hardware acceptance scenario and honestly mark actual unavailable hardware and Linux checks unverified. Parent will supply executed build/CLI results after final integration; do not fabricate examples as captured device output. Distinguish illustrative valid syntax from actually observed serial-peer runs. Add discoverability through Makefile targets only for real commands.

## Acceptance

Complete discoverable instructions covering all spec operational and evidence requirements. Commit assigned docs/Makefile only in own worktree. No builds/tests/formatters mid-flight. Parent verifies final branch and updates evidence before PR readiness.
