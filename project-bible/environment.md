# Environment

The Vaunhold plaza — a ruined gothic courtyard leading up to a cathedral
facade, per the original design brief's "ruined gothic-fantasy region"
direction. Layout below is transcribed directly from the prototype's
procedural arena-building code (`hellwake-game.js`, arena section) since no
level exists yet to place these as real UE actors/BSP/static meshes.

## Dimensions & bounds

- Playable floor: 96×96m plane, arena half-extent (soft player bound)
  `ARENA = 30m` on X, hard Z clamp `[-27, 27]`.
- Hero spawn: `(0, 0, 18)` → UE `(0, 1800, 0)` (Y = prototype Z, ×100 for cm).
- Gravewarden spawn: `(0, -22)` → `(0, -2200, 0)`.

## Geometry (convert each to a static mesh / BSP placeholder)

| Element | Placement | Notes |
|---|---|---|
| Columns (7 per side) | `x = ∓22∓1.5m, z = -18..18` step 6m | heights 9-11m, alternating broken (truncated + rubble base) |
| Architrave fragments | atop columns, gaps at i=2 (left) / i=4 (right) | tells a "partially collapsed" story per env-storytelling brief |
| Cathedral wall (upstage) | `(0, 13, -34)`, 46×26×3m | anchors the north end |
| Gothic window recesses | 5, spaced 8.5m apart on the cathedral wall | dark recessed boxes + arched tops |
| Spires (3) | `x = -16,0,16`, `z=-36`, heights 16-26m (center tallest) | silhouette above the cathedral wall |
| Downstage ruin wall | `(0, 2.5, 30)`, 70×5×4m | frames the camera's near edge, plus two angled 16×9×4m wall sections flanking it |
| Cathedral steps | 4 steps, `z = -24..-15.4`, narrowing 30→24m wide | the Gravewarden's "ground" — matches gravewarden.md's spawn point |
| Braziers (5) | `(-13,6) (13,6) (-9,-16) (9,-16) (0,18)` | warm key lights — see lighting-vfx.md |
| Rubble clusters (6) | scattered, 7-8 pieces each, 3-4m spread | env-storytelling debris |
| Graves (6) | scattered near the entrance/flanks | headstone + rounded top, slight random tilt |
| Bone scatter (4 clusters) | mid-arena | 5 capsule "bones" each, random rotation |
| Fallen banners (2) | `(-21,18)`, `(21,12)`, tilted | dark red cloth panels |

## Colliders

The prototype uses simple circle colliders (`{x, z, r}`) pushed out of by
both the player and enemies — columns (r=1.6m), graves (r=1.1m), braziers
(r≈1m), downstage wall (r=6m), cathedral wall (r=8m). Port these 1:1 as
either simple capsule/box collision on the static meshes themselves (once
real meshes exist) or a matching set of invisible blocking volumes if
meshes are visual-only.

## NavMesh

No NavMesh exists in the prototype (it's not a real 3D pathing engine —
enemies move via direct vector math). For the Unreal port:
1. Add a `NavMeshBoundsVolume` covering the full 96×96m floor + a margin.
2. Once the geometry above exists as real collidable meshes, rebuild
   navigation and verify `AHellwakeEnemyBase`'s ring-hold movement doesn't
   need NavMesh (it doesn't path — it's a straight vector chase with
   circle-collider avoidance, ported as `AddMovementInput` + Character
   Movement's own collision response). NavMesh only matters if/when the
   State Tree migration (enemies.md) adds a `MoveTo` task using real
   pathfinding around the columns/rubble.

## Character presentation (from the design chat's brief)

- **Kaervoss (hero)**: hooded cloak, leather + dark metal chest piece,
  glowing ember-orange right forearm/weapon-glyph, rim-lit silhouette.
- **Ashbound Reaver**: boxy dark-metal armor, horned helm, ember chest
  core, oversized cleaver — melee-obviously silhouette.
- **Cinder Wraith**: translucent cone-bodied specter, bone head, soul-blue
  glowing eyes/claws — floats, doesn't walk.
- **Pyre Acolyte**: tall conical robe + hood, bone face, staff with a
  floating ember orb — ranged-obviously silhouette, reads as a caster at a
  glance.
- **Gravewarden**: ~1.9× scale of a standard enemy, heavy plated torso +
  cloth skirt, ember core, horned helm, greataxe with an ember-glyph torus
  — must "visually dominate ordinary enemies without simply becoming
  larger text and a larger health bar" (direct design-chat quote).

## Build in Editor

- Block out the arena geometry table above as UE static meshes (ruins/
  columns/cathedral kit — either a purchased dark-fantasy environment kit
  or custom meshes matching the silhouette descriptions).
- `NavMeshBoundsVolume` per the NavMesh section above.
- Skeletal meshes for all 5 character silhouettes described above, rigged
  for the AnimBPs referenced in enemies.md/gravewarden.md/combat.md.
- `L_Hellwake_VerticalSlice` level: place the arena, set
  `GameDefaultMap`/`EditorStartupMap` in `Config/DefaultEngine.ini` once
  built.
