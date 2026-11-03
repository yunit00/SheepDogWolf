#!/usr/bin/env python3
"""Create a Finder desktop shortcut to an existing personal app, preserving other files."""
import argparse
from pathlib import Path
import plistlib

parser = argparse.ArgumentParser()
parser.add_argument('app', type=Path)
parser.add_argument('--desktop', type=Path, default=Path.home() / 'Desktop')
args = parser.parse_args()
app = args.app.resolve(strict=True)
if app.suffix != '.app':
    parser.error('Expected a macOS .app bundle')
with (app / 'Contents/Info.plist').open('rb') as handle:
    info = plistlib.load(handle)
if info.get('CFBundleIdentifier') != 'io.github.yunit00.sheepdogwolf':
    parser.error('Expected the personal SheepDogWolf bundle')
link = args.desktop / 'Играть в SheepDogWolf.app'
if link.exists() or link.is_symlink():
    if not link.is_symlink() or link.resolve() != app:
        parser.error('Existing desktop item preserved. Move it explicitly before creating a different shortcut.')
else:
    link.symlink_to(app, target_is_directory=True)
print(f'{link} -> {app}')
