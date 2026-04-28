#!/usr/bin/env python3
"""Generate C++ palette headers from TOML definitions in tools/palettes/."""

import argparse
import os
import sys

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib


def rgb888_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def generate_palette_header(toml_path, output_dir):
    raw = open(toml_path, "rb").read()
    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    data = tomllib.loads(raw.decode("utf-8"))

    meta = data.get("meta", {})
    managed = meta.get("managed", True)
    palette_name = meta.get("name", os.path.basename(toml_path))

    if not managed:
        print(f"Skipping: {palette_name} (unmanaged)")
        return "skipped"

    enum_type = meta["enum"]
    header_file = meta["header"]
    description = meta.get("description", "")

    colors = sorted(data.get("color", []), key=lambda c: c["index"])

    # Build RGB565 array; slot 0 is always 0x0000 (transparent, reserved)
    palette = [0x0000] * 32
    for color in colors:
        idx = color["index"]
        if 1 <= idx <= 31:
            r, g, b = color["rgb"]
            palette[idx] = rgb888_to_rgb565(r, g, b)

    color_by_index = {c["index"]: c for c in colors}

    # Enum lines — align '=' to the longest name
    enum_entries = [(c["enum"], c["index"]) for c in colors]
    max_name_len = max((len(name) for name, _ in enum_entries), default=0)
    enum_lines = []
    for enum_name, idx in enum_entries:
        padding = " " * (max_name_len - len(enum_name) + 1)
        enum_lines.append(f"    {enum_name}{padding}= {idx},")

    # Array lines — 32 slots in order, pad missing with (unused)
    array_lines = []
    for i in range(32):
        val = palette[i]
        if i == 0:
            label = "Transparent (reserved)"
        elif i in color_by_index:
            label = color_by_index[i]["name"]
        else:
            label = "(unused)"
        array_lines.append(f"    0x{val:04X},  // {i:<2} - {label}")

    source_rel = "tools/palettes/" + os.path.basename(toml_path)

    lines = [
        "// AUTO-GENERATED — do not edit",
        f"// Source: {source_rel}",
        "#pragma once",
        "#include <stdint.h>",
        "",
        f"// {description}",
        "// Index 0 is always transparent (reserved by the engine).",
        "",
        f"enum {enum_type} : uint8_t {{",
        *enum_lines,
        "};",
        "",
        f"constexpr uint16_t {palette_name}[32] = {{",
        *array_lines,
        "};",
        "",
    ]

    output_path = os.path.join(output_dir, header_file)
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    print(f"Generated: {header_file}")
    return "generated"


def main(palettes_dir, output_dir):
    os.makedirs(output_dir, exist_ok=True)

    toml_files = sorted(f for f in os.listdir(palettes_dir) if f.endswith(".toml"))

    generated = 0
    skipped = 0
    errors = 0

    for filename in toml_files:
        toml_path = os.path.join(palettes_dir, filename)
        try:
            result = generate_palette_header(toml_path, output_dir)
            if result == "generated":
                generated += 1
            else:
                skipped += 1
        except Exception as e:
            print(f"ERROR processing {filename}: {e}", file=sys.stderr)
            errors += 1

    print(
        f"Palette generation complete: {generated} generated, "
        f"{skipped} skipped (unmanaged), {errors} errors."
    )

    if errors:
        sys.exit(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Generate C++ palette headers from TOML definitions."
    )
    parser.add_argument("palettes_dir", help="Directory containing palette .toml files")
    parser.add_argument("output_dir", help="Directory to write generated .hpp headers")
    args = parser.parse_args()
    main(args.palettes_dir, args.output_dir)
