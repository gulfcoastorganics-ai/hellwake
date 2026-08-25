# HELLWAKE Execution State

This file is the evidence ledger for agents. Update it only after a command, test, repository read, or reviewed artifact proves the state.

## Repository-visible state when agent orchestration was installed

- Repository: `gulfcoastorganics-ai/hellwake`
- `main` baseline: `4361ad788052e42e9eaf42c322be332361480208`
- UE build branch observed: `fix/ue5-first-build`
- UE build branch head observed at install time: `bc06f6a1abe202aa07aa1faf30bf29850ef36d2d`
- Target engine: Unreal Engine 5.4
- Full UE compile/Editor/PIE was not proven in the connected repository environment at install time.

## Important repository-integrity note

Later Grok-only migration/preflight commits and tags discussed outside GitHub are not automatically repository truth. If refs such as `hellwake-web-vertical-slice-reference` or `migration/canonical/` are absent from this checkout, do not invent them. They must be pushed/imported before agents rely on them.

## Current milestone

See `docs/current-milestone.md`.

## Gates

- [x] Repository agent contract installed
- [x] Copilot repository instructions installed
- [x] Specialist custom-agent profiles installed
- [x] Low-resource validation entrypoints installed
- [ ] Later migration/canonical artifacts reconciled into this GitHub repository if/when supplied
- [ ] Canonical/reference integrity validation passes
- [ ] Portable gameplay/core tests exist and pass
- [ ] UE static/source preflight passes on current checkout
- [ ] UE Linux/project compile proven when an engine is available
- [ ] Headless Unreal Automation / `-nullrhi` proven when supported by the environment
- [ ] Interactive Editor/PIE parity proven
- [ ] GPU visual validation proven
- [ ] Final Win64 package proven

## Active blockers

- No blocker should be inferred solely from lack of GPU/Windows. Complete cheaper validation layers first.
- Unreal-only gates remain unavailable until a compatible UE5.4 installation is present.

## Last validated commands

Populate with exact commands and outcomes as agents run them.

## Last known good commit

`bc06f6a1abe202aa07aa1faf30bf29850ef36d2d` was the observed `fix/ue5-first-build` head before adding the orchestration package.

## Rules for updates

For every gate change include:

- command/test/artifact
- result
- commit SHA
- environment
- remaining validation not performed

Never mark a gate passed from a plan, code review, or expected behavior alone.
