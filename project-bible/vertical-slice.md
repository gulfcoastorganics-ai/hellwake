# Vertical Slice — Build Plan

Ties gameplay-loop.md, environment.md, enemies.md, and gravewarden.md into
a concrete "what to build, in what order" plan for the first Editor
session.

## Definition of done

Matches the design chat's own acceptance framing for the prototype, now as
Unreal deliverables:

1. Player can move, dodge, light/heavy attack, and cast all 4 abilities
   with correct cost/cooldown/damage.
2. All 3 enemy archetypes spawn, path/ring-hold correctly, attack, die,
   and (probabilistically) drop loot.
3. The Gravewarden fight runs all 3 phases with adds, aura, and all 3
   telegraphed attacks, and drops the guaranteed Legendary on death.
4. The 8-stage encounter advances automatically start to finish with
   correct objective text/banners.
5. HUD reflects live game state for every field listed in hud.md's mapping
   table (accepting the "not wired yet" gaps documented there as known
   follow-up work, not slice blockers).
6. The whole thing runs inside one level, one camera setup, matching the
   frozen visual composition.

## Build order (first Editor session)

1. **Generate project files, compile** — see the verification checklist.
   Get a clean compile before touching any content; every class here
   depends on the others compiling correctly (GAS tags, DataTable row
   structs, etc.).
2. **Data tables**: import all 4 CSVs in `Content/Data/` as DataTables with
   their matching row struct (`FHellwakeEnemyDefinition`,
   `FHellwakeLootDefinition`, `FHellwakeEncounterStageDefinition`,
   `FHellwakeAbilityDefinition`). Fix any CSV parsing errors (FVector/
   FLinearColor CSV syntax is finicky — verify each row imports cleanly).
3. **GameplayEffects**: create the Blueprint children listed in
   combat.md/abilities.md (7 cooldown GEs, the status/buff GEs). This
   unblocks ability testing.
4. **Player**: `BP_HellwakeCharacter` + `BP_HellwakePlayerController`,
   Enhanced Input assets (abilities.md), all 7 ability Blueprint children,
   a placeholder capsule/cube mesh is fine initially — verify movement,
   attacks, dodge, and all 4 abilities work with `stat` commands / on-
   screen debug before investing in real meshes.
5. **Minimal level**: a flat plane + `NavMeshBoundsVolume` + the fixed
   spawn point, `AHellwakeGameMode` set as the level's GameMode override.
   Verify the character spawns, camera frames correctly (camera.md).
6. **Encounter subsystem wiring**: `DT_EncounterStages` assigned on
   `AHellwakeGameMode`/wherever `UHellwakeEncounterSubsystem` reads it,
   `DT_EnemyDefinitions` assigned on `AHellwakeGameMode`. Verify the Enter
   → Wave1 transition fires by walking forward.
7. **Enemies**: the 3 archetype Blueprints (enemies.md), placeholder
   meshes, verify Wave1/Wave2 spawn/fight/die/drop correctly end-to-end.
8. **Gravewarden**: `BP_Gravewarden` (gravewarden.md), verify Intro → Boss
   → Reward → Done with all 3 phases reachable (may need temporarily
   lowered HP for fast iteration — do this via a debug console command or
   PIE-only override, not by editing the DataTable's shipped value).
9. **HUD**: `WBP_HellwakeHUD` (hud.md), wire the fields marked "wired" in
   its mapping table first, then the polling-based fields.
10. **Full block-out environment** (environment.md), then real lighting/VFX
    (lighting-vfx.md) as a final pass — matches the design chat's own
    process (mechanics before final polish).
11. Run the full verification checklist (see project-bible root — separate
    file) before calling the slice done.

## Spawn table reference

See `Content/Data/EncounterStages.csv`'s `SpawnRowNames`/`SpawnLocations`
columns for the exact enemy composition and coordinates per wave — already
authored to match the prototype's `setStage()` spawn calls.
