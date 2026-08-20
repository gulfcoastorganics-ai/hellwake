# Abilities

Seven player actions, each a `UHellwakeGameplayAbility` subclass in
`Source/Hellwake/Abilities/`. All share the same activation gate
(`ActivationBlockedTags` on `State.Dead` + `State.Cinematic`, set in the
base class constructor) — ports the prototype's universal
`if (P.dead || cine > 0) return;` check without repeating it per-ability.

Full numbers table: `Content/Data/AbilityDefinitions.csv` (reference only —
see its header comment for why; the ability class's own `EditDefaultsOnly`
properties, set on its Blueprint child, are authoritative at runtime).

| Key | Ability | C++ class | Cost | Cooldown | Notes |
|---|---|---|---|---|---|
| LMB | Light Attack | `UHellwakeAbility_LightAttack` | 0 | 0.42s | see combat.md |
| RMB | Heavy Attack | `UHellwakeAbility_HeavyAttack` | 0 | 0.924s | see combat.md |
| Shift/Space | Dodge | `UHellwakeAbility_Dodge` | 0 | 1.1s | 0.34s i-frame |
| Q | Emberbrand | `UHellwakeAbility_Emberbrand` | 18 Wrath | 6s | instant AoE, always crit-tier |
| F | Ashen Bulwark | `UHellwakeAbility_Bulwark` | 12 Wrath | 12s | 10s, -55% incoming dmg |
| E | Ruinfall | `UHellwakeAbility_Ruinfall` | 22 Wrath | 9s | 0.75s telegraph, then AoE |
| R | Wake of Hell | `UHellwakeAbility_WakeOfHell` | 0 | 40s | self nova, distance falloff, 1.6s cinematic lock |

## Per-ability GAS wiring needed in Editor

Every ability class exposes `TSubclassOf<UGameplayEffect>` properties for
its cooldown/damage/status effects — none have C++ defaults, all must be
set on a Blueprint child:

- **All melee/AoE abilities**: `DamageEffectClass` → `GE_Hellwake_Damage`
  (`UHellwakeGE_Damage`, already fully configured).
- **Every ability**: `CooldownGameplayEffectClass` (inherited from
  `UGameplayAbility`) → a Blueprint child of `UHellwakeGE_Cooldown` with
  `DurationMagnitude` set to that ability's cooldown and exactly one
  `Cooldown.*` tag added (see `Config/Tags/GameplayTags.ini` for the full
  list) — this is the standard GAS cooldown pattern, see
  `HellwakeGE_Cooldown.h`'s header comment for the full table.
- **Emberbrand**: `EmberStatusEffectClass` → a duration-9s GE granting
  `Status.Ember` (cosmetic HUD pip only, no attribute modifier needed).
- **Bulwark**: `BulwarkEffectClass` → `UHellwakeGE_Bulwark` (already fully
  configured).
- **Dodge**: `IFrameEffectClass` → `UHellwakeGE_IFrame` (already
  configured; keep its 0.34s duration in sync with
  `UHellwakeAbility_Dodge::IFrameDuration` if either changes).
- **Ruinfall / Wake of Hell**: `DamageEffectClass` as above; Wake of Hell
  additionally needs `CinematicLockEffectClass` → `UHellwakeGE_CinematicLock`.

Then create `BP_Ability_<Name>` Blueprint children of each C++ class,
assign the above, and add all seven to `AHellwakeCharacter::StartingAbilities`
on `BP_HellwakeCharacter`.

## Enhanced Input

`IMC_Hellwake_Default` mapping context with `IA_Move` (2D axis, WASD),
`IA_LightAttack`/`IA_HeavyAttack` (mouse buttons), `IA_Dodge` (Shift +
Space, both bound to the same action), `IA_Emberbrand`/`IA_Bulwark`/
`IA_Ruinfall`/`IA_WakeOfHell` (Q/F/E/R). Assign all eight `UInputAction*`
properties + the mapping context on `BP_HellwakeCharacter`. All seven
ability actions use `ETriggerEvent::Started` (already bound in
`AHellwakeCharacter::SetupPlayerInputComponent`) — no additional Blueprint
input logic needed.

## VFX gaps (see lighting-vfx.md for the full Niagara list)

Every ability's `.cpp` has `TODO(VFX):`/`TODO(Camera):` comments citing the
exact prototype call (`burst(...)`, `shockwave(...)`, `shake = max(...)`)
at the point a Niagara spawn / `TriggerShake` call belongs.
