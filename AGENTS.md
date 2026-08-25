# HELLWAKE Agent Contract

HELLWAKE is an existing Unreal Engine 5.4 dark-fantasy action-RPG project. Preserve working systems and advance the current milestone; do not restart the project or replace it with a generic template.

## Repository truth

- UE baseline on `main`: `4361ad788052e42e9eaf42c322be332361480208`.
- Active UE build work is on `fix/ue5-first-build`; the orchestration package was based on its current head when installed.
- `design-reference/`, `docs/`, and `project-bible/` are repository-visible behavioral/design evidence.
- If `migration/canonical/` and tag `hellwake-web-vertical-slice-reference` are present, canonical migration data and that frozen tag take precedence for migrated gameplay values.
- Never invent a commit, tag, file, asset, build result, or test result that is not present in the checkout.
- External/chat-only Grok work is not repository truth until it has actually been pushed or imported.

## Execution rule

For every task:

1. Inspect the repository, current branch, recent commits, and relevant docs.
2. Reproduce the problem or establish the current state.
3. Identify the smallest correct change.
4. Implement without deleting functioning systems unless evidence requires it.
5. Run the strongest validation available in the current environment.
6. Compare behavior with canonical/reference data where applicable.
7. Update `docs/execution-state.md` only with evidence.
8. Commit logical checkpoints; keep generated Unreal junk out of Git.

## Resource policy

Prefer the cheapest proof that establishes correctness:

1. schema/data validation
2. pure C++/Python/unit tests
3. static UE source checks
4. Unreal commandlets / automation / `-nullrhi` when available
5. interactive Editor/PIE
6. GPU visual validation
7. final Win64 packaging

Do not require a GPU to validate logic that can be tested headlessly. Do not use expensive compute while basic source, data, or automation failures remain.

## Validation claims

Never say any of the following unless that exact operation ran successfully:

- `COMPILED`
- `EDITOR VALIDATED`
- `PIE VALIDATED`
- `NANITE VALIDATED`
- `NIAGARA VALIDATED`
- `PACKAGED`

Static inspection is not compilation. Typechecking is not behavioral parity. A screenshot is not a gameplay regression test.

## Unreal rules

- Target Unreal Engine 5.4 unless the user deliberately changes the target.
- C++ owns core systems where appropriate; Blueprints/UMG/DataAssets configure content and presentation.
- Reuse the existing GAS/Enhanced Input architecture when present; do not build a parallel ability/input stack.
- Avoid `Tick` everywhere, repeated `GetAllActorsOfClass`, hardcoded asset paths, Blueprint spaghetti, and circular dependencies.
- Keep `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, and `.vs/` untracked.

## Canonical gameplay

When canonical migration data exists, never silently rebalance it. If runtime code, legacy CSVs, docs, or Blueprints disagree, record the conflict and resolve it explicitly against the designated source of truth.

## Agent organization

- `hellwake-director`: decomposes milestones and delegates.
- `hellwake-architect`: protects architecture and dependency boundaries.
- `unreal-engineer`: UE5.4 C++, GAS, input, UMG, AI, build/integration.
- `gameplay-engineer`: combat, abilities, enemies, boss, encounter parity.
- `automation-engineer`: CI, headless validation, scripts, deterministic data generation.
- `world-assets`: map/layout/import contracts and asset pipeline.
- `hellwake-qa`: adversarial gate; primarily read/test, not implementation.
- `hellwake-reviewer`: independent final technical review.

The Director should delegate independent work to specialists and require QA/review evidence before closing a milestone.
