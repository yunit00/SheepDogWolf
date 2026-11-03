#!/usr/bin/env python3
"""Record selected timing/quality retakes, preserving every old take and review."""
import argparse, concurrent.futures, hashlib, json
from pathlib import Path
from openai_voice import synthesize


def record(row, style, output):
    unit=row['unit']; target=output/unit['file']
    spec={**style, 'text':unit['text'], 'unit':unit,
          'speed':1.2 if row['seconds']/row['room']>1.24 else style.get('speed',1.05),
          'instructions':style['instructions']+
          f" This is a single short game utterance, not a conversation. Fit ALL spoken words into {row['room']*.95:.2f} seconds. "
          'Use brisk continuous speech, no dramatic pauses, no elongated vowels. Say the text EXACTLY ONCE. '
          'Do not repeat short exclamations or add introductory sounds. Dry isolated voice, NO music, sound effects or background. '
          'Leave a brief quiet lead-in and tail; pronounce the first and final consonants completely.'}
    script=target.with_suffix('.script.json')
    if script.exists() and json.loads(script.read_text())!=spec:raise ValueError('Changed retake spec')
    script.write_text(json.dumps(spec,ensure_ascii=False,indent=2)+'\n')
    if not target.exists():synthesize(script,target)
    meta=json.loads(target.with_suffix('.generation.json').read_text())
    digest=hashlib.sha256(target.read_bytes()).hexdigest()
    if meta['sha256']!=digest:raise ValueError('Audio hash mismatch')
    return {**unit,'provider':'OpenAI','voice':spec['voice'],'model':meta['model'],
            'seconds':meta['seconds'],'audio_sha256':digest,'qa':'requires independent file QA'}


def main():
    p=argparse.ArgumentParser();p.add_argument('plan',type=Path);p.add_argument('style',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--workers',type=int,default=2);a=p.parse_args()
    a.output.mkdir(parents=True,exist_ok=True);style=json.loads(a.style.read_text());style.pop('text',None)
    done=[];errors=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        jobs={pool.submit(record,r,style,a.output):r for r in json.loads(a.plan.read_text())}
        for job in concurrent.futures.as_completed(jobs):
            if job.cancelled():continue
            try:done.append(job.result());print('recorded',len(done),jobs[job]['unit']['file'],flush=True)
            except Exception as e:
                errors.append({'file':jobs[job]['unit']['file'],'error':str(e)});print('ERROR',str(e),flush=True)
                if any(code in str(e) for code in ('429','401','402')):
                    for pending in jobs:pending.cancel()
            (a.output/'manifest.json').write_text(json.dumps(sorted(done,key=lambda x:x['file']),ensure_ascii=False,indent=2)+'\n')
    (a.output/'errors.json').write_text(json.dumps(errors,ensure_ascii=False,indent=2)+'\n')
    if errors:raise SystemExit(1)

if __name__=='__main__':main()
