# Domain Docs

Single-context layout: `CONTEXT.md` at the repo root and ADRs under
`docs/adr/`.

## Before exploring

- Read root `CONTEXT.md`.
- Read ADRs in `docs/adr/` relevant to the area being explored.

If these files or directories do not exist, proceed silently. Do not
flag their absence or suggest creating them upfront. The
`domain-modeling` skill creates them lazily when terms or decisions
are resolved.

## Use the glossary's vocabulary

When naming a domain concept in an issue title, refactor proposal,
hypothesis, or test name, use the term defined in `CONTEXT.md`.
Avoid synonyms the glossary explicitly excludes.

If a needed concept is absent, reconsider whether it belongs to the
project's language; note genuine gaps for `domain-modeling`.

## Flag ADR conflicts

If a proposal contradicts an existing ADR, identify the ADR and explain
why reopening the decision is warranted instead of silently overriding it.
