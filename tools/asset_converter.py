"""
WolfEngine asset converter.
Usage: asset_converter.py <images_dir> <output_dir> <palettes_dir>

Scans <images_dir> for PNG files (→ Sprite) and GIF files (→ WE_Animation),
converts each and writes .cpp + WE_Assets.hpp into <output_dir>.
"""

import sys
import pathlib

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib

from PIL import Image, ImageSequence


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def rgb888_to_rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def nearest_idx(r: int, g: int, b: int, palette_rgb: list) -> int:
    """Return 1-based palette index of the nearest color in palette_rgb (31 entries)."""
    best_i, best_d = 0, float('inf')
    for i, (pr, pg, pb) in enumerate(palette_rgb):
        d = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
        if d < best_d:
            best_d, best_i = d, i
    return best_i + 1  # 1-based


def name_to_symbol(image_path, images_dir) -> str:
    relative = pathlib.Path(image_path).relative_to(images_dir)
    parts = relative.with_suffix("").parts
    return "_".join(parts).upper().replace("-", "_")


def fmt_hex(v: int) -> str:
    return f"0x{v:04X}"


# ---------------------------------------------------------------------------
# Palette loading
# ---------------------------------------------------------------------------

def _load_toml(path) -> dict:
    """Load a TOML file, transparently stripping a UTF-8 BOM if present."""
    raw = pathlib.Path(path).read_bytes()
    if raw.startswith(b'\xef\xbb\xbf'):
        raw = raw[3:]
    return tomllib.loads(raw.decode('utf-8'))


def load_named_palette(palettes_dir: str, palette_name: str):
    """
    Scan all .toml files in palettes_dir for one whose meta.name matches.
    Returns the parsed TOML dict or None if not found.
    """
    for p in pathlib.Path(palettes_dir).glob('*.toml'):
        data = _load_toml(p)
        if data.get('meta', {}).get('name') == palette_name:
            return data
    return None


def palette_rgb_from_toml(data: dict) -> list:
    """
    Extract 31-entry list of (r,g,b) tuples from a palette TOML dict.
    Index 0 of the returned list corresponds to palette slot 1.
    Missing slots are filled with (0, 0, 0).
    """
    palette_rgb = [(0, 0, 0)] * 31
    for c in sorted(data.get('color', []), key=lambda x: x['index']):
        i = c['index']
        if 1 <= i <= 31:
            palette_rgb[i - 1] = tuple(c['rgb'])
    return palette_rgb


# ---------------------------------------------------------------------------
# Image conversion — PNG
# ---------------------------------------------------------------------------

def auto_palette_convert(img_rgba: Image.Image):
    """
    Auto-quantize to at most 31 colors.
    Returns (indices_2d, palette565_32) where:
      indices_2d    — list of rows, each row a list of uint8 palette indices
      palette565_32 — list of 32 uint16 RGB565 values (index 0 always 0x0000)
    """
    w, h = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h) for x in range(w)]

    opaque_rgb = [(r, g, b) for r, g, b, a in pixels if a >= 128]

    seen = {}
    for c in opaque_rgb:
        if c not in seen:
            seen[c] = len(seen)
    unique = list(seen.keys())

    if len(unique) <= 31:
        palette_rgb = unique + [(0, 0, 0)] * (31 - len(unique))
    else:
        if not opaque_rgb:
            palette_rgb = [(0, 0, 0)] * 31
        else:
            temp = Image.new('RGB', (len(opaque_rgb), 1))
            temp.putdata(opaque_rgb)
            q = temp.quantize(colors=31, dither=0)
            raw = q.getpalette()[:93]  # 31 * 3
            palette_rgb = [(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, 93, 3)]

    palette565 = [0x0000] + [rgb888_to_rgb565(r, g, b) for r, g, b in palette_rgb]

    indices_2d = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = pixels[y * w + x]
            row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
        indices_2d.append(row)

    return indices_2d, palette565


def named_palette_convert(img_rgba: Image.Image, palette_rgb: list):
    """
    Map each pixel to the nearest color in palette_rgb (31 entries).
    Returns indices_2d only (the palette array is the named constant).
    """
    w, h = img_rgba.size
    pix_ = img_rgba.load()
    pixels = [pix_[x, y] for y in range(h) for x in range(w)]

    indices_2d = []
    for y in range(h):
        row = []
        for x in range(w):
            r, g, b, a = pixels[y * w + x]
            row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
        indices_2d.append(row)

    return indices_2d


# ---------------------------------------------------------------------------
# Image conversion — GIF
# ---------------------------------------------------------------------------

def extract_gif_frames(path):
    """
    Open a GIF and return a list of RGBA PIL Images, one per frame.
    Returns None and prints an error if the GIF fails validation.
    """
    name = pathlib.Path(path).name
    try:
        gif = Image.open(path)
    except Exception as e:
        print(f"ERROR: Could not open {name}: {e}")
        return None

    frames = [frame.copy().convert('RGBA') for frame in ImageSequence.Iterator(gif)]

    if len(frames) < 2:
        print(f"ERROR: {name} has only {len(frames)} frame — use a PNG for single sprites. Skipping.")
        return None

    w0, h0 = frames[0].size
    for f in frames[1:]:
        if f.size != (w0, h0):
            print(f"ERROR: {name} has inconsistent frame sizes — all frames must be the same dimensions. Skipping.")
            return None

    if w0 > 96 or h0 > 96:
        print(f"ERROR: {name} frame size {w0}x{h0} exceeds max dimension 96. Skipping.")
        return None

    return frames


def gif_auto_palette_convert(frames: list):
    """
    Build a shared palette from all frames combined, then map each frame to indices.
    Returns (all_indices_2d, palette565) where all_indices_2d is a list of
    per-frame indices_2d arrays.
    """
    all_opaque = []
    for frame in frames:
        all_opaque += [(r, g, b) for r, g, b, a in frame.getdata() if a >= 128]

    if len(set(all_opaque)) <= 31:
        palette_rgb = list(dict.fromkeys(all_opaque))
        palette_rgb += [(0, 0, 0)] * (31 - len(palette_rgb))
    else:
        if not all_opaque:
            palette_rgb = [(0, 0, 0)] * 31
        else:
            temp = Image.new('RGB', (len(all_opaque), 1))
            temp.putdata(all_opaque)
            q = temp.quantize(colors=31, dither=0)
            raw = q.getpalette()[:93]
            palette_rgb = [(raw[i], raw[i + 1], raw[i + 2]) for i in range(0, 93, 3)]

    palette565 = [0x0000] + [rgb888_to_rgb565(*c) for c in palette_rgb]

    all_indices_2d = []
    for frame in frames:
        w, h = frame.size
        pixels = list(frame.getdata())
        indices_2d = []
        for y in range(h):
            row = []
            for x in range(w):
                r, g, b, a = pixels[y * w + x]
                row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
            indices_2d.append(row)
        all_indices_2d.append(indices_2d)

    return all_indices_2d, palette565


def gif_named_palette_convert(frames: list, palette_rgb: list):
    """
    Map each frame's pixels to the nearest color in palette_rgb.
    Returns all_indices_2d (list of per-frame indices_2d).
    """
    all_indices_2d = []
    for frame in frames:
        w, h = frame.size
        pixels = list(frame.getdata())
        indices_2d = []
        for y in range(h):
            row = []
            for x in range(w):
                r, g, b, a = pixels[y * w + x]
                row.append(0 if a < 128 else nearest_idx(r, g, b, palette_rgb))
            indices_2d.append(row)
        all_indices_2d.append(indices_2d)
    return all_indices_2d


def deduplicate_frames(all_indices_2d: list):
    """
    Returns (unique_frames, seq) where:
      unique_frames — list of unique indices_2d (one entry per distinct bitmap)
      seq           — per-original-frame index into unique_frames
    Comparison is byte-for-byte (full memcmp equivalent).
    """
    unique_frames = []
    unique_flat = []
    seq = []

    for indices_2d in all_indices_2d:
        flat = bytes(pixel for row in indices_2d for pixel in row)
        found = -1
        for i, existing in enumerate(unique_flat):
            if flat == existing:
                found = i
                break
        if found >= 0:
            seq.append(found)
        else:
            seq.append(len(unique_frames))
            unique_frames.append(indices_2d)
            unique_flat.append(flat)

    return unique_frames, seq


# ---------------------------------------------------------------------------
# Code generation
# ---------------------------------------------------------------------------

def render_pixel_array(stem: str, indices_2d: list, w: int, h: int) -> str:
    lines = [f"static constexpr uint8_t s_{stem}_pixels[{h}][{w}] = {{"]
    for row in indices_2d:
        inner = ', '.join(str(v) for v in row) + ','
        lines.append(f"    {{ {inner} }},")
    lines.append("};")
    return '\n'.join(lines)


def render_frame_array(stem: str, frame_idx: int, indices_2d: list, w: int, h: int) -> str:
    lines = [f"static constexpr uint8_t s_{stem}_f{frame_idx}[{h}][{w}] = {{"]
    for row in indices_2d:
        inner = ', '.join(str(v) for v in row) + ','
        lines.append(f"    {{ {inner} }},")
    lines.append("};")
    return '\n'.join(lines)


def render_auto_palette_array(stem: str, palette565: list) -> str:
    lines = [f"static constexpr uint16_t s_{stem}_palette[32] = {{"]
    lines.append(f"    {fmt_hex(palette565[0])},  // 0 - transparent (reserved)")
    for i in range(1, 32):
        lines.append(f"    {fmt_hex(palette565[i])},  // {i}")
    lines.append("};")
    return '\n'.join(lines)


def emit_auto_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                  indices_2d: list, palette565: list, w: int, h: int):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    palette_block = render_auto_palette_array(stem, palette565)

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"\n"
        f"{pixel_block}\n"
        f"\n"
        f"{palette_block}\n"
        f"\n"
        f"constexpr Sprite Assets::{symbol} = Sprite::Create(s_{stem}_pixels, s_{stem}_palette);\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_named_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                   indices_2d: list, w: int, h: int,
                   palette_name: str, palette_header: str):
    pixel_block = render_pixel_array(stem, indices_2d, w, h)
    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  palette: {palette_name}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"#include \"{include_path}\"\n"
        f"\n"
        f"{pixel_block}\n"
        f"\n"
        f"constexpr Sprite Assets::{symbol} = Sprite::Create(s_{stem}_pixels, {palette_name});\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_auto_gif_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                      all_indices_2d: list, palette565: list, w: int, h: int):
    n_frames = len(all_indices_2d)
    unique_frames, seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)

    palette_block = render_auto_palette_array(stem, palette565)
    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h)
        for i in range(n_unique)
    )

    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, s_{stem}_palette);"
        for i in range(n_unique)
    )

    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"

    seq_inner = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_frames} frames  {n_unique} unique\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"\n"
        f"{palette_block}\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"{seq_array}\n"
        f"\n"
        f"constexpr WE_AnimationRaw Assets::{symbol} = {{ s_{stem}_sprites, s_{stem}_seq }};\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_named_gif_cpp(output_dir: str, stem: str, symbol: str, source_rel: str,
                       all_indices_2d: list, w: int, h: int,
                       palette_name: str, palette_header: str):
    n_frames = len(all_indices_2d)
    unique_frames, seq = deduplicate_frames(all_indices_2d)
    n_unique = len(unique_frames)

    include_path = f"WolfEngine/Graphics/ColorPalettes/{palette_header}"
    frame_blocks = '\n\n'.join(
        render_frame_array(stem, i, unique_frames[i], w, h)
        for i in range(n_unique)
    )

    sprite_block = '\n'.join(
        f"static constexpr Sprite s_{stem}_spr_{i} = Sprite::Create(s_{stem}_f{i}, {palette_name});"
        for i in range(n_unique)
    )

    sprites_inner = ', '.join(f"&s_{stem}_spr_{i}" for i in range(n_unique))
    sprites_array = f"static constexpr const Sprite* s_{stem}_sprites[{n_unique}] = {{ {sprites_inner} }};"

    seq_inner = ', '.join(str(i) for i in seq) + ', 0xFF'
    seq_array = f"static constexpr uint8_t s_{stem}_seq[] = {{ {seq_inner} }};"

    content = (
        f"// AUTO-GENERATED — do not edit\n"
        f"// Source: {source_rel}  [{w}W x {h}H]  {n_frames} frames  {n_unique} unique  palette: {palette_name}\n"
        f"// #include may look errored in IDEs — it is auto-included by CMake\n"
        f"#include \"WE_Assets.hpp\"\n"
        f"#include \"{include_path}\"\n"
        f"\n"
        f"{frame_blocks}\n"
        f"\n"
        f"{sprite_block}\n"
        f"\n"
        f"{sprites_array}\n"
        f"{seq_array}\n"
        f"\n"
        f"constexpr WE_AnimationRaw Assets::{symbol} = {{ s_{stem}_sprites, s_{stem}_seq }};\n"
    )

    out_path = pathlib.Path(output_dir) / f"{stem}.cpp"
    out_path.write_text(content, encoding='utf-8')
    return str(out_path)


def emit_assets_header(output_dir: str, sprite_symbols: list, anim_symbols: list):
    lines = [
        "// AUTO-GENERATED — do not edit",
        "#pragma once",
        "#include \"WolfEngine/Graphics/SpriteSystem/WE_Sprite.hpp\"",
        "#include \"WolfEngine/Graphics/AnimationSystem/WE_Animation.hpp\"",
        "",
        "namespace Assets {",
    ]
    for sym in sprite_symbols:
        lines.append(f"    extern const Sprite {sym};")
    for sym in anim_symbols:
        lines.append(f"    extern const WE_AnimationRaw {sym};")
    lines.append("}")
    lines.append("")

    out_path = pathlib.Path(output_dir).parent / "WE_Assets.hpp"
    out_path.write_text('\n'.join(lines), encoding='utf-8')


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main(images_dir: str, output_dir: str, palettes_dir: str):
    images_path = pathlib.Path(images_dir)
    output_path = pathlib.Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    # --- Load assets.toml ---
    assets_toml_path = images_path / 'assets.toml'
    asset_config = {}
    if assets_toml_path.exists():
        asset_config = _load_toml(assets_toml_path)

    # --- Collect PNGs and GIFs (sorted for deterministic output) ---
    png_files = sorted(images_path.rglob('*.png'), key=lambda p: str(p).lower())
    gif_files = sorted(images_path.rglob('*.gif'), key=lambda p: str(p).lower())

    all_asset_keys = {
        "_".join(p.relative_to(images_path).with_suffix("").parts).lower().replace("-", "_")
        for p in png_files + gif_files
    }

    # --- Validate assets.toml keys ---
    for key in asset_config:
        if key.lower() not in all_asset_keys:
            print(f"WARNING: assets.toml entry '[{key}]' has no matching PNG or GIF — skipping.")

    # --- Track existing .cpp files for stale cleanup ---
    existing_cpps = {p for p in output_path.glob('*.cpp')}
    written_cpps = set()

    # --- Detect symbol conflicts across PNGs and GIFs ---
    symbol_map = {}
    conflicts = set()
    for path in png_files + gif_files:
        sym = name_to_symbol(path, images_path)
        if sym in symbol_map:
            if sym not in conflicts:
                print(f"ERROR: Symbol conflict — Assets::{sym} would be produced by both:\n"
                      f"  {symbol_map[sym]}\n  {path}\n"
                      f"Skipping both. Rename one of these files.")
            conflicts.add(sym)
        else:
            symbol_map[sym] = path

    converted = 0
    skipped = 0
    errors = 0
    sprite_symbols = []
    anim_symbols = []

    # --- Process PNGs → Sprite ---
    for png_path in png_files:
        symbol = name_to_symbol(png_path, images_path)

        if symbol in conflicts:
            errors += 1
            continue

        stem = symbol.lower()
        relative = png_path.relative_to(images_path)
        source_rel = str(relative).replace("\\", "/")
        config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

        try:
            img = Image.open(png_path).convert('RGBA')
        except Exception as e:
            print(f"ERROR: Could not open {source_rel}: {e}")
            errors += 1
            continue

        w, h = img.size

        if w > 96 or h > 96:
            print(f"ERROR: {source_rel} is {w}x{h}, max dimension is 96. Skipping.")
            errors += 1
            continue

        cfg = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {png_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue

            palette_rgb = palette_rgb_from_toml(palette_data)
            palette_name = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            indices_2d = named_palette_convert(img, palette_rgb)
            cpp_path = emit_named_cpp(
                output_dir, stem, symbol, source_rel, indices_2d, w, h, palette_name, palette_header
            )
        else:
            indices_2d, palette565 = auto_palette_convert(img)
            cpp_path = emit_auto_cpp(
                output_dir, stem, symbol, source_rel, indices_2d, palette565, w, h
            )

        written_cpps.add(pathlib.Path(cpp_path))
        sprite_symbols.append(symbol)
        converted += 1
        print(f"  Converted: {source_rel}  [{w}W x {h}H]  →  {stem}.cpp")

    # --- Process GIFs → WE_Animation ---
    for gif_path in gif_files:
        symbol = name_to_symbol(gif_path, images_path)

        if symbol in conflicts:
            errors += 1
            continue

        stem = symbol.lower()
        relative = gif_path.relative_to(images_path)
        source_rel = str(relative).replace("\\", "/")
        config_key = "_".join(relative.with_suffix("").parts).lower().replace("-", "_")

        frames = extract_gif_frames(gif_path)
        if frames is None:
            errors += 1
            continue

        w, h = frames[0].size
        n = len(frames)

        cfg = asset_config.get(config_key, {})
        palette_key = cfg.get('palette') if cfg else None

        if palette_key:
            palette_data = load_named_palette(palettes_dir, palette_key)
            if palette_data is None:
                available = [p.stem for p in pathlib.Path(palettes_dir).glob('*.toml')]
                print(f"ERROR: Palette '{palette_key}' not found for {gif_path.name}. "
                      f"Available palettes: {available}. Skipping.")
                errors += 1
                continue

            palette_rgb = palette_rgb_from_toml(palette_data)
            palette_name = palette_data['meta']['name']
            palette_header = palette_data['meta']['header']
            all_indices_2d = gif_named_palette_convert(frames, palette_rgb)
            cpp_path = emit_named_gif_cpp(
                output_dir, stem, symbol, source_rel, all_indices_2d, w, h, palette_name, palette_header
            )
        else:
            all_indices_2d, palette565 = gif_auto_palette_convert(frames)
            cpp_path = emit_auto_gif_cpp(
                output_dir, stem, symbol, source_rel, all_indices_2d, palette565, w, h
            )

        written_cpps.add(pathlib.Path(cpp_path))
        anim_symbols.append(symbol)
        converted += 1
        print(f"  Converted: {source_rel}  [{w}W x {h}H]  {n} frames  →  {stem}.cpp")

    # --- Clean up stale .cpp files ---
    for stale in existing_cpps - written_cpps:
        stale.unlink()
        print(f"  Removed stale: {stale.name}")

    # --- Emit WE_Assets.hpp ---
    emit_assets_header(output_dir, sprite_symbols, anim_symbols)

    # --- Summary ---
    print(f"\nAsset conversion complete: {converted} converted, {skipped} skipped, {errors} errors.")
    print(f"Generated: WE_Assets.hpp")


if __name__ == '__main__':
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <images_dir> <output_dir> <palettes_dir>")
        sys.exit(1)

    images_dir, output_dir, palettes_dir = sys.argv[1], sys.argv[2], sys.argv[3]
    main(images_dir, output_dir, palettes_dir)
