#!/usr/bin/env python3
"""Bounded native-renderer startup smoke test; not a gameplay playthrough."""
import argparse
import json
import os
import signal
import subprocess
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument("game", type=Path)
p.add_argument("data", type=Path)
p.add_argument("output", type=Path)
p.add_argument("--seconds", type=int, default=8)
p.add_argument("--levels", type=int, nargs="+", default=[-1, -2, -3, *range(18)], help="Scene IDs; default is all 21 scenes")
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=True)
saves = a.output / "saves"
saves.mkdir(exist_ok=True)
results = []
for level in a.levels:
    log = a.output / f"scene-{level}.log"
    with log.open("w") as f:
        child = subprocess.Popen([str(a.game.resolve()), "--renderer=metal",
            f"--game-dir={a.data.resolve()}", "--width=960", "--height=600",
            "--scale=100", "--msaa=4", "--controller=keyboard", f"--level={level}"],
            stdout=f, stderr=f, env={**os.environ, "SDW_SAVE_DIR": str(saves.resolve())})
        alive = False
        try:
            child.wait(timeout=a.seconds)
        except subprocess.TimeoutExpired:
            alive = True
            child.send_signal(signal.SIGINT)
            try:
                child.wait(timeout=5)
            except subprocess.TimeoutExpired:
                child.kill()
                child.wait()
    text = log.read_text(errors="replace")
    result = {"level": level, "alive_at_deadline": alive, "exit": child.returncode,
              "drew_120_frames": "first 120 frames" in text,
              "loaded": "Scene load succeeded" in text,
              "log": log.name}
    result["pass"] = alive and result["drew_120_frames"] and result["loaded"] and child.returncode == 0
    results.append(result)
    print(json.dumps(result), flush=True)
    (a.output / "results.json").write_text(json.dumps(results, indent=2))
raise SystemExit(0 if all(r["pass"] for r in results) else 1)
