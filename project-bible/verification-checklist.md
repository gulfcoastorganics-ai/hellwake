# Verification Checklist

For the first session on a workstation with Unreal Engine 5 actually
installed. Nothing in this repository has been compiled, opened in the
Editor, or run — every item below is a real unknown, not a formality.
Work top to bottom; later items assume earlier ones pass.

## 1. Project generation
- [ ] `Hellwake.uproject`'s `EngineAssociation` matches the installed
      engine version (currently set to `5.4` — update if using a newer/older 5.x).
- [ ] Right-click `Hellwake.uproject` → "Generate Visual Studio project
      files" (Windows) succeeds, or `RunUAT`/`UnrealBuildTool -projectfiles`
      succeeds (Linux/Mac).
- [ ] No missing-plugin errors on load (`GameplayAbilities`, `EnhancedInput`,
      `StateTree`, `GameplayStateTree`, `Niagara`, `CommonUI` — all
      declared in `Hellwake.uproject`).

## 2. C++ compilation
- [ ] `Hellwake` (Game target) compiles clean, zero errors.
- [ ] `HellwakeEditor` (Editor target) compiles clean, zero errors.
- [ ] Check warnings for the GAS `Modifiers`/`Period` API usage in
      `GameplayEffects/*.cpp` — these are the least certain API surface
      (see unreal-implementation.md's "written but not compiled" caveat);
      if any are deprecated/renamed in the installed engine version, fix
      the call site, don't suppress the warning.
- [ ] `HellwakeGameplayTags.cpp`'s `UE_DEFINE_GAMEPLAYTAG` calls resolve
      correctly (native tag registration) — check Project Settings >
      GameplayTags shows all tags listed in `Config/Tags/GameplayTags.ini`.

## 3. Editor startup
- [ ] Editor opens without a missing-asset/redirector error storm (expected:
      it will complain about the unset `GameDefaultMap`/`EditorStartupMap`
      in `DefaultEngine.ini` until vertical-slice.md's level exists — that's
      expected, not a bug).
- [ ] `DT_EnemyDefinitions`, `DT_LootDefinitions`, `DT_EncounterStages`,
      `DT_AbilityDefinitions` import cleanly from their `Content/Data/*.csv`
      sources with the correct row struct assigned. Watch for FVector/
      FLinearColor CSV parse errors specifically (`EncounterStages.csv`'s
      `SpawnLocations` column, `LootDefinitions.csv`'s `Color` column).

## 4. Enhanced Input
- [ ] `IMC_Hellwake_Default` + all 8 `UInputAction` assets created and
      assigned on `BP_HellwakeCharacter` (see abilities.md).
- [ ] WASD produces movement; character rotates to face movement direction
      (not camera-locked, not mouse-locked — see combat.md).

## 5. GAS initialization
- [ ] `AHellwakeCharacter::PossessedBy` runs without error; `Health`/`Wrath`
      show correct starting values (100/100 each) via a debug widget or
      `showdebug abilitysystem`.
- [ ] Passive regen visibly ticks (Wrath climbs at ~4.5/s, Health at
      ~1.6/s below max) — confirms `UHellwakeGE_PassiveRegen`'s
      AttributeBased periodic modifier is wired correctly (this is the
      least-certain GE in the project; if it doesn't tick, check the
      `FAttributeBasedFloat`/`FGameplayEffectAttributeCaptureDefinition`
      construction in `HellwakeGE_PassiveRegen.cpp` first).

## 6. Player spawn
- [ ] Character spawns at `(0, 1800, 0)` (SpawnPoint), capsule not
      clipping into the floor.
- [ ] `Die()`/respawn cycle works: reduce Health to 0 via a debug command,
      confirm 1.8s delay then full restore + teleport back to spawn.

## 7. Camera
- [ ] Fixed isometric framing matches the approved composition (compare
      against `design-reference/project/uploads/Screenshot*.png` for a
      visual reference of the frozen composition, or the behavioral-spec
      screenshots in `docs/`).
- [ ] Zoom lerps correctly on entering/leaving a boss fight and during
      cinematics (`UHellwakeCameraDirector`).
- [ ] `TriggerShake`/`TriggerHitStop` visibly work when called from a debug
      Blueprint node (global time dilation dip + camera jitter).

## 8. Attacks
- [ ] Light/Heavy attack costs the correct cooldown, deals damage only to
      targets within the facing cone (see combat.md — confirm this
      matches the *documented* behavior, i.e. verify it whiffs when
      standing still and clicking on an off-facing target — that's
      correct, not a bug).
- [ ] Crit rate roughly matches 27% over enough swings; crit damage is
      visibly ~2.6× base.
- [ ] Knockback visibly pushes hit enemies back.

## 9. Dodge
- [ ] 1.1s cooldown enforced; 0.34s i-frame window makes the player
      briefly immune (test by dodging into an active enemy attack).
- [ ] Dodge direction matches current movement input, falls back to
      facing when stationary.

## 10. Abilities
- [ ] All 4 (Q/F/E/R) activate, cost the correct Wrath, respect cooldown,
      and are blocked while `State.Dead`/`State.Cinematic` is present
      (test by triggering Wake of Hell, confirm no other ability can fire
      during its 1.6s cinematic lock).
- [ ] Emberbrand/Ruinfall/Wake of Hell deal damage to enemies within their
      documented radius (abilities.md); Ruinfall's 0.75s telegraph delay
      is present and the hit resolves at the *captured* point even if the
      player moves during the delay.
- [ ] Bulwark visibly reduces incoming damage by ~55% for its 10s duration.

## 11. Enemy AI
- [ ] All 3 archetypes spawn on Wave1/Wave2, hold their engagement ring
      around the player rather than stacking on top of each other.
- [ ] Pyre Acolyte confirmed to NOT close distance on its own (documented
      behavior, not a bug — see enemies.md) — if this needs fixing for
      production, that's a deliberate follow-up decision, not something
      this checklist item should silently "pass" by working around.
- [ ] Melee enemies land hits after their ~0.3s swing delay; Acolyte
      telegraphs before its ranged bolt lands.
- [ ] Enemy death → sink/tumble → despawn (0.9s) works; loot roll fires
      correctly per enemies.md/loot.md's drop-chance table.

## 12. Gravewarden phases
- [ ] Phase 1 → 2 transition at 66% HP spawns 3 Reavers, enables Soulrend
      Aura (visible periodic damage within 9m).
- [ ] Phase 2 → 3 transition at 33% HP spawns 2 Wraiths, speeds up attacks.
- [ ] All 3 attack types (Sweeping Axe / Ground Slam / Area Denial) are
      reachable and telegraph correctly before dealing damage.
- [ ] Death spawns the guaranteed Legendary drop 3m in front of the death
      position.

## 13. HUD
- [ ] Health/Wrath bars update live and match `showdebug abilitysystem`
      values.
- [ ] Objective text/banner update correctly on every stage transition,
      matching `Content/Data/EncounterStages.csv`'s text columns.
- [ ] Confirm which fields from hud.md's mapping table are still
      unwired at this point (cooldowns/boss HP/minimap) and either wire
      them per that doc's plan or explicitly scope them out — don't leave
      them silently broken without a decision.

## 14. Loot
- [ ] Pickup beacon spawns at the correct location, bobs/rotates, and
      auto-picks-up within 220cm with no button press.
- [ ] `Event.Loot.PickedUp` fires (confirm via log, `UE_LOG` already present
      in `HellwakeLootPickup.cpp`) even though the HUD toast payload is a
      known gap (loot.md).

## 15. Niagara
- [ ] Every system listed in lighting-vfx.md's table exists and is
      assigned at its documented call site (search `TODO(VFX)` in
      `Source/Hellwake` — each should be replaced with a real
      `UNiagaraFunctionLibrary::SpawnSystemAtLocation` call once the
      system asset exists).
- [ ] Particle counts/lifetimes match the "controlled effects, not spam"
      budget noted in lighting-vfx.md — spot-check against the prototype's
      counts (10-70 particles, 0.22-0.75s life) if in doubt.

## 16. Packaged build
- [ ] Package for the target platform (Development configuration first).
- [ ] Cook completes without missing-asset errors.
- [ ] Packaged build launches, full slice (Enter → Done) playable start to
      finish without a crash.
- [ ] Frame rate holds steady during the Gravewarden fight with adds +
      Niagara active (the heaviest concurrent-VFX moment in the slice).
