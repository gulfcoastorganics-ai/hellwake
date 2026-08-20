# Unreal Implementation Notes

Engineering decisions, module layout, and consolidated known gaps. Written
from a Linux CLI environment with **no Unreal Editor, no UnrealBuildTool,
no compiler toolchain, and no GPU available** — every class here is
hand-written to correct UE5.4 API usage from documentation/experience, but
**none of it has been compiled or run**. Treat first compilation as a real
first pass, not a formality — see the verification checklist.

## Module layout

Single primary module (`Hellwake`, `Source/Hellwake/Hellwake.Build.cs`),
organized by concern rather than by C++/Blueprint split:

```
Abilities/     GAS ability classes (7 concrete + 3 shared bases)
Attributes/    UHellwakeAttributeSet
Camera/        UHellwakeCameraDirector
Combat/        UHellwakeCombatComponent, UHellwakeVitalityComponent
Data/          DataTable row structs
Enemies/       AHellwakeEnemyBase, AHellwakeGravewarden, AI controller
GameplayEffects/  C++ GameplayEffect classes
HUD/           UHellwakeHUDWidget
Loot/          AHellwakeLootPickup, UHellwakeLootDropComponent
Player/        AHellwakeCharacter, AHellwakePlayerController
SaveGame/      UHellwakeSaveGame
HellwakeGameplayTags.h/.cpp   native GameplayTags
HellwakeEncounterSubsystem.h/.cpp
HellwakeGameMode.h/.cpp
```

Dependencies declared in `Hellwake.Build.cs`: Core/CoreUObject/Engine/
InputCore, EnhancedInput, GameplayAbilities/GameplayTags/GameplayTasks,
AIModule, StateTreeModule/GameplayStateTreeModule, NavigationSystem, UMG,
Niagara. `Hellwake.uproject` enables the matching plugins
(GameplayAbilities, EnhancedInput, StateTree/GameplayStateTree, Niagara,
CommonUI).

## Key architecture decisions

- **Every ASC-owning actor shares one `UHellwakeAttributeSet`** (player and
  every enemy, including the Gravewarden) rather than per-role attribute
  sets. Simpler, and the prototype's player/enemy state shapes
  (`P.hp/hpMax` vs `e.hp/hpMax`) were already parallel — this just
  actually shares the implementation. Enemies never use Wrath.
- **Each enemy owns its own ASC** (not a shared pool) — simplest correct
  setup for a slice-scale roster (≤10 concurrent enemies). Revisit if a
  much larger concurrent-enemy count is ever needed.
- **Enemy AI is a Tick-based C++ port, not a State Tree**, by necessity of
  this environment (no Editor to author a State Tree asset) — see
  enemies.md for the explicit migration plan and why the Tick fallback
  should be kept until its replacement is verified equivalent, not deleted
  on sight.
- **Damage always flows through one path**: ability/enemy code →
  `UHellwakeGE_Damage` (SetByCaller) → `UHellwakeAttributeSet::PostGameplayEffectExecute`
  → Health reduction + `Event.Damage.Taken`/`Event.Death`. Don't add a
  second damage path (e.g. directly calling `ApplyModToAttribute` on
  Health) — it would bypass the i-frame/Bulwark/death gating that lives in
  exactly one place on purpose.
- **GameplayTags are native** (`HellwakeGameplayTags.h`, `UE_DECLARE_GAMEPLAYTAG_EXTERN`)
  for anything C++ branches on; `Config/Tags/GameplayTags.ini` mirrors them
  for designer-facing pickers but isn't load-bearing.

## Multiplayer note

Everything here is written with GAS replication in mind (`SetIsReplicated(true)`,
`GetLifetimeReplicatedProps`, `EGameplayEffectReplicationMode::Mixed` on the
player / `Minimal` on enemies) but the prototype is single-player and this
was never tested against an actual netcode pass. If Hellwake ever needs
multiplayer, move the player's ASC from the Character to the PlayerState
(standard GAS practice for anything that needs to survive
possession changes) — currently on the Character for simplicity, matching
"this is a single-player vertical slice" scope.

## Consolidated known gaps

Grep `TODO(` across `Source/Hellwake` for the full list with exact call
sites. By category:

- **`TODO(VFX)`**: every ability/enemy/boss attack that should spawn a
  Niagara system — none are wired to an actual system reference yet (no
  Niagara assets exist). Full list in lighting-vfx.md.
- **`TODO(Camera)`**: shake/hit-stop trigger points not yet calling
  `UHellwakeCameraDirector`.
- **`TODO(Presentation)`**: banner text / stage-change broadcasts not yet
  connected to their trigger point (mostly boss phase/death).
- **Loot pickup toast payload** (hud.md, loot.md): `Event.Loot.PickedUp`
  doesn't yet carry which item was picked up.
- **Camera boss-focus blend** (camera.md): the 28%-toward-boss lerp isn't
  ported; needs an attachment-model decision first.
- **HUD polling fields** (hud.md): ability cooldowns, boss HP/phase,
  minimap blips need level actors to exist before they can be wired.
- **Damage numbers** (lighting-vfx.md): no floating-text system exists yet.

None of these block compilation or basic playability — they're
presentation/polish gaps, consistent with "C++ gameplay layer, not a
finished game" being the explicit scope of this port.

## How to open this project

1. Install Unreal Engine 5.4 (or update `Hellwake.uproject`'s
   `EngineAssociation` to match whatever 5.x version is actually
   installed — nothing here depends on a specific 5.x patch).
2. Right-click `Hellwake.uproject` → "Generate Visual Studio project files"
   (Windows) / run `UnrealBuildTool` directly (Linux/Mac) — see the
   verification checklist for exact commands.
3. Open in Editor; it will prompt to compile the `Hellwake` module.
4. Follow vertical-slice.md's build order from there.
