# Current Milestone — Resource-Efficient Agent Execution

## Objective

Turn HELLWAKE into a repository that frontier coding agents can advance end-to-end with strong evidence while using small/free compute for everything that does not genuinely require GPU rendering or Windows packaging.

## Immediate priorities

1. Keep the UE5.4 source tree buildable and auditable.
2. Reconcile any later migration/canonical artifacts when they are actually present in Git.
3. Push deterministic gameplay/data checks into CPU-only tests.
4. Add headless Unreal Automation / commandlet validation when a UE5.4 Linux environment becomes available.
5. Use interactive/GPU sessions only after cheaper gates are green.

## Acceptance criteria

- Repository custom agents are available and have clear responsibility boundaries.
- `scripts/agent/validate_all.sh` is the default validation entrypoint.
- CI can run repository/reference/data/static checks on a standard Linux runner.
- Missing prerequisites are reported as SKIP/UNAVAILABLE, never falsely as PASS.
- No agent moves a frozen reference tag or silently rebalances gameplay.
- QA and independent review are required before milestone closure.

## Next engineering gate

Build the portable validation layer around the gameplay/data that is actually present in the checkout. If later `migration/canonical/` data is pushed, reconcile it first and make it the authoritative generated-data source before adding more duplicated balance values.

## Deferred gates

These require infrastructure not guaranteed on small/free runners and must not block cheaper work:

- UE Editor visual inspection
- PIE visual parity
- Lumen/Nanite/Niagara quality
- GPU performance
- final Win64 packaging
