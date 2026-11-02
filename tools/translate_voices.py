#!/usr/bin/env python3
"""Build a reviewable dubbing script, reusing the game's translated terminology."""
import argparse
import difflib
import json
from pathlib import Path
import re
from gemini_voice import request

p = argparse.ArgumentParser()
p.add_argument("transcripts", type=Path)
p.add_argument("english_text", type=Path)
p.add_argument("russian_text", type=Path)
p.add_argument("output", type=Path)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
rows = [r for f in sorted(a.transcripts.glob("batch-*.json")) for r in json.loads(f.read_text())]
source = json.loads(a.english_text.read_text())
ru = json.loads(a.russian_text.read_text())
def clean(s):
    return re.sub(r"[^a-z0-9 ]", "", re.sub(r"\$[^$]*\$", " ", s).lower())

for start in range(0, len(rows), 16):
    output = a.output / f"translated-{start:03}.json"
    if output.exists(): continue
    batch = []
    for row in rows[start:start+16]:
        spoken = " ".join(s["text"] for s in row["segments"])
        matches = sorted(source, key=lambda r: difflib.SequenceMatcher(None,clean(spoken),clean(r["en"])).ratio(), reverse=True)[:2]
        batch.append({**row,"reference_translations":[{"id":m["id"],"en":m["en"],"ru":ru[str(m["id"])],"at":m["at"]} for m in matches]})
    prompt = """Translate each game's spoken English clip faithfully into natural performable Russian dubbing.
Return JSON array of {file, segments:[{speaker,en,ru}], notes}. Preserve input filename, speaker order and every segment.
Do NOT invent missing speech. References are suggestions: use the existing Russian translation where it corresponds,
but never include parts of a reference not spoken in this audio clip. Remove subtitle timing/color markers.
Speak button names as actions (кнопка прыжка, кнопка действия, инвентарь), never technical tokens or dollar codes.
Names: Ralph=Ральф, Sam Sheepdog=Сэм, Daffy=Даффи, Porky=Порки, Marvin=Марвин, Instant Martians=марсиане,
goal=финиш, lettuce=салат. Preserve humor, complete meaning, no excessive wordiness; similar spoken duration to English.
Keep Porky's occasional short stutter but readable. Daffy's lisp will be performed by TTS: spell his Russian normally.
If speech seems garbled in the source, flag it in notes; do not silently invent a coherent sentence.
Data:\n""" + json.dumps(batch, ensure_ascii=False)
    result = request("models/gemini-3.8-flash:generateContent", {"contents":[{"parts":[{"text":prompt}]}],
        "generationConfig":{"responseMimeType":"application/json","temperature":0.2,"maxOutputTokens":24000}})
    c = result["candidates"][0]
    if c.get("finishReason") != "STOP": raise RuntimeError("Incomplete translation")
    translated = json.loads("".join(x.get("text","") for x in c["content"]["parts"]))
    if {r["file"] for r in translated} != {r["file"] for r in batch}: raise RuntimeError("Missing clip")
    output.write_text(json.dumps(translated,ensure_ascii=False,indent=2)+"\n")
    print(output.name, len(translated), flush=True)
