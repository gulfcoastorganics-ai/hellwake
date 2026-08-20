# HUD

The design chat explicitly **froze** this layout before the Unreal port
began: "FREEZE the current: HUD layout ... typography hierarchy ...
Do NOT continue cosmetically redesigning this screen." Treat the
`.dc.html`'s HTML/CSS as the pixel-accurate visual spec — recreate it in
UMG, don't reinterpret it.

## Field-by-field mapping

`UHellwakeHUDWidget` (`Source/Hellwake/HUD/HellwakeHUDWidget.h`) exposes one
`BlueprintImplementableEvent` per HUD region, matching the `.dc.html`
template's `{{ }}` interpolations and `renderVals()` in the prototype:

| HUD element | Prototype field(s) | Widget event | Source |
|---|---|---|---|
| Player name/level, top-left | `KAERVOSS`, `LV {{level}}` | `OnLevelChanged` | static text + `AttributeSet` |
| HP bar | `hp` (0-100%) | `OnHealthChanged(Health, MaxHealth)` | `UHellwakeVitalityComponent`, wired |
| Wrath bar | `wrath` (0-100%) | `OnWrathChanged` | wired |
| Status pips | `statuses[{short,color}]` | `OnStatusPipsChanged` | **not wired** — see below |
| Ability bar cooldown wipes | `cds.{LMB,Q,W,E,R}` | `OnAbilityCooldownsChanged` | **not wired** — see below |
| Boss panel visibility/name/HP | `opBoss`, `bossHp` | `OnBossVisibilityChanged`, `OnBossHealthChanged` | **not wired** — see below |
| Boss affixes line | `bossAffixes` | `OnBossAffixesChanged` | **not wired** |
| Objective text + hostile count | `objective.{text,alive,index,total}` | `OnObjectiveChanged` | wired (`UHellwakeEncounterSubsystem`) |
| Minimap blips | `blips[]`, `lootBlips[]` | `OnMinimapBlipsChanged` | **not wired** — see below |
| Center banner | `banner`, `opBanner` | `OnBannerTextChanged` | wired |
| Cinematic letterbox | `opCine` | `OnCinematicChanged` | wired (fires on Intro stage; should also fire on Wake of Hell — see gap below) |
| Bottom-right stage label | stageNames map | `OnStageLabelChanged` | wired |
| Loot pickup toast | `loot.{label,sub,rarity}` | `OnLootToast` | **stubbed, not functional** — see loot.md |

## What's wired vs. what needs level actors to exist first

`UHellwakeHUDWidget::BindToPlayerController` wires everything with a clean
delegate source today (health/wrath attribute changes, encounter stage/
banner, the loot pickup event — though its payload is incomplete, see
loot.md). The rest — ability cooldowns, boss HP/phase, minimap — don't yet
have a "one authoritative source" the way the prototype's single `onHud()`
payload did, because those need real spawned actors:

- **Ability cooldowns**: read `ASC->GetActiveGameplayEffects()` filtered by
  each ability's `CooldownTags`; percent = remaining/duration. Poll once
  per tick in a `NativeTick` override (cheap for 5 fixed slots).
- **Boss HP/phase**: `UHellwakeEncounterSubsystem::RegisterGravewarden`
  already stores the boss pointer — add a getter and bind to its
  `VitalityComponent`/`GetPhase()` once it's registered (on the `Intro`
  stage transition).
- **Minimap**: `UGameplayStatics::GetAllActorsOfClass` for
  `AHellwakeEnemyBase`/`AHellwakeLootPickup` within minimap range each
  tick, convert to player-relative normalized offsets exactly like the
  prototype's `(50 + b.x*50)%` conversion (this widget uses raw -1..1
  instead of a 0..100% CSS value — convert in the UMG Designer).
- **Cinematic letterbox on Wake of Hell**: currently only fires on the
  `Intro` stage; `AHellwakeAbility_WakeOfHell` grants `State.Cinematic` via
  GameplayEffect but doesn't currently notify the HUD widget directly —
  bind `OnCinematicChanged` to the player ASC's tag-change delegate for
  `State.Cinematic` instead of only the encounter stage, so both sources
  drive the same letterbox.

## Visual spec (from the frozen `.dc.html`)

- Fonts: Cinzel (headers/names, weight 600-900) + Barlow Condensed (body/
  labels, weight 400-600) — both Google Fonts, need local font assets
  imported for UMG (no runtime web-font loading in a packaged build).
- Diegetic panel shapes: hexagonal clip-paths for portraits/ability icons,
  not rectangles — recreate as UMG `Image` with a matching alpha-masked
  texture or a custom `SlateVectorArt`/material.
- World occupies ~80-90% of visual attention; HUD is restrained,
  semi-transparent, small type — this was the single biggest revision in
  the design chat ("STOP MAKING THE UI THE MAIN IMAGE"). Do not scale HUD
  elements up "for readability" without re-reading that chat section.
- Safe zones (from the chat's viewport-fit pass): top ~12% height, bottom
  ~15% height, left/right ~5% width reserved for HUD; keep gameplay-critical
  elements out of the extreme edges.

## Build in Editor

- Import Cinzel + Barlow Condensed as UMG font assets.
- `WBP_HellwakeHUD` — Blueprint child of `UHellwakeHUDWidget`, build the
  widget tree matching the `.dc.html` layout region-by-region (see the
  mapping table above for which native event feeds which region), assign
  to `AHellwakePlayerController::HUDWidgetClass`.
- Diegetic hexagonal panel textures/materials for portrait, ability icons,
  minimap frame, boss panel.
