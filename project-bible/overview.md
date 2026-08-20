# Hellwake — Project Bible

Dark-fantasy isometric action RPG. This directory is the design-to-engineering
bridge for the native Unreal Engine 5 port: everything here documents what
the game *is* and how the Unreal implementation reproduces it, cross-
referenced against real C++ in `Source/Hellwake/`.

## Provenance

Hellwake started as a Claude Design mockup, iterated into a fully playable
Three.js prototype (`design-reference/project/hellwake-game.js` +
`Hellwake Playable Slice.dc.html`), then was play-tested and documented
(`docs/hellwake-behavioral-spec.md`) before this native port began. Three
artifacts matter, in order of authority for *behavior*:

1. **`docs/hellwake-behavioral-spec.md`** — what the prototype actually
   does, confirmed by driving a real running build with Playwright, not
   just reading source. This is the primary behavioral spec.
2. **`design-reference/`** — the original Claude Design chat transcript and
   exported prototype files, preserved unmodified. Primary source for
   *visual direction* (palette, typography, composition) — see the "FREEZE"
   list the user approved partway through the chat.
3. **This directory (`project-bible/`)** — the Unreal-specific translation
   of both, plus everything Unreal needs that the prototype didn't (level
   composition, NavMesh, State Trees, save data).

Where the prototype's behavior was a bug or accidental limitation rather
than a design intent, it's called out explicitly rather than silently
carried forward — see combat.md's "Targeting" section for the clearest
example (no lock-on, facing-cone melee).

## Directory map

```
Hellwake.uproject
Source/Hellwake/          C++ gameplay code (see unreal-implementation.md)
Source/Hellwake.Target.cs, HellwakeEditor.Target.cs
Config/                   DefaultEngine.ini, DefaultGame.ini, DefaultInput.ini,
                           Tags/GameplayTags.ini
Content/Data/              *.csv sources for the DataTables (import in-editor)
design-reference/          Original Claude Design handoff, unmodified
docs/hellwake-behavioral-spec.md   Play-tested behavioral spec
project-bible/              This directory
```

## What exists vs. what needs the Editor

Written and real: every `.h`/`.cpp` under `Source/Hellwake/` — GAS
AttributeSet, all seven player abilities, the enemy/boss AI, loot, the
encounter state machine, GameMode, HUD C++ binding layer, DataTable row
structs and their CSV data, GameplayEffect C++ classes, ini config.

**Not created, and not fakeable outside the Editor:** any `.uasset` —
Blueprint children of the C++ classes above (every ability needs a BP
child that points its `CooldownGameplayEffectClass`/`DamageEffectClass`
at real GameplayEffect assets), meshes, animations/montages, Niagara
systems, the UMG widget tree, Input Actions/Mapping Context, the State
Tree asset, NavMesh bounds, and the level itself. Each `project-bible/*.md`
file below has a "Build in Editor" section listing exactly what to create
and how it should reference the C++ this bible describes.

See `project-bible/unreal-implementation.md` for the engineering notes and
`project-bible/vertical-slice.md` + the verification checklist for the
end-to-end build/validate plan once a workstation with UE5 installed picks
this up.
