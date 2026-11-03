#!/usr/bin/env python3
"""Optional, explicitly authorized TTS backend. No credentials in files or logs.

Speech API documentation checked 2026-11-02:
https://developers.openai.com/api/docs/guides/text-to-speech
This request-based model is scheduled to retire 2027-01-06. Generated local
WAVs keep working; future regeneration may require an API/model migration.
"""
import argparse, hashlib, json, os, ssl, urllib.error, urllib.request, wave
from pathlib import Path


def synthesize(script, output):
    if output.exists():raise ValueError('Output exists; preserve takes under new names')
    spec=json.loads(script.read_text())
    text=spec.get('text') or '\n\n'.join(c['text'] for c in spec['clips'])
    if len(text)>4096:raise ValueError('Speech API input exceeds 4096 characters')
    payload={'model':spec.get('model','gpt-4o-mini-tts-2025-12-15'),
             'voice':spec.get('voice','cedar'),'input':text,
             'instructions':spec['instructions'],'response_format':'pcm','speed':spec.get('speed',1.0)}
    key=os.environ.get('OPENAI_API_KEY')
    if not key:raise RuntimeError('OPENAI_API_KEY is not set')
    req=urllib.request.Request('https://api.openai.com/v1/audio/speech',data=json.dumps(payload).encode(),
        headers={'Authorization':'Bearer '+key,'Content-Type':'application/json'})
    try:
        with urllib.request.urlopen(req,context=ssl.create_default_context(cafile='/etc/ssl/cert.pem'),timeout=240) as r:
            pcm=r.read();request_id=r.headers.get('x-request-id')
    except urllib.error.HTTPError as e:
        raise RuntimeError(f'OpenAI HTTP {e.code}: '+e.read().decode(errors='replace').replace(key,'[redacted]')[:1500]) from None
    if not pcm or len(pcm)%2:raise ValueError('Invalid PCM response')
    output.parent.mkdir(parents=True,exist_ok=True)
    with wave.open(str(output),'wb') as wav:
        wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(24000);wav.writeframes(pcm)
    output.with_suffix('.generation.json').write_text(json.dumps({
        'provider':'OpenAI','model':payload['model'],'voice':payload['voice'],'speed':payload['speed'],
        'seconds':len(pcm)/48000,'request_id':request_id,'script':spec,
        'sha256':hashlib.sha256(output.read_bytes()).hexdigest(),'status':'unreviewed'},ensure_ascii=False,indent=2)+'\n')


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('script',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
    synthesize(a.script,a.output);print('generated',a.output)
