#!/usr/bin/env python3
"""Assemble a personal native app. Game resources remain outside the repository."""
import argparse
import hashlib
import json
import plistlib
import shutil
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
for name in ("app", "data", "sdl", "imgui", "output"):
    p.add_argument("--" + name, required=True, type=Path)
a = p.parse_args()
root = Path(__file__).resolve().parents[1]
if a.output.exists():
    p.error("Output already exists; use a new path to preserve earlier builds")
allowed = {".wav", ".txt", ".mlt", ".dav", ".war", ".snd", ".sdw", ".bvs",
           ".bsc", ".h", ".bsv", ".bsm", ".textindex", ".soundindex", ".png"}
manifest = []
for item in sorted(a.data.rglob("*")):
    if item.is_symlink():
        p.error(f"Data symlinks are not allowed: {item}")
    if not item.is_file():
        continue
    if item.suffix.lower() not in allowed:
        p.error(f"Unexpected data file: {item}")
    content = item.read_bytes()
    if content[:2] == b"MZ":
        p.error(f"Windows executable found in game data: {item}")
    manifest.append({"path": item.relative_to(a.data).as_posix(), "bytes": len(content),
                     "sha256": hashlib.sha256(content).hexdigest()})
for required in ("Levels", "References", "Musics", "Voices", "Languages/01-Russian"):
    if not (a.data / required).is_dir():
        p.error(f"Missing game folder: {required}")
shutil.copytree(a.app, a.output, symlinks=True)
contents = a.output / "Contents"
resources = contents / "Resources"
shutil.copytree(a.data, resources / "GameData")
with (contents / "Info.plist").open("rb") as f:
    info = plistlib.load(f)
old_exe = contents / "MacOS" / info["CFBundleExecutable"]
new_exe = old_exe.with_name("SheepDogWolf")
old_exe.rename(new_exe)
info.update(CFBundleExecutable="SheepDogWolf", CFBundleName="SheepDogWolf",
            CFBundleDisplayName="SheepDogWolf", CFBundleShortVersionString="1.1.3",
            CFBundleVersion="1.1.3.1", LSMinimumSystemVersion="13.0",
            CFBundleIdentifier="io.github.yunit00.sheepdogwolf")
with (contents / "Info.plist").open("wb") as f:
    plistlib.dump(info, f)
for lang in ("en", "ru"):
    folder = resources / (lang + ".lproj")
    folder.mkdir(exist_ok=True)
    (folder / "InfoPlist.strings").write_text('CFBundleDisplayName = "SheepDogWolf";\nCFBundleName = "SheepDogWolf";\n')
licenses = resources / "Licenses"
licenses.mkdir(exist_ok=True)
for source, name in [(root / "LICENSE", "Engine-MIT.txt"), (root / "NOTICE.md", "NOTICE.md"),
                     (root / "res/credits.txt", "Credits.txt"),
                     (root / "licenses/Roboto-Apache-2.0.txt", "Roboto-Apache-2.0.txt"),
                     (a.sdl / "LICENSE.txt", "SDL-zlib.txt"),
                     (a.sdl / "src/hidapi/LICENSE-bsd.txt", "HIDAPI-BSD.txt"),
                     (a.sdl / "src/video/yuv2rgb/LICENSE", "yuv2rgb.txt"),
                     (a.imgui / "LICENSE.txt", "Dear-ImGui-MIT.txt")]:
    shutil.copy2(source, licenses / name)
(licenses / "Roboto-NOTICE.txt").write_text("Roboto Medium\nCopyright 2011 Google Inc. All Rights Reserved.\nLicensed under the Apache License, Version 2.0.\n")
(resources / "data-manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2))
for binary in (contents / "MacOS/SheepD3D", new_exe):
    subprocess.run(["codesign", "--force", "--sign", "-", str(binary)], check=True)
subprocess.run(["codesign", "--force", "--sign", "-", str(a.output)], check=True)
subprocess.run(["codesign", "--verify", "--deep", "--strict", str(a.output)], check=True)
print(f"Personal app ready: {a.output} ({len(manifest)} data files). Ad-hoc signed, not notarized.")
