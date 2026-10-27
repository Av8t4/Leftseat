#!/usr/bin/env python3
"""Makes the printable AR card for Left Seat's hangar: docs/ar_marker.png (and a print-ready A4 page, docs/ar_marker_a4.png).
The card is a 6 x 6 grid: a black border one cell wide, and a 4 x 4 pattern inside. The pattern is turned so that the game
reads it the same way the card is printed (the top of the card is the nose of the aircraft)."""
import sys
from PIL import Image, ImageDraw

PATTERN = [  # 1 = white, 0 = black (any pattern that looks different when turned will do)
    [1, 1, 0, 1],
    [0, 1, 0, 0],
    [1, 0, 0, 1],
    [0, 0, 1, 1],
]

def code(p):
    return sum((1 << (r * 4 + c)) for r in range(4) for c in range(4) if p[r][c])

def turn(p):
    return [[p[3 - c][r] for c in range(4)] for r in range(4)]

def canonical():
    best, bp, q = None, None, PATTERN
    for _ in range(4):
        if best is None or code(q) < best:
            best, bp = code(q), q
        q = turn(q)
    return bp

def card(cell=100, margin=1):
    p = canonical()
    n = 6 + 2 * margin
    im = Image.new("L", (n * cell, n * cell), 255)
    d = ImageDraw.Draw(im)
    o = margin * cell
    d.rectangle([o, o, o + 6 * cell - 1, o + 6 * cell - 1], fill=0)
    for r in range(4):
        for c in range(4):
            if p[r][c]:
                x, y = o + (c + 1) * cell, o + (r + 1) * cell
                d.rectangle([x, y, x + cell - 1, y + cell - 1], fill=255)
    return im

if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else "docs/ar_marker.png"
    card().save(out)
    # A4 at 150 dpi (1240 x 1754): the card about 9 cm square, a little under 4 inches
    a4 = Image.new("L", (1240, 1754), 255)
    c = card(cell=90, margin=1)
    a4.paste(c, ((1240 - c.width) // 2, 300))
    d = ImageDraw.Draw(a4)
    d.text((120, 120), "LEFT SEAT - AR card. Print at 100% (no scaling), cut out and put on your desk.", fill=0)
    d.text((120, 150), "Black border must stay clear and the paper flat. Matt paper works best.", fill=0)
    a4.save(out.replace(".png", "_a4.png"))
