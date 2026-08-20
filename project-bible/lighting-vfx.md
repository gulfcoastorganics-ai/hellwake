# Lighting & VFX

## Lighting rig (ported from `hellwake-game.js`'s Three.js lights)

| Prototype light | UE equivalent | Notes |
|---|---|---|
| `HemisphereLight(0x4a5b73, 0x0a0b0f, 0.5)` | Sky Light, cold blue-gray sky / near-black ground colors | ambient fill |
| `DirectionalLight(0x9fbbe0, 1.15)` at `(-26,40,-18)`, shadows on | Directional Light, cool moonlight-blue tint, cast shadows, 2048 shadow map | primary key light — "moonlight through ruined architecture" |
| `PointLight(0xff7a28, dynamic 1.1-7.1)` following the hero | attach a warm point light to `AHellwakeCharacter`, intensity pulses on swing (`P.swing > 0 ? 6 : 0` boost) | close-range fill, ties the hero into the scene per the "characters must react to environment lighting" brief |
| 5× brazier `PointLight(0xff6f22, ~5.2, 34, decay 2)` with flicker | Point Lights at each brazier location (see environment.md), flicker via a sine-noise material/light-intensity curve (`0.78 + sin(t*7)*0.12 + sin(t*19)*0.08`) | the scene's warm key sources |
| `FogExp2(0x0a0e14, 0.028)` | Exponential Height Fog, matched density/color | atmospheric depth |
| Bloom (UnrealBloomPass, strength 0.62) | Post Process Volume bloom, moderate intensity | ember/emissive accents read as "glowing," not blown out |
| ACES Filmic tonemapping, exposure 1.05 | Post Process tonemapper (UE default is ACES-based already) | — |

Color target (from the design chat's frozen palette): charcoal, cold stone,
desaturated earth, blackened metal, muted crimson, ember orange, subtle
supernatural blue. Reserve bright/saturated color for abilities, elites,
loot, and critical HUD info only — never the environment at large.

## Ambient particles

900-particle ash field (`hellwake-game.js`'s `ash` Points system), each
drifting upward (0.6-2.5 u/s) with slight horizontal sine drift, recycled
around the player position when they exceed 26m height. Port as a Niagara
GPU sprite emitter, world-locked to a large volume around the player, low
opacity, additive-blended, warm-tinted (`0xffa257`).

## Niagara systems needed (referenced from ability/enemy TODOs)

Every one of these has a `TODO(VFX):` comment in the C++ citing the exact
prototype call it replaces — search `Source/Hellwake` for `TODO(VFX)` to
find every call site.

| System | Used by | Prototype reference |
|---|---|---|
| `NS_Dodge_Dust` | `UHellwakeAbility_Dodge` | `burst(pos, 0.5, 10)`, cool gray |
| `NS_Emberbrand_Burst` | `UHellwakeAbility_Emberbrand` | `shockwave` + `burst(1.5, 30)`, ember orange |
| `NS_Bulwark_Raise` | `UHellwakeAbility_Bulwark` | `shockwave(4.5)`, cool blue |
| `NS_Ruinfall_Telegraph` | `UHellwakeAbility_Ruinfall` | ring decal filling in over 0.75s |
| `NS_Ruinfall_Detonate` | `UHellwakeAbility_Ruinfall` | `burst(2, 34)` + `shockwave(6.5)`, violet |
| `NS_WakeOfHell` | `UHellwakeAbility_WakeOfHell` | `shockwave(22)` + `burst(3, 70)`, largest in the kit |
| `NS_Boss_SweepTelegraph` / `NS_Boss_SlamTelegraph` / `NS_Boss_PillarTelegraph` | `AHellwakeGravewarden` | ring/circle/scattered-point telegraphs, see gravewarden.md |
| `NS_Boss_PhaseTransition` | `AHellwakeGravewarden::EnterPhase` | large shockwave, screen-filling but brief |
| `NS_Enemy_Death` / `NS_Boss_Death` | `AHellwakeEnemyBase::HandleDeath` | burst + shockwave, elite version much larger (scale 3 vs 1.3, count 60 vs 26 in the prototype) |
| `NS_Loot_Beacon` | `AHellwakeLootPickup` | beam + ground pool, colored per rarity |
| `NS_Ambient_Ash` | level-persistent | see "Ambient particles" above |
| Melee hit sparks | `UHellwakeAbility_MeleeAttack` (not yet flagged with a TODO — add one) | prototype doesn't spawn a distinct spark burst per light/heavy swing beyond the shared `burst()` helper; reuse `NS_Emberbrand_Burst`-style small burst at hit location |

Design constraint from the chat (repeated across multiple revision passes):
**controlled effects, not particle spam.** Every burst in the prototype is
small (10-70 particles), short-lived (0.22-0.75s), and paired with a light
flash that fades fast. Match that budget — this was explicitly called out
as what separates "AAA" from "generic AI game" in the design brief.

## Damage numbers

Prototype spawns floating DOM text per hit (26px player-damage red,
40px crit orange, 24px normal white), drifting upward with random
horizontal jitter, fading over 1s. Port as a `UWidgetComponent` spawned
per hit (or a pooled Niagara text-sprite system if performance matters at
scale) — not yet stubbed anywhere in C++; add at the same call sites as
`Event.Damage.Taken` is fired.

## Build in Editor

All of the above — this is the largest pure-Editor-authoring surface in
the project. None of it can be created as text files; every row in the
Niagara table needs its own system asset.
