#!/bin/bash
# extract_frames.sh - turns the clips in assets/*.mp4 into numbered pictures (assets/frames/<clip>/001.jpg, 002.jpg, ...)
#
# SDL2 cannot play .mp4 by itself, so the game shows each clip as a picture sequence instead:
# it picks the picture that matches how many seconds have passed (see clip.cpp).
# Needs ffmpeg. Run it once from the project folder (or again if you re-render a clip):
#     tools/extract_frames.sh
# The pictures are 12 per second, 640 pixels wide, JPEG quality 5 (about 15 KB each).

set -e
cd "$(dirname "$0")/.."
FFMPEG=$(command -v ffmpeg || echo "$HOME/bin/ffmpeg")
[ -x "$FFMPEG" ] || { echo "ffmpeg not found"; exit 1; }

for video in assets/*.mp4; do
    name=$(basename "$video" .mp4)
    out="assets/frames/$name"
    rm -rf "$out"
    mkdir -p "$out"
    "$FFMPEG" -v error -y -i "$video" -vf "fps=12,scale=640:-1" -q:v 5 "$out/%03d.jpg"
    echo "$name: $(ls "$out" | wc -l | tr -d ' ') frames"
done
du -sh assets/frames
