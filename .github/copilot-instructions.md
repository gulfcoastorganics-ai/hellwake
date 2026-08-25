# GitHub Copilot Instructions — HELLWAKE

HELLWAKE is an Unreal Engine 5.4 dark-fantasy action-RPG. Treat it as an existing production project, not a greenfield demo.

Before changing code, read `AGENTS.md`, `docs/execution-state.md`, `docs/current-milestone.md`, the relevant files under `docs/` and `project-bible/`, then inspect `git status` and recent commits.

Use repository-visible evidence only. The public `main` baseline is `4361ad788052e42e9eaf42c322be332361480208`; active UE build work may be on `fix/ue5-first-build`. If `migration/canonical/` and `hellwake-web-vertical-slice-reference` exist in the checkout, use them as the migrated gameplay source of truth. Never fabricate missing refs from chat history.

Preserve existing GAS, Enhanced Input, combat, AI, encounter, HUD, and save architecture unless a concrete defect requires change. Prefer targeted fixes over rewrites. Do not silently change gameplay balance.

Resource order: data/schema checks → pure unit tests → static UE checks → headless Unreal/commandlets/`-nullrhi` → interactive PIE → GPU visual validation → final Win64 packaging. Do not require expensive compute for logic validation.

Never claim compile, Editor, PIE, Nanite, Niagara, packaging, or runtime validation unless the exact operation ran successfully. Record unavailable validation explicitly.

Keep Unreal generated artifacts out of Git: `Binaries/`, `DerivedDataCache/`, `Intermediate/`, `Saved/`, `.vs/`.

When a task spans multiple domains, use the repository custom agents in `.github/agents/`: Director for orchestration, Architect for design boundaries, Unreal Engineer for UE integration, Gameplay Engineer for parity, Automation Engineer for CI/headless tests, World/Assets for map/import work, QA for adversarial validation, and Reviewer for the final independent gate.

Every implementation task should end with: changed files, validation actually run, validation not available, remaining risks, and the next gate. Update `docs/execution-state.md` only with evidence.
