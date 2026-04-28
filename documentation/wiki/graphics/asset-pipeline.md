# Asset Pipeline

WolfEngine’s asset pipeline turns PNG artwork and palette TOML files into compile-time C++ assets. The important idea is that sprite data is authored as images, not as hand-written pixel arrays. The build then converts that source data into `constexpr Sprite` definitions and palette headers before compilation.

If you only need the user-facing graphics workflow, see [How To Setup Graphics](how-to-setup-graphics.md). If you want the palette side of the system in more detail, see [Palette System](palette.md).

---

## What The Pipeline Produces

The pipeline produces two kinds of generated output:

| Output | Produced by | Purpose |
|---|---|---|
| `src/GeneratedAssets/palettes/*.hpp` | `tools/generate_palettes.py` | C++ palette headers generated from `tools/palettes/*.toml` |
| `src/GeneratedAssets/sprites/*.cpp` | `tools/asset_converter.py` | One sprite translation unit per PNG |
| `src/GeneratedAssets/WE_Assets.hpp` | `tools/asset_converter.py` | Central include that declares every generated sprite in the `Assets` namespace |

> **Generated files are build artifacts. Do not edit them by hand.**

## Practical Workflow

**To add a new sprite:**

1. Drop the PNG into `Images/` (or a subdirectory like `Images/enemies/`).
2. Optionally add a section to `Images/assets.toml` if you want a fixed palette. (Key must match the generated symbol name in lowercase.)
3. Build the project.
4. Include `GeneratedAssets/WE_Assets.hpp` and use `Assets::<NAME>` in game code.

> **Subdirectory organizing tip:** You can organize sprites into folders (e.g., `Images/enemies/`, `Images/ui/`, `Images/player-states/`) to keep the source directory clean. The build process flattens them into a single output directory but preserves the folder hierarchy in the C++ symbol name for clarity.

**To create a reusable palette for multiple sprites:**

1. Add or update a TOML file in `tools/palettes/`.
2. Set `managed = true` if it should be generated.
3. Reference its `meta.name` from `Images/assets.toml`.
4. Rebuild so both the palette header and the sprites regenerate together.

## Source Inputs

### 1. PNG sprites in `Images/` (including subdirectories)

Each PNG becomes one generated sprite translation unit. The symbol name is built from the full relative path from `Images/`:

| PNG file | C++ symbol |
|---|---|
| `player.png` | `Assets::PLAYER` |
| `enemies/blob.png` | `Assets::ENEMIES_BLOB` |
| `ui/button.png` | `Assets::UI_BUTTON` |
| `player_idle.png` | `Assets::PLAYER_IDLE` |
| `boss-attack.png` | `Assets::BOSS_ATTACK` |
| `enemies-folder/blob.png` | `Assets::ENEMIES_FOLDER_BLOB` |

Rules:

- Subdirectory names and filename are joined with `_`.
- All characters are uppercased.
- Hyphens become underscores (in both directory and file names).
- Underscores are preserved.
- The image must be no larger than 96x96 pixels.
- Symbol conflicts (two files → same name) are detected and both are skipped with an error.

The converter opens each image as RGBA and treats alpha below 128 as transparent.

### 2. Optional per-image settings in `Images/assets.toml`

This file lets you override the palette for a specific image. **Keys must match the generated symbol name in lowercase.**

```toml
[player]
palette = "PALETTE_WARM"

[enemies_blob]
palette = "PALETTE_COOL"

[ui_button]
# no palette key means auto palette selection
```

If no entry exists for a PNG, or the section exists but omits `palette`, the converter uses auto-palette mode.

### 3. Palette definitions in `tools/palettes/*.toml`

The palette generator reads every TOML file in `tools/palettes/`.

The five built-in palettes are:

- `PALETTE_WARM`
- `PALETTE_COOL`
- `PALETTE_GAMEBOY`
- `PALETTE_GRAYSCALE`
- `PALETTE_SUNSET`

Those files are hand-authored and skipped.

**Important rules:**

- Index 0 is always transparent and always written as `0x0000`.
- Slots 1 through 31 are the visible palette entries.
- The generated array always has 32 entries.
- `meta.name` is the palette constant name used by sprites and `Images/assets.toml`.
- `meta.header` controls the output filename.
- `managed = false` means “skip generation” and leave the file alone.

The generated headers provide two things:

1. A named enum for readable color indices.
2. A `constexpr uint16_t` palette array for `Sprite::Create`.

That lets the same palette data serve both hand-authored code and generated sprites.

---

## Image Conversion Modes:

### Auto-palette mode

If `Images/assets.toml` does not provide a palette for an image, the converter automatically generate a palette for it in its hpp file.

This mode is good for quick imports when you do not care about palette reuse.

### Named-palette mode

If `Images/assets.toml` sets `palette = "PALETTE_WARM"` or another palette name, the converter:

1. Looks up the matching TOML file in `tools/palettes/` by `meta.name` and builds a 31-color list from the TOML data.
2. Maps each opaque pixel to the nearest palette entry.
3. Emits a sprite `.cpp` that includes the generated palette header and references the named palette constant.

This is the preferred path when you want consistent art style or want to share a palette across multiple sprites.

---

### Source inclusion

`src/CMakeLists.txt` glob-recurse picks up everything under `src/`, including the generated sprite and palette folders. That means generated files are compiled automatically once they exist.

### Generated asset header

`WE_Assets.hpp` is the single include point for generated sprites. It exposes only declarations, so other translation units can link against the generated `Sprite` constants without including every generated `.cpp` directly.

---

## Common Failure Modes

| Situation | What usually happened |
|---|---|
| Sprite is skipped because it is too large | The PNG exceeded 96x96 pixels |
| Named palette not found | `Images/assets.toml` referenced a `meta.name` that does not exist in `tools/palettes/` |
| Palette header did not regenerate | The palette TOML is marked `managed = false` |
| Symbol conflict — both files skipped | Two PNG paths produce the same C++ symbol name (e.g., `enemies/blob.png` and `enemies-blob.png` both → `ENEMIES_BLOB`). Rename one file to resolve. |
| `assets.toml` key not found | An entry in `Images/assets.toml` has no matching PNG file (check that the key matches the generated symbol name in lowercase) |

---