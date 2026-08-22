# Hellwake — native first-play acceptance run

This checklist is for `fix/ue5-first-build` before any production `.umap`, Blueprint, UMG, Niagara, skeletal mesh, or animation asset is required.

## One-command run

From a Windows machine with Unreal Engine 5.4 installed:

```powershell
cd <hellwake-repo>
PowerShell -ExecutionPolicy Bypass -File .\Scripts\RunHellwakeNative.ps1
```

If UE 5.4 is installed somewhere non-standard:

```powershell
PowerShell -ExecutionPolicy Bypass -File .\Scripts\RunHellwakeNative.ps1 `
  -EngineRoot 'D:\Epic Games\UE_5.4'
```

The script builds `HellwakeEditor` first, then launches `/Engine/Maps/Entry` in `-game` mode. `AHellwakeGameMode` creates the temporary arena at runtime.

## Controls

- WASD — move
- LMB — light attack
- RMB — heavy attack
- Shift or Space — dodge
- Q — Emberbrand
- F — Ashen Bulwark
- E — Ruinfall
- R — Wake of Hell

Enhanced Input assets may replace these mappings later. The native slice also carries legacy mappings so this acceptance run is not blocked on binary input assets.

## Expected flow

1. Kaervoss spawns at the north end of the temporary arena with the fixed isometric camera.
2. Moving south past Y=1200 starts Wave 1.
3. Four enemies spawn with native placeholder geometry and readable attack telegraphs.
4. Killing Wave 1 advances the objective.
5. Moving south past Y=-400 starts Wave 2.
6. Clearing Wave 2 begins the Gravewarden intro.
7. Kaervoss is movement/ability locked during the 3.4-second intro; the Gravewarden is visible but dormant.
8. Boss stage begins, boss camera zoom activates, and the Gravewarden starts attacking.
9. Phase 2 spawns three Reavers and enables the Soulrend danger ring.
10. Phase 3 spawns two Wraiths and increases boss pressure.
11. Boss death destroys any remaining phase adds and drops the guaranteed legendary pickup.
12. Picking up the legendary advances Reward -> Done.

## HUD acceptance

The temporary native HUD must show:

- Kaervoss health, Wrath, and XP
- active state/status tags
- objective, stage label, and wave hostile count
- ability READY/CD state
- Gravewarden health, phase number, and living add count during the boss fight

The native HUD is validation scaffolding. `UHellwakeHUDWidget` / `WBP_HellwakeHUD` remain the production UI path.

## Combat acceptance

- Light/heavy attacks only hit targets inside the current facing arc.
- Successful melee attacks grant Wrath and produce short camera impact feedback.
- Dodge grants the iframe gameplay tag.
- Emberbrand, Bulwark, Ruinfall, and Wake of Hell consume/enforce their native costs/cooldowns.
- Ruinfall displays a 0.75-second ground telegraph before damage resolves.
- Pyre Acolyte attacks snapshot the impact point at cast time, allowing the player to dodge out before the 0.85-second detonation.
- Gravewarden sweep, slam, pillars, aura, phase changes, and death all have temporary native debug reads until Niagara replaces them.

## Automated smoke pass

```powershell
PowerShell -ExecutionPolicy Bypass -File .\Scripts\TestHellwake.ps1
```

This builds `HellwakeEditor` and runs `Hellwake.Smoke` through `UnrealEditor-Cmd -NullRHI`.

## Gate

Do **not** call the branch compile-clean or PIE-verified until both scripts succeed on a real Unreal Engine 5.4 Windows installation. After that gate, replace the temporary runtime arena/debug visuals with the authored `L_Hellwake_VerticalSlice`, skeletal characters, UMG/CommonUI HUD, Niagara telegraphs/VFX, materials, lighting, and animation montages without changing the verified combat/state-machine behavior unless deliberately retuned.
