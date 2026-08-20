# Loot

Four rarities, `Content/Data/LootDefinitions.csv` → `DT_LootDefinitions`
(`FHellwakeLootDefinition`, `Source/Hellwake/Data/HellwakeLootDefinition.h`).

| Rarity | Item | Roll threshold |
|---|---|---|
| Common | Ashen Fragment | (default) |
| Magic | Warded Sigil | roll > 0.6 |
| Rare | Cinderforged Plate | roll > 0.93 |
| Legendary | Ashfall, the Last Vow | guaranteed Gravewarden drop only — never rolled |

## Drop flow

1. Non-elite enemy dies → `AHellwakeEnemyBase::HandleDeath` calls
   `UHellwakeLootDropComponent::RollAndMaybeDrop(LootDropChance, Location)`.
2. First roll: `FRand() < LootDropChance` (per-archetype, see enemies.md's
   table) — if it misses, no drop, done.
3. Second roll: `FRand()` thresholded against `RollThreshold` walking
   common → magic → rare, taking the highest threshold cleared — matches
   the prototype's single-roll-two-threshold logic exactly (see
   `HellwakeLootDropComponent.cpp`'s comment for why it's a threshold walk
   rather than a weighted table).
4. `AHellwakeLootPickup` spawns at the death location; bobs/rotates in
   `Tick()`.
5. Player overlaps `PickupSphere` (220cm radius, matching the prototype's
   unconditional 2.2u auto-pickup — no button press) → destroys itself and
   sends `Event.Loot.PickedUp` to the player's ASC.

The Gravewarden bypasses this component entirely —
`AHellwakeGravewarden::HandleDeath` spawns `LegendaryLootPickupClass`
directly, unconditionally, matching `dropLoot(..., 'legendary')` in the
prototype's `setStage('reward')`.

## Known gap: HUD toast doesn't know the item yet

`UHellwakeHUDWidget::HandleLootPickedUp` is wired to the `Event.Loot.PickedUp`
gameplay event but currently can't resolve *which* rarity/item was picked
up — `FGameplayEventData` has no clean slot for an `FName` row identifier,
and threading it through wasn't worth guessing at blind. Before this HUD
toast can work, either:
- Add a small `UHellwakeLootPickupInterface` the pickup calls directly on
  the player pawn with `(FName RarityRow, UDataTable* Table)`, or
- Extend `FGameplayEventData` usage via `EventMagnitude` as an index into
  a well-known rarity enum, or
- Have `AHellwakeLootPickup` cache the resolved `FHellwakeLootDefinition`
  and pass a pointer via `Payload.OptionalObject` (currently passes the
  *table*, not the resolved row — see the `TODO` in
  `HellwakeLootPickup.cpp`).

Pick one and wire it before shipping the pickup toast — don't leave the
`OnLootToast(FText::GetEmpty(), ...)` stub call in `HellwakeHUDWidget.cpp`
as final.

## Build in Editor

- `BP_LootPickup_Common/Magic/Rare/Legendary` (or one `BP_LootPickup` with
  `RarityRowName` set per spawn — the C++ already supports either) —
  Blueprint children of `AHellwakeLootPickup` with `ItemMesh` (an
  octahedron gem, matching the prototype's geometry) and
  `LootDefinitionTable` assigned.
- A beam/pool Niagara or material effect around the beacon light, matching
  the prototype's `beam`/`pool` cylinder+circle meshes (restrained,
  additive-blended, colored per rarity — see lighting-vfx.md).
