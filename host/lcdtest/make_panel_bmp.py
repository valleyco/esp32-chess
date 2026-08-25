#!/usr/bin/env python3
"""Write a 320x240 RGB BMP of the intended CYD panel layout for offline inspection.

Panel: 320x240
Game:  256x224 (native Midway: 32 bytes/row x 224) centered → margins 32+32 X, 8+8 Y
"""
from __future__ import annotations

import struct
from pathlib import Path

PANEL_W, PANEL_H = 320, 240
GAME_W, GAME_H = 256, 224
OFF_X = (PANEL_W - GAME_W) // 2  # 32
OFF_Y = (PANEL_H - GAME_H) // 2  # 8

# RGB888
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
YELLOW = (255, 255, 0)
CYAN = (0, 255, 255)
MAGENTA = (255, 0, 255)
DARK = (32, 32, 32)


def fill_rect(px: list[list[tuple[int, int, int]]], x: int, y: int, w: int, h: int, c):
    for row in range(y, y + h):
        if row < 0 or row >= PANEL_H:
            continue
        for col in range(x, x + w):
            if 0 <= col < PANEL_W:
                px[row][col] = c


# Minimal 5x7 font, MSB = left (same as firmware after fix)
FONT: dict[str, list[int]] = {
    " ": [0, 0, 0, 0, 0, 0, 0],
    "2": [0x1E, 0x01, 0x01, 0x0E, 0x10, 0x10, 0x1F],
    "4": [0x11, 0x11, 0x11, 0x1F, 0x01, 0x01, 0x01],
    "5": [0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E],
    "6": [0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E],
    "B": [0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E],
    "E": [0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F],
    "G": [0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E],
    "I": [0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E],
    "L": [0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F],
    "M": [0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11],
    "N": [0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11],
    "O": [0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E],
    "R": [0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11],
    "T": [0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04],
    "V": [0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04],
    "X": [0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11],
}

SCALE = 2
GLYPH_W = 5 * SCALE
GLYPH_H = 7 * SCALE
GAP = SCALE


def text_width(s: str) -> int:
    n = len(s)
    return 0 if n == 0 else n * GLYPH_W + (n - 1) * GAP


def draw_char(px, x, y, ch, fg, bg):
    g = FONT.get(ch, FONT[" "])
    for row, bits in enumerate(g):
        for col in range(5):
            color = fg if (bits & (1 << (4 - col))) else bg
            fill_rect(px, x + col * SCALE, y + row * SCALE, SCALE, SCALE, color)


def draw_text(px, x, y, s, fg, bg):
    for ch in s:
        draw_char(px, x, y, ch, fg, bg)
        x += GLYPH_W + GAP


def build_panel() -> list[list[tuple[int, int, int]]]:
    px = [[BLACK for _ in range(PANEL_W)] for _ in range(PANEL_H)]

    # Game rect origin in panel coords
    ox, oy = OFF_X, OFF_Y

    def gfill(x, y, w, h, c):
        fill_rect(px, ox + x, oy + y, w, h, c)

    def gtext(x, y, s, fg, bg):
        draw_text(px, ox + x, oy + y, s, fg, bg)

    gfill(0, 0, GAME_W, GAME_H, DARK)
    gfill(0, 0, GAME_W, 28, RED)
    gfill(0, GAME_H - 28, GAME_W, 28, BLUE)
    gfill(0, 0, 24, GAME_H, GREEN)
    gfill(GAME_W - 24, 0, 24, GAME_H, YELLOW)
    gfill(0, 0, 24, 28, WHITE)
    gfill(GAME_W - 24, 0, 24, 28, CYAN)
    gfill(0, GAME_H - 28, 24, 28, MAGENTA)
    gfill(GAME_W - 24, GAME_H - 28, 24, 28, YELLOW)

    gtext((GAME_W - text_width("TOP")) // 2, 6, "TOP", BLACK, RED)
    gtext((GAME_W - text_width("BOTTOM")) // 2, GAME_H - 22, "BOTTOM", WHITE, BLUE)
    gtext(4, GAME_H // 2 - GLYPH_H // 2, "L", BLACK, GREEN)
    gtext(GAME_W - 4 - GLYPH_W, GAME_H // 2 - GLYPH_H // 2, "R", BLACK, YELLOW)
    gtext(2, 6, "TL", BLACK, WHITE)
    gtext(GAME_W - 2 - text_width("TR"), 6, "TR", BLACK, CYAN)
    gtext(2, GAME_H - 22, "BL", WHITE, MAGENTA)
    gtext(GAME_W - 2 - text_width("BR"), GAME_H - 22, "BR", BLACK, YELLOW)
    gtext((GAME_W - text_width("256x224")) // 2, GAME_H // 2 - GLYPH_H // 2, "256x224", WHITE, DARK)

    # Mark panel corners so empty borders are obvious
    fill_rect(px, 0, 0, 4, 4, WHITE)
    fill_rect(px, PANEL_W - 4, 0, 4, 4, CYAN)
    fill_rect(px, 0, PANEL_H - 4, 4, 4, MAGENTA)
    fill_rect(px, PANEL_W - 4, PANEL_H - 4, 4, 4, YELLOW)

    return px


def write_bmp_rgb(path: Path, px: list[list[tuple[int, int, int]]]):
    h = len(px)
    w = len(px[0])
    row_stride = (w * 3 + 3) & ~3
    pixel_size = row_stride * h
    file_size = 14 + 40 + pixel_size

    with path.open("wb") as f:
        # BITMAPFILEHEADER
        f.write(b"BM")
        f.write(struct.pack("<IHHI", file_size, 0, 0, 14 + 40))
        # BITMAPINFOHEADER
        f.write(
            struct.pack(
                "<IIIHHIIIIII",
                40,  # header size
                w,
                h,
                1,  # planes
                24,  # bpp
                0,  # compression
                pixel_size,
                2835,
                2835,
                0,
                0,
            )
        )
        # Pixels bottom-up, BGR
        pad = b"\x00" * (row_stride - w * 3)
        for y in range(h - 1, -1, -1):
            for r, g, b in px[y]:
                f.write(struct.pack("BBB", b, g, r))
            f.write(pad)


def write_rgb565_header(path: Path, px: list[list[tuple[int, int, int]]]):
    h = len(px)
    w = len(px[0])
    pixels: list[int] = []
    for y in range(h):
        for r, g, b in px[y]:
            pixels.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

    lines = [
        "#pragma once",
        "#include <stdint.h>",
        "",
        f"#define LCD_TEST_PANEL_W {w}",
        f"#define LCD_TEST_PANEL_H {h}",
        "#define LCD_TEST_PANEL_PIXELS (LCD_TEST_PANEL_W * LCD_TEST_PANEL_H)",
        "",
        "/* Generated by host/lcdtest/make_panel_bmp.py — do not edit. */",
        "static const uint16_t lcd_test_panel_rgb565[LCD_TEST_PANEL_PIXELS] = {",
    ]
    row: list[str] = []
    for p in pixels:
        row.append(f"0x{p:04X}")
        if len(row) == 12:
            lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("    " + ", ".join(row) + ",")
    lines.append("};")
    lines.append("")
    path.write_text("\n".join(lines))


def main():
    root = Path(__file__).resolve().parents[2]
    bmp = root / "lcd_test_320x240.bmp"
    gen = root / "host/lcdtest/generated"
    gen.mkdir(parents=True, exist_ok=True)
    hdr = gen / "lcd_test_panel_rgb565.h"
    px = build_panel()
    write_bmp_rgb(bmp, px)
    write_rgb565_header(hdr, px)
    print(f"wrote {bmp}")
    print(f"wrote {hdr}")
    print(f"panel {PANEL_W}x{PANEL_H}, game {GAME_W}x{GAME_H} at ({OFF_X},{OFF_Y})")
    print(f"empty X={PANEL_W - GAME_W} Y={PANEL_H - GAME_H}")


if __name__ == "__main__":
    main()
