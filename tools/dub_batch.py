#!/usr/bin/env python3
"""Resumable batch TTS and independent speech QA; no installation on generation alone."""
import argparse, concurrent.futures, hashlib, json, subprocess, sys
from pathlib import Path
from gemini_voice import synthesize

CAST = {
 'Daffy Duck': ('Puck', 'Male cartoon duck show host: bright medium-high nasal quacky timbre, energetic theatrical comic confidence. CLEAR natural lateral lisp on Russian с з ш ж sounds, slightly wet whistling consonants, but every word intelligible. Brisk and animated, never a calm audiobook narrator.'),
 'Marvin the Martian': ('Charon', 'Male small Martian villain: soft nasal slightly metallic timbre, precise pedantic diction, dry comic menace, restrained theatricality. Calm but fairly brisk, no digital effects.'),
 'Porky Pig': ('Achird', 'Friendly youthful male cartoon pig, rounded warm slightly nasal high-mid voice. Keep only the short stutters written in the text, do not add excessive stuttering. Helpful, expressive and clear.'),
 'Yosemite Sam': ('Algenib', 'Gruff short-tempered male cartoon cowboy, low-mid gravelly raspy booming voice, lively comic bravado. Russian speech clear, not screaming or clipping.'),
 'Elmer Fudd': ('Umbriel', 'Soft rounded nasal male cartoon hunter voice, gentle slightly comical consonants and small speech impediment, fully intelligible Russian.'),
 'Ghost A': ('Fenrir', 'Theatrical male cartoon ghost dance instructor, medium-high rasp, lively playful spooky voice, clear energetic Russian, no added echo.'),
 'Ghost B': ('Orus', 'Deeper gruff male cartoon ghost dance instructor, short impatient comic commands, clear Russian, no added echo.'),
 'Sam Sheepdog': ('Charon', 'Low warm deadpan male cartoon sheepdog, stoic and unhurried but concise, clear Russian, preserve the written blooper fragments.'),
 'Blooper voice': ('Algenib', 'Low gruff raspy male cartoon voice, slightly confused broken fragments from a studio blooper. Clear Russian fragments, no invented words, no named actor imitation.'),
 'Ralph Wolf': ('Orus', 'Low-mid male cartoon wolf, warm dry expressive reactions, concise natural Russian.')}

def units(review):
    result=[]
    for f in sorted(review.glob('review-[0-9][0-9][0-9].json')):
        for row in json.loads(f.read_text()):
            if row.get('needs_review'): continue
            spoken=[(i,s) for i,s in enumerate(row['segments']) if s.get('ru','').strip()]
            if not spoken: continue
            if any(s['speaker'] not in CAST for _,s in spoken): continue
            # Dialogue files have one uninterrupted role; cutscenes need original per-sentence timing.
            has_nonverbal=any(not s.get('ru','').strip() for s in row['segments'])
            if not row.get('separate_utterances') and not row['speech_over_music'] and not row['effects'] and not has_nonverbal and len({s['speaker'] for _,s in spoken})==1:
                result.append({'file':row['file'],'source_file':row['file'],'speaker':spoken[0][1]['speaker'],
                    'text':' '.join(s['ru'] for _,s in spoken),'start':spoken[0][1]['start'],
                    'end':spoken[-1][1]['end'],'source_seconds':row['source_seconds'],'segments':[i for i,_ in spoken]})
            else:
                for i,s in spoken:
                    result.append({'file':Path(row['file']).stem+f'__{i:02}.wav','source_file':row['file'],
                        'speaker':s['speaker'],'text':s['ru'],'start':s['start'],'end':s['end'],
                        'source_seconds':row['source_seconds'],'segments':[i]})
    return sorted(result,key=lambda u:(u['speaker'],u['source_file'],u['start']))

def prepare(review, output):
    output.mkdir(parents=True,exist_ok=True)
    existing={c['file'] for f in output.glob('*.script.json') for c in json.loads(f.read_text())['clips']}
    pending=[u for u in units(review) if u['file'] not in existing]
    groups=[];batch=[];words=0
    for u in pending:
        n=len(u['text'].split())
        if batch and (batch[0]['speaker']!=u['speaker'] or words+n>430 or len(batch)>=16):
            groups.append(batch);batch=[];words=0
        batch.append(u);words+=n
    if batch:groups.append(batch)
    for batch in groups:
        who=batch[0]['speaker']; voice,style=CAST[who]
        prompt=('Russian dubbing for a classic cartoon game. '+style+
            ' Use the SAME character voice throughout. Speak every Russian sentence exactly and completely, no missing words. '
            'Brisk clear conversational rhythm, around 3 Russian words per second. No music, no sound effects, no introduction or conclusion. '
            'Read ONLY the Russian text of each entry below. Do not say entry numbers or labels. '
            'Leave TWO SECONDS of SILENCE between entries so the editor can split recordings. Do not say any separator words. '
            'Begin and end with a short silence.\n\n'+'\n\n'.join(f'{i+1}. {u["text"]}' for i,u in enumerate(batch)))
        spec={'speaker':who,'voice':voice,'clips':batch,'prompt':prompt}
        digest=hashlib.sha256(json.dumps(spec,sort_keys=True,ensure_ascii=False).encode()).hexdigest()[:12]
        path=output/(who.split()[0].lower()+'-'+digest+'.script.json')
        path.write_text(json.dumps(spec,ensure_ascii=False,indent=2)+'\n')
        print('prepared',path.name,len(batch),'clips',sum(len(u['text'].split()) for u in batch),'words',flush=True)

def run(script):
    prefix=script.name.removesuffix('.script.json');wav=script.with_name(prefix+'.wav');qa=script.with_name(prefix+'.review.json')
    spec=json.loads(script.read_text())
    if spec.get('hold_reason'):return prefix+' held: '+spec['hold_reason']
    if not wav.exists():
        synthesize(script,wav,'gemini-3.1-flash-tts-preview',spec['voice'])
        print('generated',wav.name,flush=True)
    if not qa.exists():
        with script.with_name(prefix+'.review.log').open('w') as log:
            subprocess.run([sys.executable,str(Path(__file__).with_name('review_tts.py')),str(wav),str(script),str(qa)],stdout=log,stderr=subprocess.STDOUT,check=True)
        print('reviewed',qa.name,flush=True)
    return prefix

def main():
    p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('output',type=Path)
    p.add_argument('--run',action='store_true');p.add_argument('--workers',type=int,default=2);a=p.parse_args()
    prepare(a.review,a.output)
    if a.run:
        with concurrent.futures.ThreadPoolExecutor(max_workers=a.workers) as pool:
            jobs={pool.submit(run,f):f for f in sorted(a.output.glob('*.script.json'))}
            errors=[]
            for job in concurrent.futures.as_completed(jobs):
                try:print('complete',job.result(),flush=True)
                except Exception as e:
                    if job.cancelled():continue
                    errors.append({'script':str(jobs[job]),'error':str(e)});print('ERROR',jobs[job].name,str(e)[:1000],flush=True)
                    if '429' in str(e):
                        for pending in jobs:pending.cancel()
            (a.output/'errors.json').write_text(json.dumps(errors,ensure_ascii=False,indent=2)+'\n')
            if errors:raise SystemExit(1)
if __name__=='__main__':main()
