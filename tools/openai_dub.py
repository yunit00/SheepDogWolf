#!/usr/bin/env python3
"""Generate one complete utterance per file with a single approved voice style."""
import argparse, concurrent.futures, hashlib, json
from pathlib import Path
from dub_batch import units
from openai_voice import synthesize


def generate(unit,output,style):
    target=output/unit['file'];script=target.with_suffix('.script.json')
    seconds=max(.7,unit['end']-unit['start'])
    spec={**style,'text':unit['text'],'unit':unit,
          'instructions':style['instructions']+f' Длительность самой реплики — примерно {seconds:.1f} секунды. '
            'Произнеси весь текст, без пропусков. Короткая пауза перед первым и после последнего слова.'}
    if script.exists() and json.loads(script.read_text())!=spec:
        raise ValueError('Source/style changed; choose another output directory: '+unit['file'])
    script.write_text(json.dumps(spec,ensure_ascii=False,indent=2)+'\n')
    if not target.exists():synthesize(script,target)
    meta=json.loads(target.with_suffix('.generation.json').read_text())
    digest=hashlib.sha256(target.read_bytes()).hexdigest()
    if meta['sha256']!=digest:raise ValueError('Generated audio changed '+unit['file'])
    return {**unit,'provider':'OpenAI','voice':spec['voice'],'model':meta['model'],
            'seconds':meta['seconds'],'audio_sha256':digest,'qa':'requires independent file QA'}


def main():
    p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('style',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--speaker',default='Daffy Duck');p.add_argument('--workers',type=int,default=2);a=p.parse_args()
    a.output.mkdir(parents=True,exist_ok=True);style=json.loads(a.style.read_text());style.pop('text',None)
    plan=[u for u in units(a.review) if u['speaker']==a.speaker];done=[];errors=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
        jobs={pool.submit(generate,u,a.output,style):u for u in plan}
        for j in concurrent.futures.as_completed(jobs):
            if j.cancelled():continue
            try:done.append(j.result());print('recorded',len(done),'/',len(plan),jobs[j]['file'],flush=True)
            except Exception as e:
                errors.append({'file':jobs[j]['file'],'error':str(e)});print('ERROR',jobs[j]['file'],str(e),flush=True)
                if '429' in str(e) or '401' in str(e) or '402' in str(e):
                    for pending in jobs:pending.cancel()
            (a.output/'manifest.json').write_text(json.dumps(sorted(done,key=lambda x:x['file']),ensure_ascii=False,indent=2)+'\n')
    (a.output/'errors.json').write_text(json.dumps(errors,ensure_ascii=False,indent=2)+'\n')
    if len(done)!=len(plan):raise SystemExit(1)


if __name__=='__main__':main()
