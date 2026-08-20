# Gameplay Loop

A single ~5-10 minute encounter, matching the approved vertical-slice scope
(design-reference chat: "We are building ONE extremely polished AAA-quality
combat slice"). No menus, no persistent progression beyond the optional
`UHellwakeSaveGame` checkpoint marker.

## The 8 stages

Owned by `UHellwakeEncounterSubsystem` (`Source/Hellwake/HellwakeEncounterSubsystem.h`),
data-described by `Content/Data/EncounterStages.csv` → `DT_EncounterStages`.

```
Enter ──(player.Y < 12m)──▶ Wave1 ──(all dead, +1s)──▶ Advance
  ──(player.Y < -4m)──▶ Wave2 ──(all dead, +1s)──▶ Intro
  ──(3.4s elapsed)──▶ Boss ──(Gravewarden dead, +2.2s)──▶ Reward
  ──(legendary loot taken, +1s)──▶ Done
```

Every transition broadcasts `OnStageChanged` (drives HUD objective text +
enemy spawning via `AHellwakeGameMode::HandleStageChanged`) and
`OnBannerRequested` (drives the ~3s full-screen banner text). See
docs/hellwake-behavioral-spec.md §1 for the exact prototype thresholds this
ports.

## Player-facing beats

1. **Enter the Vaunhold plaza** — walk-in, no combat, establishes scale
   and environment (see environment.md).
2. **Encounter 1** — 4 enemies (2 Reaver, 1 Wraith, 1 Acolyte). First
   combat test: teaches attack/dodge/ability rhythm.
3. **Advance** — short traversal beat, no combat, builds anticipation for
   the cathedral steps.
4. **Encounter 2** — 5 enemies (2 Reaver, 2 Wraith, 1 Acolyte), denser
   than Encounter 1.
5. **Elite intro** — Gravewarden spawns, ~3.4s cinematic lock
   (`AHellwakeCharacter::BeginCinematic`), camera zooms per
   `UHellwakeCameraDirector`.
6. **Gravewarden battle** — see gravewarden.md. The centerpiece.
7. **Legendary drop** — guaranteed "Ashfall, the Last Vow" spawns near the
   boss's death position.
8. **Slice complete** — no further systems; a real build would transition
   to a results screen, out of scope here.

## Loss condition

There isn't one, structurally — player death is a soft reset
(`AHellwakeCharacter::Die`): 1.8s delay, full HP/Wrath restore, teleport to
`SpawnPoint`. Encounter stage/progress and already-dead enemies are
untouched, matching the prototype's respawn() exactly. This is a deliberate
low-friction choice for a demo slice, not a placeholder to "fix" — see
docs/hellwake-behavioral-spec.md §1 "Respawn."
