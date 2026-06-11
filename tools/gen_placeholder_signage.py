#!/usr/bin/env python3
"""Generate placeholder signage PNGs for AZADI UE5 (no PIL, stdlib only).

Abstract geometric placeholders — regime propaganda boards (grim, striped)
vs. liberation murals (tricolor bands, sun disc). Real Persian signage art
replaces these via signage sets or mod packs; the pipeline (runtime PNG
import) is identical.

Usage: python3 tools/gen_placeholder_signage.py   (from ue5/azadi/)
"""

from __future__ import annotations

import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Palette
INK = (24, 24, 30)
SLATE = (52, 52, 62)
GRAY = (88, 88, 98)
CRIMSON = (148, 32, 36)
TEAL = (31, 191, 168)
GOLD = (255, 210, 74)
ROSE = (224, 69, 90)
CREAM = (244, 238, 222)
MAGENTA = (236, 64, 222)
CYAN = (64, 222, 236)
NIGHT = (16, 12, 24)


def write_png(path: Path, width: int, height: int, painter) -> None:
    rows = bytearray()
    for y in range(height):
        rows.append(0)  # no filter
        for x in range(width):
            r, g, b = painter(x, y, width, height)
            rows.extend((r, g, b))

    def chunk(tag: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    png = (
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", ihdr)
        + chunk(b"IDAT", zlib.compress(bytes(rows), 9))
        + chunk(b"IEND", b"")
    )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png)
    print(f"  wrote {path.relative_to(ROOT)} ({width}x{height})")


def border(x, y, w, h, margin, color, inner):
    if x < margin or y < margin or x >= w - margin or y >= h - margin:
        return color
    return inner


def occupied(variant: int):
    """Regime board: dark slate, harsh diagonal crimson band, heavy border."""

    def paint(x, y, w, h):
        period = max(24, w // 6)
        band = ((x + y * 2) // period) % 4 == variant % 4
        base = SLATE if ((x // 16) + (y // 16)) % 2 == 0 else INK
        inner = CRIMSON if band else base
        if band and ((x + y) % 7 == 0):
            inner = INK  # scratched
        return border(x, y, w, h, max(3, w // 40), INK, inner)

    return paint


def liberated(variant: int):
    """Mural: tricolor horizontal bands + gold sun disc."""

    def paint(x, y, w, h):
        cx, cy, radius = w * 0.5, h * 0.46, min(w, h) * 0.21
        if (x - cx) ** 2 + (y - cy) ** 2 < radius**2:
            return GOLD
        third = h / 3
        bands = [TEAL, CREAM, ROSE]
        color = bands[min(2, int(y / third))]
        if variant % 2 == 1 and (x // max(12, w // 18)) % 5 == 0:
            color = GOLD  # vertical gold threads
        return border(x, y, w, h, max(2, w // 56), INK, color)

    return paint


def neon(variant: int):
    """Mod pack look: neon stripes over near-black."""

    def paint(x, y, w, h):
        period = max(16, w // 9)
        stripe = ((x + (y // 2 if variant % 2 else -y // 2)) // period) % 3
        color = NIGHT
        if stripe == 0:
            color = MAGENTA
        elif stripe == 1 and (y // 8) % 2 == 0:
            color = CYAN
        return border(x, y, w, h, max(2, w // 48), NIGHT, color)

    return paint


SLOTS = [
    ("awning", 256, 128),
    ("billboard", 512, 256),
    ("mural", 256, 256),
    ("banner", 128, 256),
]


def main() -> None:
    core = ROOT / "Content/Azadi/Data/SignageSets/textures"
    mod = ROOT / "Mods/example_neon_dawn/textures"

    print("core signage placeholders:")
    for i, (slot, w, h) in enumerate(SLOTS):
        write_png(core / f"occupied_{slot}.png", w, h, occupied(i))
        write_png(core / f"liberated_{slot}.png", w, h, liberated(i))

    print("example mod (neon dawn):")
    for i, (slot, w, h) in enumerate(SLOTS):
        write_png(mod / f"occupied_{slot}.png", w, h, occupied(i + 1))
        write_png(mod / f"neon_{slot}.png", w, h, neon(i))

    print("done.")


if __name__ == "__main__":
    main()
