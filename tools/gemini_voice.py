#!/usr/bin/env python3
"""Resumable Gemini audio work. Credentials are read only from the environment.

Analysis preserves original clip names. Generated takes remain separate until
their transcript and timing have been checked; an API response is not approval.
"""
import argparse
import base64
import json
import os
from pathlib import Path
import ssl
import urllib.error
import urllib.request
import wave

ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/"


def request(path, payload):
    key = os.environ.get("GEMINI_API_KEY")
    if not key:
        raise RuntimeError("Set GEMINI_API_KEY locally; never put it in a source file")
    context = ssl.create_default_context(cafile="/etc/ssl/cert.pem")
    req = urllib.request.Request(ENDPOINT + path,
        data=json.dumps(payload).encode(), headers={
            "x-goog-api-key": key, "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, context=context, timeout=240) as response:
            return json.load(response)
    except urllib.error.HTTPError as error:
        # Google error messages contain quota names, not the request credential.
        detail = error.read().decode(errors="replace").replace(key, "[redacted]")
        raise RuntimeError(f"Gemini HTTP {error.code}: {detail[:3000]}") from None


def analyze(files, output):
    parts = [{"text": "Transcribe these original game voice clips exactly in English. "
        "For EACH named file return {file, segments:[{speaker, text}], non_speech, notes}. "
        "Use consistent character names where recognizable: Daffy Duck, Porky Pig, Bugs Bunny, "
        "Elmer Fudd, Yosemite Sam, Marvin the Martian, Ralph Wolf, narrator; otherwise unknown. "
        "Preserve all words, stutters, laughter, and speaker turns; do not invent speech in silent clips. "
        "The file label immediately precedes that file's audio. Output a JSON array only."}]
    for path in files:
        parts += [{"text": "FILE: " + path.name}, {"inlineData": {
            "mimeType": "audio/wav", "data": base64.b64encode(path.read_bytes()).decode()}}]
    result = request("models/gemini-3.8-flash:generateContent", {
        "contents": [{"parts": parts}], "generationConfig": {
            "responseMimeType": "application/json", "temperature": 0.1,
            "maxOutputTokens": 24000}})
    candidate = result["candidates"][0]
    if candidate.get("finishReason") != "STOP":
        raise RuntimeError("Incomplete transcription: " + str(candidate.get("finishReason")))
    rows = json.loads("".join(p.get("text", "") for p in candidate["content"]["parts"]))
    if len(rows) != len(files) or {r["file"] for r in rows} != {p.name for p in files}:
        raise RuntimeError("Transcription does not cover the input files exactly")
    output.write_text(json.dumps(rows, ensure_ascii=False, indent=2) + "\n")


def synthesize(script, output, model, voice):
    specification = json.loads(script.read_text())
    result = request(f"models/{model}:generateContent", {
        "contents": [{"parts": [{"text": specification["prompt"]}]}],
        "generationConfig": {"responseModalities": ["AUDIO"],
            "speechConfig": {"voiceConfig": {"prebuiltVoiceConfig": {"voiceName": voice}}}}})
    candidate = result["candidates"][0]
    if candidate.get("finishReason") != "STOP":
        raise RuntimeError("Incomplete speech: " + str(candidate.get("finishReason")))
    chunks = [p["inlineData"] for p in candidate["content"]["parts"] if "inlineData" in p]
    if not chunks:
        raise RuntimeError("No audio returned")
    pcm = b"".join(base64.b64decode(c["data"]) for c in chunks)
    if any("audio/l16" not in c["mimeType"].lower() or "rate=24000" not in c["mimeType"].replace(" ", "") for c in chunks):
        raise RuntimeError("Unexpected audio format: " + str([c["mimeType"] for c in chunks]))
    if not pcm or len(pcm) % 2:
        raise RuntimeError("Invalid PCM length")
    with wave.open(str(output), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(24000)
        wav.writeframes(pcm)
    output.with_suffix(".generation.json").write_text(json.dumps({
        "model": model, "voice": voice, "seconds": len(pcm)/48000,
        "script": specification, "usage": result.get("usageMetadata"),
        "status": "unreviewed"}, ensure_ascii=False, indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    a = sub.add_parser("analyze")
    a.add_argument("source", type=Path)
    a.add_argument("output", type=Path)
    a.add_argument("--batch-size", type=int, default=8)
    a.add_argument("--start", type=int, default=0)
    a.add_argument("--count", type=int, default=999)
    s = sub.add_parser("synthesize")
    s.add_argument("script", type=Path)
    s.add_argument("output", type=Path)
    s.add_argument("--model", default="gemini-3.1-flash-tts-preview")
    s.add_argument("--voice", default="Charon")
    args = parser.parse_args()
    if args.command == "analyze":
        args.output.mkdir(parents=True, exist_ok=True)
        files = sorted(args.source.glob("*.wav"))
        for index in range(args.start, min(len(files), args.start + args.count), args.batch_size):
            output = args.output / f"batch-{index:03}.json"
            if output.exists():
                print("Cached", output.name, flush=True)
                continue
            batch = files[index:min(index + args.batch_size, args.start + args.count)]
            analyze(batch, output)
            print("Transcribed", index, len(batch), flush=True)
    else:
        if args.output.exists():
            parser.error("Output exists; preserve takes using a new filename")
        args.output.parent.mkdir(parents=True, exist_ok=True)
        synthesize(args.script, args.output, args.model, args.voice)
        print("Generated", args.output)


if __name__ == "__main__":
    main()
