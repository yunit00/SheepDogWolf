#!/usr/bin/env python3
"""Batch retexturing: export every image of the game's texture pages, upscale them with any tool, import them back.

Each level's .DAV holds its texture pages (256 x 256, 16-bit) and a rectangle table that cuts them into images
{x, w, y, h, page}; the 3D models and the 2D code point at those rectangles. This tool:

  export   cuts every page into its images and writes each one as a PNG with a margin of its own edge pixels around it
           (context for the upscaler, so nothing of the neighbouring image leaks in). An image used by several levels
           is written once (same pixels = same file). The fonts are cut into their characters, one image each, named
           font_<game|debug>_<code>.png (the hex character code: font_game_5f.png is the action button's icon;
           PlayStation levels' are font_game_5f_psx.png, with the console's button symbols; a level whose font differs
           from the others gets its own, font_game_5f_<Level>.png). Also writes the original pages, for
           reference.

  import   reads the (upscaled) images back, crops their margins and pastes each at its place on a page scaled by the
           same factor, over the original page enlarged (for the few texels no image covers). The pages are written
           as <game>/retexture/<Level>/page_NN.png, which the game uses in place of the .DAV's pages
           (PolyBatcher::LoadTexturePages): full 32-bit colour, any whole multiple of 256.

PlayStation levels: a level whose .DAV is the PlayStation's is converted first, by psxdav_convert (built with the
game: the same conversion the game makes when it loads the level), into WORK/converted/<Level>.DAV, and its pages go
to <game>/retexture-psx/<Level>/ instead: the converted pages are not the PC's, and the game reads them from there.

Upscaled images may be any size: each page takes the scale most of its images have (or --scale), and every image is
resized to fit it. Images left as they are keep the original. A page none of whose images changed is not written.

Alpha: the PNGs use the usual alpha (opacity: 0 = transparent); the game's own is the other way round, and the game
flips it back when it loads an override.

Why per image and not per page: an upscaler blends each pixel with its neighbours, so upscaling a whole page smears
the images that touch on it into each other. Upscaled one by one and stitched back, nothing bleeds: the game keeps
every UV half an original texel inside its image, which is a full texel or more at 2x and above.

Usage (needs Pillow: pip install pillow):
  python3 tools/retexture.py export WORK                 # every level, into WORK/images (+ WORK/pages)
  python3 tools/retexture.py export WORK --levels Lvl-01 Wheel --margin 8
      ... upscale WORK/images/*.png in place (same file names; Real-ESRGAN, an image editor, anything) ...
  python3 tools/retexture.py import WORK                 # writes <game>/retexture/<Level>/page_NN.png
  python3 tools/retexture.py import WORK --scale 4 --out SOME/retexture
  --game DIR: the game's data folder (default: the disc folder next to this repository).
  --psxdav FILE: the psxdav_convert program (default: the one in a cmake-build-* or build folder of this repository).
"""
import argparse
import array
import hashlib
import json
import shutil
import struct
import subprocess
import sys
from collections import Counter, defaultdict
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    sys.exit("retexture.py needs Pillow: pip install pillow")

sys.path.insert(0, str(Path(__file__).resolve().parent))
import war_meshes  # noqa: E402  the .DAV / .WAR readers

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_GAME = ROOT / "Sheep, Dog 'n' Wolf (PAL Version)"
MANIFEST = "manifest.json"
PAGE_FORMATS = {0: "565", 1: "1555", 2: "4444"}  # format & 3 (TEXFMT_PIXEL_MASK)
# the fonts, by .DAV id list (Font_LoadFromRes): exported one character per image. Every font has 16 characters a row
# from 0x20 (div(c - 0x20, 16), src/engine/text.cpp), so a cell is the sheet's width / 16 wide and 16 high.
FONTS = {0x24: "game", 0x1D: "debug"}  # DAV_IDI_IGLTYPO_, DAV_IDI_IGLFONTE
FONT_CELL_H = 16


# ---- reading the game's files ----

def levels(game, only=None):
    """{level folder name: (.dav path, .war path or None)} for every level folder holding a .DAV."""
    out = {}
    for folder in sorted((game / "Levels").iterdir() if (game / "Levels").is_dir() else []):
        if not folder.is_dir() or (only and folder.name.lower() not in {o.lower() for o in only}):
            continue
        files = {f.suffix.lower(): f for f in folder.iterdir()}
        if ".dav" in files:
            out[folder.name] = (files[".dav"], files.get(".war"))
    return out


def _texel_table(fmt):
    """RGBA bytes for every 16-bit texel value of a page format, with the usual alpha (opacity)."""
    table = []
    for c in range(65536):
        if fmt == 1:  # ARGB1555: the bit set = transparent
            r, g, b, t = (c >> 10 & 31) * 255 // 31, (c >> 5 & 31) * 255 // 31, (c & 31) * 255 // 31, 255 * (c >> 15)
        elif fmt == 2:  # ARGB4444
            r, g, b, t = (c >> 8 & 15) * 17, (c >> 4 & 15) * 17, (c & 15) * 17, (c >> 12 & 15) * 17
        else:  # RGB565, no alpha
            r, g, b, t = (c >> 11 & 31) * 255 // 31, (c >> 5 & 63) * 255 // 63, (c & 31) * 255 // 31, 0
        table.append(bytes((r, g, b, 255 - t)))
    return table


_TABLES = {}


def is_psx_dav(path):
    """whether a .DAV is the PlayStation's (psx_dav.cpp: "V2.6" at 4, the material section's label at 0x30)"""
    with open(path, "rb") as f:
        head = f.read(0x44)
    return head[4:8] == b"V2.6" and head[0x30:0x44] == b"----Section material"


def find_converter(given):
    if given:
        return given
    names = ("psxdav_convert", "psxdav_convert.exe")
    for folder in sorted(ROOT.glob("cmake-build-*")) + [ROOT / "build", ROOT / "build" / "Release", ROOT / "build" / "Debug"]:
        for n in names:
            if (folder / n).is_file():
                return folder / n
    found = shutil.which("psxdav_convert")
    if found:
        return Path(found)
    sys.exit("a PlayStation .DAV needs psxdav_convert: build the game (target psxdav_convert) or pass --psxdav")


def page_file(name, dav_path, work, converter):
    """the VDX7 file to read a level's pages from, and whether it was converted from the PlayStation's"""
    if not is_psx_dav(dav_path):
        return dav_path, False
    out = work / "converted" / ("%s.DAV" % name)
    out.parent.mkdir(parents=True, exist_ok=True)
    r = subprocess.run([str(find_converter(converter)), str(dav_path), str(out)], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit("%s: psxdav_convert failed: %s" % (dav_path, r.stderr.strip()))
    return out, True


def decode_pages(dav_path):
    """The .DAV's pages as RGBA images, and its rectangle table (x, w, y, h, page)."""
    dav = war_meshes.read_dav(dav_path)
    data = dav_path.read_bytes()
    pages = []
    for p in dav["pages"]:
        fmt = p["format"] & 3
        table = _TABLES.setdefault(fmt, _texel_table(fmt))
        texels = array.array("H")
        texels.frombytes(data[p["offset"] + 10:p["offset"] + 10 + 2 * p["w"] * p["h"]])
        if sys.byteorder != "little":
            texels.byteswap()
        pages.append(Image.frombytes("RGBA", (p["w"], p["h"]), b"".join(table[t] for t in texels)))
    return dav, pages


def model_usage(war_path, dav):
    """{rectangle index: number of 3D models drawing with it}."""
    usage = Counter()
    if not war_path:
        return usage
    d = war_path.read_bytes()
    count, = struct.unpack_from("<I", d, 0x0C)
    for e in struct.unpack_from("<%dI" % count, d, 0x10):
        if (e >> 24) & 0xBF not in war_meshes.MESH_TYPES:
            continue
        try:
            mesh = war_meshes.read_mesh(d, e & 0xFFFFFF)
        except (ValueError, struct.error):
            continue
        for rect in {dav["entries"][x] for en in mesh["entries"] for x in en["ids"] if x < len(dav["entries"])}:
            usage[rect] += 1
    return usage


def font_cells(dav_path, dav):
    """{rectangle index: (font name, [(character code, x, y, w, h), ...])}: the character cells of each font sheet."""
    out = {}
    lists = war_meshes.read_dav_idlists(dav_path)
    for res, name in FONTS.items():
        for rect in lists.get(res, []):
            x, w, y, h, page = dav["rects"][rect]
            cw = w // 16
            rows = min(h // FONT_CELL_H, (0x100 - 0x20) // 16)
            out[rect] = (name, [(0x20 + r * 16 + c, x + c * cw, y + r * FONT_CELL_H, cw, FONT_CELL_H)
                                for r in range(rows) for c in range(16)])
    return out


# ---- images ----

def pad_edges(img, m):
    """img with m pixels of its own edge pixels repeated around it."""
    if m <= 0:
        return img.copy()
    w, h = img.size
    out = Image.new("RGBA", (w + 2 * m, h + 2 * m))
    out.paste(img, (m, m))
    out.paste(img.crop((0, 0, w, 1)).resize((w, m)), (m, 0))
    out.paste(img.crop((0, h - 1, w, h)).resize((w, m)), (m, h + m))
    out.paste(img.crop((0, 0, 1, h)).resize((m, h)), (0, m))
    out.paste(img.crop((w - 1, 0, w, h)).resize((m, h)), (w + m, m))
    for (sx, sy), (dx, dy) in (((0, 0), (0, 0)), ((w - 1, 0), (w + m, 0)), ((0, h - 1), (0, h + m)),
                               ((w - 1, h - 1), (w + m, h + m))):
        out.paste(Image.new("RGBA", (m, m), img.getpixel((sx, sy))), (dx, dy))
    return out


# ---- export ----

def export(game, work, only, margin, converter):
    images_dir, pages_dir = work / "images", work / "pages"
    images_dir.mkdir(parents=True, exist_ok=True)
    manifest = {"version": 1, "margin": margin, "game": str(game), "levels": {}, "files": {}}
    by_hash = {}
    found = levels(game, only)
    if not found:
        sys.exit("no level with a .DAV under %s" % (game / "Levels"))
    for name, (dav_path, war_path) in found.items():
        dav_path, psx = page_file(name, dav_path, work, converter)
        dav, pages = decode_pages(dav_path)
        usage = model_usage(war_path, dav)
        (pages_dir / name).mkdir(parents=True, exist_ok=True)
        for i, page in enumerate(pages):
            page.save(pages_dir / name / ("page_%02d.png" % i))
        level = {"pages": [{"w": p["w"], "h": p["h"], "format": p["format"]} for p in dav["pages"]], "images": [],
                 "psx": psx}
        fonts = font_cells(dav_path, dav)
        for index, (x, w, y, h, page) in enumerate(dav["rects"]):
            if page >= len(pages) or w == 0 or h == 0 or x + w > pages[page].width or y + h > pages[page].height:
                continue  # the four blank pages the game adds, or an empty record
            if index in fonts:  # a font: one image per character, named by font and code
                font, cells = fonts[index]
                pieces = [("font_%s_%02x%s" % (font, code, "_psx" if psx else ""), {"glyph": code}, cx, cy, cw, ch)
                          for code, cx, cy, cw, ch in cells]
            else:
                pieces = [("%s_p%02d_r%04d" % (name, page, index), {}, x, y, w, h)]
            for stem, extra, px, py, pw, ph in pieces:
                crop = pages[page].crop((px, py, px + pw, py + ph))
                if extra and not crop.getchannel("A").getbbox():
                    continue  # an empty character cell
                # a font character is never shared between the PC and the PlayStation, so each keeps its own names
                tag = b"psx" if extra and psx else b""
                key = hashlib.sha1(crop.tobytes() + struct.pack("<HH", pw, ph) + tag).hexdigest()
                if key not in by_hash:
                    file = stem + ".png"
                    if file in manifest["files"]:  # the same character drawn differently in another level
                        file = "%s_%s.png" % (stem, name)
                    pad_edges(crop, margin).save(images_dir / file)
                    by_hash[key] = file
                    manifest["files"][file] = {"w": pw, "h": ph, "uses": 0}
                file = by_hash[key]
                manifest["files"][file]["uses"] += 1
                level["images"].append(dict({"rect": index, "page": page, "x": px, "y": py, "w": pw, "h": ph,
                                             "file": file, "models": usage.get(index, 0)}, **extra))
        manifest["levels"][name] = level
        print("%-8s %2d pages, %4d images%s" % (name, len(pages), len(level["images"]), " (PlayStation)" if psx else ""))
    (work / MANIFEST).write_text(json.dumps(manifest, indent=1))
    total = sum(len(lv["images"]) for lv in manifest["levels"].values())
    print("%d images in %d levels, %d unique files in %s (margin %d)" % (
        total, len(manifest["levels"]), len(manifest["files"]), images_dir, margin))
    print("Upscale the files in place (keep their names), then: python3 tools/retexture.py import %s" % work)


# ---- import ----

def load_image(path, w, h, margin):
    """An exported image as it is now: its scale over the original, and the image without its margin."""
    img = Image.open(path)
    has_alpha = img.mode in ("RGBA", "LA") or "transparency" in img.info
    img = img.convert("RGBA")
    scale = img.width / (w + 2 * margin)
    m = round(margin * scale)
    if img.width - 2 * m <= 0 or img.height - 2 * m <= 0:
        raise ValueError("%s: smaller than its margin" % path.name)
    return scale, img.crop((m, m, img.width - m, img.height - m)), has_alpha


def import_(game, work, out, only, forced_scale, converter):
    manifest = json.loads((work / MANIFEST).read_text())
    margin = manifest["margin"]
    found = levels(game, only)
    loaded = {}  # file: (scale, image, has_alpha) or None when missing / unreadable
    for file, info in manifest["files"].items():
        path = work / "images" / file
        try:
            loaded[file] = load_image(path, info["w"], info["h"], margin) if path.exists() else None
        except (OSError, ValueError) as e:
            print("skipping %s: %s" % (file, e))
            loaded[file] = None
    written = 0
    for name, level in manifest["levels"].items():
        if name not in found:
            if not only:
                print("%s: not in %s, skipped" % (name, game / "Levels"))
            continue
        dav_path, psx = page_file(name, found[name][0], work, converter)
        if psx != level.get("psx", False):
            print("%s: the .DAV is not the version (PC / PlayStation) that was exported, skipped" % name)
            continue
        _, originals = decode_pages(dav_path)
        root = out.parent / (out.name + "-psx") if psx else out
        by_page = defaultdict(list)
        for im in level["images"]:
            by_page[im["page"]].append(im)
        for page, images in sorted(by_page.items()):
            original = originals[page]
            scales = [round(loaded[im["file"]][0]) for im in images if loaded[im["file"]]]
            scale = forced_scale or (Counter(scales).most_common(1)[0][0] if scales else 1)
            scale = max(1, int(scale))
            changed = scale > 1
            size = (original.width * scale, original.height * scale)
            result = original.resize(size, Image.LANCZOS)  # under the images: the texels no image covers
            for im in images:
                x, y, w, h = (im[k] * scale for k in ("x", "y", "w", "h"))
                src = original.crop((im["x"], im["y"], im["x"] + im["w"], im["y"] + im["h"]))
                entry = loaded[im["file"]]
                if entry is None:
                    piece = src.resize((w, h), Image.LANCZOS)
                else:
                    _, img, has_alpha = entry
                    piece = img if img.size == (w, h) else img.resize((w, h), Image.LANCZOS)
                    if not has_alpha:  # the upscaler dropped the alpha: the original's, enlarged
                        piece.putalpha(src.getchannel("A").resize((w, h), Image.LANCZOS))
                    if scale == 1 and piece.tobytes() != src.tobytes():
                        changed = True
                result.paste(piece, (x, y))
            if not changed:
                continue
            dest = root / name / ("page_%02d.png" % page)
            dest.parent.mkdir(parents=True, exist_ok=True)
            result.save(dest)
            written += 1
            print("%s page %02d: %dx (%dx%d)" % (name, page, scale, size[0], size[1]))
    print("%d pages written to %s (PlayStation levels: %s-psx)" % (written, out, out))


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    sub = ap.add_subparsers(dest="command", required=True)
    for cmd in ("export", "import"):
        p = sub.add_parser(cmd)
        p.add_argument("work", type=Path, help="the folder of the exported images")
        p.add_argument("--game", type=Path, default=DEFAULT_GAME, help="the game's data folder")
        p.add_argument("--levels", nargs="*", help="only these level folders (Lvl-01, Wheel...)")
        p.add_argument("--psxdav", type=Path, help="the psxdav_convert program, for PlayStation levels")
        if cmd == "export":
            p.add_argument("--margin", type=int, default=8, help="edge pixels repeated around each image (default 8)")
        else:
            p.add_argument("--scale", type=int, help="the pages' scale (default: what most of each page's images have)")
            p.add_argument("--out", type=Path, help="where the pages go (default: <game>/retexture)")
    a = ap.parse_args(argv)
    if a.command == "export":
        export(a.game, a.work, a.levels, a.margin, a.psxdav)
    else:
        import_(a.game, a.work, a.out or a.game / "retexture", a.levels, a.scale, a.psxdav)


if __name__ == "__main__":
    main(sys.argv[1:])
