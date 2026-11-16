#!/usr/bin/env python3
"""Make the Galactic Detour build's LiveArea pictures from the Left Seat ones, so the two apps look different on the Vita home screen.

The blue sky turns violet, the startup picture says GALACTIC DETOUR and the icon gets a gold star. Needs Pillow.

usage: make_detour_art.py        (run from the project folder; writes sce_sys/detour/)
"""
import colorsys
import os
from PIL import Image, ImageDraw, ImageFont

SRC = "sce_sys"
DST = os.path.join("sce_sys", "detour")
FILES = ["icon0.png", os.path.join("livearea", "contents", "bg0.png"), os.path.join("livearea", "contents", "startup.png")]

def violet(im):
    """Turn blue pixels violet (gold, white and red are left alone)."""
    im = im.convert("RGB")
    px = im.load()
    for y in range(im.height):
        for x in range(im.width):
            r, g, b = px[x, y]
            h, s, v = colorsys.rgb_to_hsv(r / 255.0, g / 255.0, b / 255.0)
            if 0.50 <= h <= 0.72 and s > 0.15:
                h = 0.5 + (h - 0.5) * 0.35 + 0.30            # about 0.50-0.72 -> 0.80-0.88 (violet to magenta)
                r, g, b = colorsys.hsv_to_rgb(h % 1.0, min(1.0, s * 1.05), v)
                px[x, y] = (int(r * 255), int(g * 255), int(b * 255))
    return im

def font(size):
    for p in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf"):
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default()

def star(d, cx, cy, r, fill):
    import math
    pts = []
    for i in range(10):
        a = -math.pi / 2 + i * math.pi / 5
        rr = r if i % 2 == 0 else r * 0.42
        pts.append((cx + rr * math.cos(a), cy + rr * math.sin(a)))
    d.polygon(pts, fill=fill)

def main():
    os.makedirs(os.path.join(DST, "livearea", "contents"), exist_ok=True)
    for f in FILES:
        im = violet(Image.open(os.path.join(SRC, f)))
        d = ImageDraw.Draw(im)
        if f == "icon0.png":
            d.ellipse((4, 90, 36, 122), fill=(30, 12, 64), outline=(255, 206, 130), width=2)
            star(d, 20, 106, 11, (255, 206, 130))
        elif f.endswith("startup.png"):
            w, h = im.size
            band = im.getpixel((8, h - 12))
            d.rectangle((0, 130, w, h), fill=band)                          # cover "FLIGHT SIMULATOR"
            t = "GALACTIC DETOUR"
            ft = font(15)
            tw = d.textlength(t, font=ft)
            d.text(((w - tw) / 2, 134), t, font=ft, fill=(255, 206, 130))
        elif f.endswith("bg0.png"):
            star(d, 90, 80, 14, (255, 206, 130)); star(d, 770, 420, 10, (255, 206, 130)); star(d, 700, 60, 7, (255, 255, 255))
        im.save(os.path.join(DST, f))
        print("wrote", os.path.join(DST, f))

main()
