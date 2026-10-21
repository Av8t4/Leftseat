#!/usr/bin/env python3
"""Remove every optional chunk from a PNG (metadata, colour profiles, content-credential blocks, ...).

The PS Vita's icon loader can refuse PNGs that carry extra chunks and then shows a plain white icon.
Only the chunks needed to draw the picture are kept: IHDR, PLTE, tRNS, IDAT and IEND.

usage: strip_png.py input.png output.png     (input and output may be the same file)
"""
import struct
import sys

KEEP = (b"IHDR", b"PLTE", b"tRNS", b"IDAT", b"IEND")

def strip(src, dst):
    data = open(src, "rb").read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit("%s is not a PNG file" % src)
    pos, out, dropped = 8, [data[:8]], 0
    while pos < len(data):
        length = struct.unpack(">I", data[pos:pos + 4])[0]
        kind = data[pos + 4:pos + 8]
        chunk = data[pos:pos + 12 + length]
        if kind in KEEP:
            out.append(chunk)
        else:
            dropped += 1
        pos += 12 + length
    tmp = dst + ".tmp"
    with open(tmp, "wb") as f:
        f.write(b"".join(out))
    import os
    os.replace(tmp, dst)
    return dropped

if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit(__doc__)
    n = strip(sys.argv[1], sys.argv[2])
    print("%s: removed %d extra chunk%s" % (sys.argv[2], n, "" if n == 1 else "s"))
