# Issue tracker: Local Markdown

Issues and specs for this repo live as Markdown files in `.scratch/`.

## Conventions

- One feature per directory: `.scratch/<feature-slug>/`.
- Spec: `.scratch/<feature-slug>/spec.md`.
- Implementation issues: one file per ticket at
  `.scratch/<feature-slug>/issues/<NN>-<slug>.md`, numbered from `01`,
  never a single combined tickets file.
- Triage state: a `Status:` line near the top of each issue file.
  Use the role strings in `triage-labels.md`.
- Comments and conversation history: append under `## Comments`.

## When a skill says "publish to the issue tracker"

Write the spec or individual issue files at the paths above, creating
directories as needed.

## When a skill says "fetch the relevant ticket"

Read the referenced file. Resolve a bare issue number within the current
feature's `issues/` directory.

## Wayfinding operations

Used by `/wayfinder`. The map is a file with one child file per ticket.

- Map: `.scratch/<effort>/map.md`, containing Notes, Decisions-so-far,
  and Fog.
- Child ticket: `.scratch/<effort>/issues/<NN>-<slug>.md`, numbered from
  `01`, with the question in the body. A `Type:` line records
  `research`, `prototype`, `grilling`, or `task`.
- Wayfinding lifecycle uses `Status: claimed` and `Status: resolved`,
  rather than the triage role strings.
- Blocking: a `Blocked by: NN, NN` line near the top. A ticket is
  unblocked when every listed ticket is `resolved`.
- Frontier: scan the effort's `issues/` directory for unresolved,
  unblocked, unclaimed tickets; first by number wins.
- Claim: set `Status: claimed` and save before any work.
- Resolve: append the answer under `## Answer`, set `Status: resolved`,
  then append a context pointer (gist + link) to Decisions-so-far in
  `map.md`.
