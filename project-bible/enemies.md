# Enemies

Three standard archetypes, all `AHellwakeEnemyBase` (`Source/Hellwake/Enemies/`).
Data in `Content/Data/EnemyDefinitions.csv` → `DT_EnemyDefinitions`.

| Role | HP | Speed | Dmg | Range | Cooldown | Ring | Drop | XP |
|---|---|---|---|---|---|---|---|---|
| Ashbound Reaver | 180 | 4.4 m/s | 11 | 2.9m | 1.7s | 3.2m | 35% | 40 |
| Cinder Wraith | 110 | 7.2 m/s | 7 | 2.4m | 1.1s | 2.6m | 30% | 32 |
| Pyre Acolyte | 130 | 3.4 m/s | 14 | 17m | 2.6s | 13m | 45% | 46 |

## AI — current implementation is a Tick fallback, by design

`AHellwakeEnemyBase::UpdateAI`/`TryAttack` is a direct, line-by-line port
of `enemyAI(e, dt)` from the prototype, running in `Tick()` rather than as
a State Tree. This is deliberate, not a shortcut taken because State Tree
is harder: **it's the only version of this behavior that's actually
runnable and testable in this environment** (no Unreal Editor here to
author a State Tree asset). Treat it as the verified reference
implementation, not throwaway code — keep it working until its State Tree
replacement is confirmed to reproduce the same behavior, then remove it.

**Ring-hold positioning**: each enemy holds an angular slot
(`RingSlotRadians`, randomized at spawn) around the player at
`EngagementRingCm` distance — `AddMovementInput` toward that point every
tick it's more than 60cm away. This is what produces "enemies surround
the player instead of stacking on top of them." Cinder Wraiths additionally
orbit their slot over time (`RingSlotRadians + AnimTime * 0.5`).

**Facing**: every enemy faces the target directly and continuously
(`SetActorRotation` each tick) — unlike the player, whose facing is
movement-driven. This is correct as-is; don't change it to match the
player's model.

**Attack**: melee enemies deal damage after a fixed 0.3s delay (swing
timing); the Acolyte fires a telegraphed ranged bolt after 0.85s. Both use
`AHellwakeEnemyBase::DealDamageTo`, which applies the same `UHellwakeGE_Damage`
every other damage source uses.

## Known behavioral quirk — do not "fix" without a decision

**The Pyre Acolyte never closes distance.** Confirmed live: in a 2-minute
bot playtest it was the one enemy that survived because nothing pulled it
into range once its ring-hold distance (13m) was already satisfied at
spawn. This is accurately ported as-is (`EngagementRingCm = 1300` combined
with `AttackRangeCm = 1700` means it's almost always "in range" from its
resting position and has no code path to approach further). If this
should change for production, the fix belongs in a State Tree "approach if
player retreats past N seconds without a valid cast" branch — not a patch
to the ring-hold math, which is correct for every other role.

## State Tree migration path (once Editor available)

Replace `AHellwakeEnemyBase::UpdateAI`/`TryAttack` with State Tree tasks
that call the same `Blueprint`-callable functions already exposed
(`TryAttack`, `DealDamageTo`) so combat resolution logic isn't rewritten —
only the decision layer (when to move vs. attack vs. retreat) moves into
the tree:

- **Sequence**: `MoveTo(ring slot point)` → `Wait(until in range)` →
  `PlayAttack`.
- Add a `Reaver`/`Wraith`/`Acolyte` selector at the tree root branching on
  `RoleTag`, or three separate trees selected by `AHellwakeEnemyAIController`.
- NavMesh: enemies currently move via raw `AddMovementInput`, not NavMesh
  pathing — fine for the open plaza arena, but a State Tree `MoveTo` task
  should use `UNavigationSystemV1`/`AIMoveTo` once real collision geometry
  exists (see environment.md "NavMesh").

## Build in Editor

- `BP_AshboundReaver` / `BP_CinderWraith` / `BP_PyreAcolyte` — Blueprint
  children of `AHellwakeEnemyBase`, each with `EnemyDefinitionTable` +
  `EnemyDefinitionRowName` set to the matching CSV row, `DamageEffectClass`
  → `GE_Hellwake_Damage`, and a mesh/animation blueprint assigned.
- Skeletal meshes + a shared AnimBP per silhouette described in
  environment.md's character-presentation notes (from the original design
  brief: armored melee / fast flanker / ranged caster silhouettes).
- `DT_EnemyDefinitions` imported from `Content/Data/EnemyDefinitions.csv`,
  `PawnClass` column pointed at the three BPs above (+ the Gravewarden row
  at `BP_Gravewarden`, see gravewarden.md).
