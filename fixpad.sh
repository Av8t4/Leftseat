#!/bin/bash
# Run from the build folder:  bash ../fixpad.sh
# Works around: "vita-elf-create: Cannot allocate N bytes for SCE data at end of segment 0;
# segment 1 overlaps". It measures the free space after the code segment and, if it is too
# small, grows src/pad.c so the data segment moves out of the way, then builds the VPK.
set -e
PAD=../src/pad.c
if [ ! -f ../CMakeLists.txt ]; then echo "Run this from the build folder inside the project (the folder that holds CMakeLists.txt is one level up)"; exit 1; fi
cmake .. > /dev/null      # re-read the project, so a renamed project or new file is always picked up
NEED=3200

for attempt in 1 2 3 4; do
  make leftseat > /dev/null
  read -r V0 M0 V1 < <(arm-vita-eabi-readelf -lW leftseat | awk '$1=="LOAD"{n++; if(n==1){v0=$3; m0=$6} if(n==2){v1=$3}} END{print v0, m0, v1}')
  if [ -z "$V1" ]; then echo "Could not read the program headers"; arm-vita-eabi-readelf -lW leftseat; exit 1; fi
  GAP=$(( V1 - (V0 + M0) ))
  echo "free space after the code segment: $GAP bytes (need about $NEED)"
  if [ "$GAP" -ge "$NEED" ]; then break; fi
  CUR=$(grep -o 'leftseat_pad\[[0-9]*\]' "$PAD" | grep -o '[0-9]*')
  NEW=$(( CUR + ((GAP + 64 + 15) / 16) * 16 ))
  echo "growing padding from $CUR to $NEW bytes"
  sed -i "s/leftseat_pad\[[0-9]*\]/leftseat_pad[$NEW]/" "$PAD"
done

make
