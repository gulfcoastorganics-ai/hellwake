# Combat

## Targeting — read this before changing anything here

**Discovered behavior, confirmed by play-testing** (docs/hellwake-behavioral-spec.md
§2.1): the prototype has **no lock-on and no mouse-aim**. Melee attacks
resolve as an instantaneous overlap check within a reach + a facing-relative
angular cone; facing is driven purely by the player's last movement
direction. A bot standing still and clicking left 3 of 4 enemies alive
after 25 real seconds of continuous attacking — only circle-strafing (so
facing swept across the enemy ring) reliably landed hits.

**This port reproduces that exact model** — `UHellwakeAbility_MeleeAttack`
does a facing-cone `SphereOverlapActors` check against `ACharacter::GetActorForwardVector()`,
which itself only changes via `bOrientRotationToMovement` (see
`AHellwakeCharacter`'s constructor comment). This is a faithful port of
*confirmed* behavior, not an oversight preserved by accident.

**Whether to keep it is a design decision, not an engineering one.** The
friction is real and was measured, not assumed. Two reasonable directions
for a production pass:
- Add a soft target-lock (hold a button to snap facing toward the nearest
  enemy in a wide forward cone) — common in isometric ARPGs, low-risk.
- Widen the cone further and/or add partial damage falloff outside it,
  keeping the "no hard lock-on" feel but reducing whiffs.

Do not silently "fix" this by adding lock-on without flagging the change —
it alters combat feel non-trivially. Log it as a deliberate deviation if
you make it.

## Resolution architecture

Every hit — melee, Emberbrand, Ruinfall, Wake of Hell, enemy/boss attacks —
ends the same way: an instant `UHellwakeGE_Damage` GameplayEffect applied
to the target's ASC with `SetByCaller("Data.Damage")` set to the rolled
amount. `UHellwakeAttributeSet::PostGameplayEffectExecute` is the single
choke point that:
1. Absorbs the hit entirely if the target has `State.IFrame` or `State.Dead`.
2. Otherwise scales by `IncomingDamageMultiplier` (Bulwark sets this to
   0.45 for 10s).
3. Reduces `Health`, fires `Event.Damage.Taken`, and — if `Health` hits 0 —
   fires `Event.Death`.

This is the correct integration point for hit-reaction, camera shake, and
damage-number VFX: bind to `Event.Damage.Taken`/`Event.Death` rather than
threading more logic through each ability.

## Do NOT carry over: instant hit-scan

The prototype resolves melee damage on the click frame — no windup, no
animation-synced contact frame. `UHellwakeAbility_MeleeAttack::ActivateAbility`
currently does the same (calls `ResolveMeleeHit()` immediately, matching
measured behavior), but this is flagged explicitly as **not** something to
keep once real animation exists:

1. Author Light/Heavy attack `AnimMontage`s with an `AnimNotify` at the true
   weapon-contact frame.
2. Change `ActivateAbility` to play the montage and move the
   `ResolveMeleeHit()` call into the notify callback (there's already a
   `UHellwakeCombatComponent::bHitWindowOpen` flag reserved for this).
3. This alone will make combat read as "weighted" per the original design
   brief ("communicate weight, acceleration, impact") — the prototype never
   achieved this mechanically, only visually (trails/shake/hit-stop).

## Numbers

See `Content/Data/AbilityDefinitions.csv` for the full table (also
duplicated as tunables on each ability class — that table is a design
reference, not the runtime source of truth; see its header comment). Key
figures:

| | Reach | Arc | Cooldown | Base Damage | Crit |
|---|---|---|---|---|---|
| Light (LMB) | 3.4m | ±63° | 0.42s | 120 (±15%) | 27% chance, ×2.6 |
| Heavy (RMB) | 4.4m | ±86° | 0.924s | 260 (±15%) | 27% chance, ×2.6 |

Wrath gain on landed hit: +5 (light) / +9 (heavy). Knockback impulse: 50cm/s
(light) / 130cm/s (heavy), via `ACharacter::LaunchCharacter`.

## Dodge

`UHellwakeAbility_Dodge`: 1.1s cooldown, 0.34s `State.IFrame` window
(absorbs all damage, see above), 26 u/s (2600 cm/s) velocity burst along
current movement input (falls back to facing if stationary).

## Build in Editor

- `GE_Hellwake_Damage` → `UHellwakeGE_Damage` (already fully configured in
  C++; no BP child strictly needed, but create one anyway for consistency
  with the other GEs and easier debug-tagging).
- `GE_Cooldown_LightAttack` / `GE_Cooldown_HeavyAttack` → children of
  `UHellwakeGE_Cooldown` (see abilities.md for the full cooldown-GE table).
- `AM_Kaervoss_LightAttack` / `AM_Kaervoss_HeavyAttack` — AnimMontages with
  a contact-frame `AnimNotify` (see "Do NOT carry over" above).
- `BP_Ability_LightAttack` / `BP_Ability_HeavyAttack` — Blueprint children
  of `UHellwakeAbility_LightAttack`/`HeavyAttack` with `DamageEffectClass`,
  `CooldownGameplayEffectClass` assigned.
