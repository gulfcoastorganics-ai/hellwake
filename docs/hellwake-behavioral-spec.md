# Hellwake — Playable Slice Behavioral Spec

Source: play-testing `design-reference/project/Hellwake Playable Slice.dc.html` +
`hellwake-game.js` (a working Three.js implementation, not a static mockup) in a
patched local harness — Playwright driving a real Chromium against the actual
game code, plus a full read of the source. This is the behavioral reference for
the Unreal Engine 5 port. It records what the prototype **actually does**, not
just what it was asked to do, and flags what should **not** carry over.

Playwright/harness output (console logs, per-step HUD dumps, `probe3`/`probe4`
scripts) is kept in the scratchpad and not part of this repo; this document is
the distilled result.

## 1. Confirmed working end-to-end (live-tested)

- **Boot/render**: WebGL scene renders correctly — ruined plaza, columns,
  braziers with flickering point lights, ash particle field, bloom
  post-process, HUD overlay driven live by `onHud`.
- **Movement**: WASD moves on fixed world axes (`w`→ -Z, `s`→ +Z, `a`→ -X,
  `d`→ +X); since the camera never rotates, this reads as camera-relative.
  Accelerates in ~0.15s to top speed (9.4 u/s), decelerates the same way when
  released. Facing smoothly turns to face the movement vector (not the
  camera or cursor).
- **Stage/encounter progression**: an explicit 8-stage machine —
  `enter → wave1 → advance → wave2 → intro → boss → reward → done` — advanced
  correctly in testing:
  - `enter → wave1` fires when hero.z < 12 (walk into the plaza).
  - `wave1 → advance` fires when all wave1 spawns are dead (+1s settle).
  - `advance → wave2` fires when hero.z < -4.
  - `intro` spawns the Gravewarden and forces a ~3.4s cinematic (input mostly
    locked, letterboxed).
  - `reward` drops a guaranteed Legendary once the Gravewarden dies.
- **Combat resolution**: light/heavy attacks are instantaneous hit-scans on
  click (no windup/animation gating), checked against every enemy each swing:
  within `reach + enemy.ring*0.4` **and** within a facing-relative angular arc
  (~63° light / ~86° heavy). 27% crit chance, crits ×2.6 damage. Confirmed via
  live damage numbers and HUD hostile-count decrementing.
- **Abilities**: Q (Emberbrand, forward cone burst), F (Ashen Bulwark,
  10s 45%-damage-reduction buff), E (Ruinfall, telegraphed delayed AoE ~5u in
  front of the player), R (Wake of Hell, arena nova centered on self) all
  fired correctly, consumed Wrath, respected cooldowns, and produced the
  matching HUD status pips (`EMBER 9s`, `FORT 10s`) that counted down live.
- **Dodge**: Shift/Space gives a 0.34s i-frame + a velocity burst (26 u/s) in
  the current movement (or facing) direction, 1.1s cooldown.
- **Loot**: on-death rarity roll spawns a glowing beacon in the world;
  walking within 2.2 units auto-picks it up and fires an HUD toast
  (`label / RARITY / PICKED UP`, 3.6s). Confirmed live with a Rare
  "Cinderforged Plate" pickup.
- **HUD data contract** (`onHud` payload — this is the authoritative state
  shape to reproduce in UMG): `hp`, `wrath`, `xp`, `level`, `cds{LMB,Q,W,E,R}`
  (0–100 %, W = Bulwark/F), `statuses[{short,color}]`, `boss{alive,hp,phase}`,
  `objective{text,index,total,alive}`, `stage`, `banner`, `cine`(0/1), `dead`,
  `blips[{x,y,elite,kind}]` (minimap, normalized ±1), `loot[{x,y,rarity}]`.
- **Camera**: fixed elevated 3/4 perspective (~34° FOV), lerps toward the
  hero (blends 28% toward the Gravewarden once the boss fight is active),
  zooms out slightly during cinematics (0.82×) and in during the boss fight
  (1.12×), plus additive shake and brief hit-stop (time scaled to 12%) on
  heavy impacts.
- **Respawn**: on hp ≤ 0, 1.8s delay, then full HP/Wrath restore and a hard
  teleport back to the fixed spawn point (0,0,18). No death penalty, no
  checkpointing — every death resets encounter position to the very start.

## 2. Design-relevant quirks confirmed by play-testing (decide before porting)

These aren't crashes — the code runs exactly as written — but they surfaced
as real friction once actually played, not just read:

1. **No lock-on / no mouse-aim.** Standing still and clicking on enemies does
   nothing unless they happen to be inside your last-faced movement cone. In
   a bot-driven playtest, standing in place and spam-clicking left **3 of 4**
   wave-1 enemies alive after 25s of continuous attacking; adding
   circle-strafe movement (so facing sweeps around) was required to land
   hits reliably. For an isometric ARPG this reads as a real usability gap,
   not a stylistic choice — recommend the Unreal port add soft target-lock
   or a wider auto-facing assist via GAS, rather than reproducing raw
   facing-cone gating.
2. **Ranged enemy (Pyre Acolyte) never closes distance.** It holds a fixed
   13-unit ring around the player indefinitely and only casts telegraphed
   bolts; it has no "advance if player retreats" or "reposition" behavior.
   In testing it was the single enemy that survived ~2 minutes of ambient
   combat because nothing pulled the player into its melee range. Intended
   as a role ("ranged supernatural attacker") but the total passivity should
   be tightened for the State Tree AI — e.g., closes in if unable to find
   line-of-sight, or after N seconds without a valid cast.
3. **No animation-gated hit windows.** Damage applies on the click frame,
   not on a weapon-sweep contact frame. It reads responsive but not
   "weighty" — the original design brief explicitly asked for weight/impact/
   acceleration, which this prototype does not deliver mechanically (only
   visually, via trails/shake/hit-stop). The Unreal GAS implementation
   should drive damage off montage notifies, not input time.
4. **Enemy collision radii are small relative to model scale** (0.9u), so
   enemies visually overlap/clip when clustered around the player.
5. Postprocessing (bloom) failure is silently swallowed (`try { } catch {
   composer = null }`) — acceptable prototype behavior, not something to
   silently reproduce in a shipping title; make the Niagara/PP failure mode
   explicit instead.

## 3. Systems reference (from source, for 1:1 porting)

### Player (Kaervoss)
- HP 100, Wrath(resource) 100 max, regen 4.5 Wrath/s passive + on-hit gain
  (+5 light / +9 heavy landed).
- Light attack: reach 3.4, arc ~63°, cooldown 0.42s, base 120 dmg (±15%).
- Heavy attack: reach 4.4, arc ~86°, cooldown 0.92s, base 260 dmg (±15%).
- Q Emberbrand: cost 18 Wrath, cd 6s, cone burst 7.5u radius from a point 5u
  ahead, ~420–600 dmg (always crit-tier), applies `EMBER` status (cosmetic,
  9s).
- F Ashen Bulwark: cost 12 Wrath, cd 12s, 10s buff reducing incoming damage
  ×0.45.
- E Ruinfall: cost 22 Wrath, cd 9s, telegraphs 0.75s then AoE 6.5u at a point
  5u ahead, ~620–840 dmg.
- R Wake of Hell: cost 0 (ultimate, cd 40s), self-centered nova radius 22u,
  damage falls off linearly with distance (1500 − 30·d), forces a 1.6s
  cinematic lock + heavy shake/hit-stop.
- Dodge: 1.1s cd, 0.34s i-frames, 26 u/s impulse.

### Enemy archetypes
| Role | HP | Speed | Dmg | Range | AtkCD | Ring | Notes |
|---|---|---|---|---|---|---|---|
| Ashbound Reaver | 180 | 4.4 | 11 | 2.9 | 1.7s | 3.2 | melee pressure |
| Cinder Wraith | 110 | 7.2 | 7 | 2.4 | 1.1s | 2.6 | fast flanker, bobs vertically, ring slot orbits over time |
| Pyre Acolyte | 130 | 3.4 | 14 | 17 | 2.6s | 13 | ranged bolt, telegraphed 0.85s, never closes distance |
| Gravewarden (elite) | 3200 | 4.0 | 26 | 5.4 | 2.4s | 5 | 3-phase boss, see below |

All non-boss enemies hold a "ring slot" (angle around the player at
`def.ring` distance) rather than pathing directly at the player — this is
what produces the "surround, don't stack" behavior.

### Gravewarden phases (hp% thresholds: >66% P1, >33% P2, ≤33% P3)
- Phase transition: banner text, 16u shockwave, camera shake; P2 spawns 3
  Reavers, P3 spawns 2 Wraiths; Soulrend Aura (P2+) ticks ~6 dmg/s to any
  player within 9u.
- Attack selection each cooldown window (random, weighted):
  - 45% Sweeping axe — 6.5u telegraph 4u ahead, 0.8s windup, 24 dmg, cd
    2.3s (1.5s in P3).
  - 30% Ground slam under player — 5.5u telegraph at player's position at
    cast time, 1.05s windup, 32 dmg, cd 2.8s (1.8s in P3).
  - 25% Area denial — 3–5 telegraphed 3.2u cinder pillars scattered 5–19u
    out, staggered ignition, 18 dmg each, cd 3.4s.
  - P3 moves 1.35× faster and telegraphs fire more often.

### Loot
Rarities: `common` (Ashen Fragment), `magic` (Warded Sigil), `rare`
(Cinderforged Plate), `legendary` (Ashfall, the Last Vow — guaranteed
Gravewarden reward drop). Non-elite kill drop chance per-enemy (`def.drop`,
0.3–0.45), rarity roll on drop: >0.93 rare, >0.6 magic, else common.

### Objective text / stage labels
`enter`→"Enter the Vaunhold plaza", `wave1`→"Break the first procession",
`advance`→"Advance to the cathedral steps", `wave2`→"Clear the ritual guard",
`intro`→"The Gravewarden wakes", `boss`→"Slay the Gravewarden",
`reward`→"Claim the Last Vow", `done`→"Vertical slice complete".

## 4. Visual/art direction to preserve (from Claude Design chat + freeze notes)

Approved and explicitly **frozen** by the user in the design chat — do not
reinterpret:
- HUD layout, typography hierarchy (Cinzel for names/headers, Barlow
  Condensed for body/labels), diegetic-feeling compact HUD (world occupies
  ~80–90% of visual attention).
- Isometric ~45° elevated 3/4 camera, player slightly below center.
- Dark gothic palette: charcoal / cold stone / desaturated earth / blackened
  metal / muted crimson / ember orange / subtle supernatural blue, with
  bright color reserved for abilities, elites, loot, and critical info.
- Gravewarden as a large, materially-detailed elite silhouette (armor/bone/
  cloth), never just a reskinned reaver.
- Restrained, controlled VFX — impact bursts, embers, dust — not particle
  spam.

## 5. Scope note

This document does **not** attempt to run or build an Unreal Engine project
in this environment — there is no Unreal Editor, compiler toolchain, or GPU
desktop available in this sandboxed container, and Blueprints/Niagara/State
Trees/UMG are binary editor assets that cannot be authored as plain text
files here. See the follow-up conversation for what shape the Unreal-side
deliverable should take given that constraint.
