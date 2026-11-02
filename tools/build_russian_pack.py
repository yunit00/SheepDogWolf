#!/usr/bin/env python3
"""Create a local Russian pack from an exported PC font and translated text.

The input font contains original button pictures; it is never checked into Git.
Requires Pillow and a locally licensed TrueType font with Cyrillic coverage.
"""
import argparse
import shutil
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser()
parser.add_argument("export", type=Path)
parser.add_argument("output", type=Path)
parser.add_argument("--font", type=Path, required=True)
args = parser.parse_args()
mapping = list((args.export / "font/characters.txt").read_text().replace("\n", "").replace("\r", ""))
assert len(mapping) == 224
seen, slots = set(), []
for i, char in enumerate(mapping):
    # Keep every ASCII cell intact: legacy formatted text and key names use
    # these byte values directly. In particular '$' and '%' are parser tokens.
    if i + 0x20 >= 0x80 and char != " " and char in seen:
        slots.append(i)
    seen.add(char)
alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"
assert len(slots) >= len(alphabet)
for i, char in zip(slots, alphabet):
    mapping[i] = char
source = Path(__file__).resolve().parents[1] / "localization/ru"
args.output.mkdir(parents=True, exist_ok=True)
# Source scripts, review metadata and reference pictures stay in the repository.
# Only runtime files belong in the distributable language pack.
for relative in ("language.txt", "wheel.png", "channel/sign.png"):
    item = source / relative
    if item.exists():
        target = args.output / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(item, target)
for relative in ("text", "voices"):
    item = source / relative
    if item.is_dir(): shutil.copytree(item, args.output / relative, dirs_exist_ok=True)
fontdir = args.output / "font"
fontdir.mkdir(exist_ok=True)
# The atlas is a technical grid. Preserve original controller icons in their
# reserved cells; rasterize text glyphs at 4x for clear text on Retina displays.
cell = 64
atlas = Image.open(args.export / "font/font.png").convert("RGBA").resize((1024, 896), Image.Resampling.NEAREST)
font = ImageFont.truetype(str(args.font), 51)
for i, char in enumerate(mapping):
    if char == " ":
        continue
    tile = Image.new("RGBA", (cell, cell))
    draw = ImageDraw.Draw(tile)
    box = draw.textbbox((0, 0), char, font=font, anchor="ls", stroke_width=1)
    draw.text(((cell - (box[2] - box[0])) / 2 - box[0], 54), char,
              font=font, anchor="ls", fill=(190, 190, 190, 255),
              stroke_width=1, stroke_fill=(190, 190, 190, 255))
    atlas.paste(tile, ((i % 16) * cell, (i // 16) * cell))
atlas.save(fontdir / "font.png")
(fontdir / "characters.txt").write_text("\n".join("".join(mapping[i:i+16]) for i in range(0,224,16)) + "\n")
print(f"Created Russian text/font pack: {args.output}")
