# The Gravewarden of the Ninth Seal

Elite boss, `AHellwakeGravewarden` (extends `AHellwakeEnemyBase`). 3200 HP,
3 HP-gated phases. Direct port of `wardenAI()` — see
docs/hellwake-behavioral-spec.md §3 for the exact source-of-truth numbers
this implements.

## Phases

| Phase | Threshold | Adds spawned on entry | Soulrend Aura |
|---|---|---|---|
| 1 | HP > 66% | — | off |
| 2 | 33% < HP ≤ 66% | 3× Ashbound Reaver | on |
| 3 | HP ≤ 33% | 2× Cinder Wraith | on, faster attacks |

Phase transitions are checked every tick in `UpdateAI` and fire
`EnterPhase()` exactly once per threshold crossing (not per-tick spam).
Phase 3 scales movement speed ×1.35 and shortens every attack's cooldown.

**Soulrend Aura** (phase 2+): ported as a probabilistic per-tick roll
(`FMath::FRand() < DeltaSeconds * 1.6`) dealing 6 damage within 9m of the
boss, matching the prototype's own probabilistic implementation exactly.
`gravewarden.md`'s header comment in the `.h` file flags this as a
candidate to replace with a real periodic `GameplayEffect` in production —
the random-roll approach was kept here specifically to stay a verifiable
1:1 port; don't "upgrade" it without re-verifying the resulting DPS matches.

## Attacks (weighted random, re-rolled each time the attack cooldown expires)

| Attack | Weight | Telegraph | Damage | Radius | Cooldown (P1/P2 → P3) |
|---|---|---|---|---|---|
| Sweeping Axe | 45% | 0.8s, fixed point 8m ahead of facing | 24 | 6.5m | 2.3s → 1.5s |
| Ground Slam | 30% | 1.05s, target's position *at cast time* (not retargeted) | 32 | 5.5m | 2.8s → 1.8s |
| Area Denial | 25% | 3 (5 in P3) staggered cinder pillars, 5-19m out, ignite over ~1.3-2.1s | 18 each | 3.2m each | 3.4s flat |

Sweeping Axe only rolls if the target is within 8m; otherwise the boss
re-rolls implicitly next cooldown (matches prototype: `dist < 8 && pick <
0.45`). All three use `GetWorld()->GetTimerManager().SetTimer(...)` to
resolve damage at the telegraph's end — the boss can't act
(`AttackCastTimeRemaining`) or move while a telegraph is resolving.

## Death

`HandleDeath()` override spawns the guaranteed Legendary
(`LegendaryLootPickupClass`, "Ashfall, the Last Vow") 3m in front of the
boss's death position — the only guaranteed drop in the slice; every other
enemy uses the probabilistic `UHellwakeLootDropComponent` roll instead.
`UHellwakeEncounterSubsystem` polls `TrackedGravewarden->IsDead()` to
advance Boss → Reward.

## What's not ported yet (see TODOs in the .cpp)

- Banner text / camera shake / VFX on phase transition and death — the
  logic branch exists (`EnterPhase`, `HandleDeath`), the presentation call
  is a `TODO(Presentation)`/`TODO(VFX)` comment at the exact point.
- Attack telegraphs have no visual representation yet — see lighting-vfx.md
  for the Niagara telegraph-ring spec these should use (matches the
  Ruinfall/enemy telegraph pattern, not boss-specific).

## Build in Editor

- `BP_Gravewarden` — Blueprint child of `AHellwakeGravewarden`: assign
  `ReaverClass`/`WraithClass` (→ `BP_AshboundReaver`/`BP_CinderWraith`),
  `DamageEffectClass`, `LegendaryLootPickupClass` (→ `BP_LootPickup_Legendary`,
  see loot.md), a large skeletal mesh matching the "elite, materially
  detailed, dominates without just being bigger text and a bigger health
  bar" brief from the original design chat.
- `DT_EnemyDefinitions`'s `gravewarden` row → `PawnClass = BP_Gravewarden`.
- 3 telegraph Niagara systems (ring/cone/scattered-pillar variants — see
  lighting-vfx.md), 1 phase-transition VFX, 1 death VFX.
