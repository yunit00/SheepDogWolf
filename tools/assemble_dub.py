#!/usr/bin/env python3
"""Build review candidates, never install an incomplete/unchecked voice pack.

Every speech event has one input and a disjoint half-open sample interval.
Original speech is removed before new speech is added. Uncertain source files,
failed clip QA, overlapping edits and excessive acceleration block that file.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
import numpy as np
from dub_batch import units
from split_dub_batches import norm, read, write

RATE = 24000


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def decode(path, tempo=1.0):
    cmd = ['ffmpeg', '-v', 'error', '-i', str(path)]
    if tempo != 1.0:
        cmd += ['-af', f'atempo={tempo:.9f}']
    cmd += ['-f', 'f32le', '-ac', '1', '-ar', str(RATE), 'pipe:1']
    return np.frombuffer(subprocess.check_output(cmd), dtype='<f4').copy()


def fade(x, seconds=.008):
    x=x.copy(); n=min(int(seconds*RATE),len(x)//2)
    if n:
        x[:n]*=np.linspace(0,1,n);x[-n:]*=np.linspace(1,0,n)
    return x


def trim_silence(x):
    # Remove exterior digital silence only, retaining 120/150 ms for weak
    # consonants. This never cuts inside an utterance to force a duration fit.
    active=np.flatnonzero(np.abs(x)>.001)
    if not len(active):return x
    return x[max(0,active[0]-2880):min(len(x),active[-1]+3601)]


def clip_qa(directory):
    result={}
    for f in sorted(directory.glob('*.json')):
        data=json.loads(f.read_text())
        for row in data.get('files',[]):
            digest=data.get('inputs',{}).get(row['file'])
            if digest: result[(row['file'],digest)] = row
    return result


def validate_unit(unit, actual, folder, reviews):
    path=folder/unit['file']
    if actual is None or not path.exists(): raise ValueError('missing TTS '+unit['file'])
    if norm(actual['text']) != norm(unit['text']): raise ValueError('obsolete TTS text '+unit['file'])
    if actual['source_file']!=unit['source_file'] or actual['segments']!=unit['segments']:
        raise ValueError('obsolete source mapping '+unit['file'])
    digest=sha(path)
    if actual.get('audio_sha256')!=digest: raise ValueError('changed extracted audio '+unit['file'])
    q=reviews.get((unit['file'],digest))
    if not q: raise ValueError('missing independent clip QA '+unit['file'])
    if not all(q.get(k) is True for k in ('complete','first_word_clear','last_word_clear')) or q.get('english_speech') or q.get('issues'):
        raise ValueError('clip QA rejected '+unit['file']+': '+str(q))
    return path,digest


def place_events(length, events):
    """Reject any shared sample: never mix two intended dialogue events."""
    spoken=np.zeros(length,dtype=np.float32)
    used=np.zeros(length,dtype=bool)
    for start,audio,name in events:
        end=start+len(audio)
        if start<0 or end>length: raise ValueError('speech outside original duration: '+name)
        if used[start:end].any(): raise ValueError('overlapping dialogue: '+name)
        used[start:end]=True
        spoken[start:end]=audio
    return spoken,used


def assemble(row, planned, found, reviews, args):
    name=row['file']; original=args.source/name
    if row.get('needs_review'): raise ValueError('source transcription needs review')
    original_audio=decode(original); length=len(original_audio)
    if not planned:
        if any(s.get('ru','').strip() for s in row['segments']): raise ValueError('unassigned character')
        # These original music/effects files stay outside the source repository.
        return original_audio,{'file':name,'text':'','kind':'original nonverbal audio','events':[]}
    events=[]; edits=[]; metadata=[]
    verbal={i for i,s in enumerate(row['segments']) if s.get('ru','').strip()}
    covered=[]
    for i,u in enumerate(sorted(planned,key=lambda x:x['start'])):
        path,digest=validate_unit(u,found.get(u['file']),args.units,reviews)
        covered += u['segments']
        # Leave breathing space; do not stretch speech into the next scene.
        begin=max(0,u['start']-.06)
        next_start=planned[i+1]['start'] if i+1<len(planned) else length/RATE
        merged=len(u['segments'])>1 or len(planned)==1
        limit=min(next_start-.08, length/RATE-.02, length/RATE if merged else u['end']+.65)
        room=limit-begin
        audio=trim_silence(decode(path))
        if room<=0:raise ValueError('no room for dialogue '+u['file'])
        tempo=max(1.0,(len(audio)/RATE+.025)/room)
        if tempo>args.max_tempo:
            raise ValueError(f'too long: {u["file"]}, {len(audio)/RATE:.2f}s / {room:.2f}s, tempo {tempo:.2f}')
        if tempo>1:
            audio=trim_silence(decode(path,tempo))
            # atempo has a small algorithmic duration error; adjust the tempo,
            # never slice off samples/words to force an exact duration.
            for attempt in range(3):
                if len(audio)<=int(room*RATE):break
                adjusted=tempo*(len(audio)/(room*RATE))*1.01
                if adjusted>args.max_tempo:break
                tempo=adjusted;audio=trim_silence(decode(path,tempo))
        if len(audio)>int(room*RATE): raise ValueError('tempo output exceeds available time '+u['file'])
        # Match dialogue loudness without hard limiting or clipped peaks.
        active=audio[np.abs(audio)>.015]
        rms=float(np.sqrt(np.mean(active**2))) if len(active) else 0
        if rms==0: raise ValueError('silent speech '+u['file'])
        gain=min(.12/rms,.88/max(float(np.max(np.abs(audio))),1e-6))
        audio=fade(audio*gain)
        start=round(begin*RATE);events.append((start,audio,u['file']))
        edits.append((max(0,round((u['start']-.18)*RATE)),min(length,round((u['end']+.22)*RATE))))
        metadata.append({'file':u['file'],'sha256':digest,'text':u['text'],
            'start_sample':start,'end_sample':start+len(audio),'tempo':tempo,'gain':gain})
    if len(covered)!=len(set(covered)) or set(covered)!=verbal:
        raise ValueError('missing or duplicated source segment assignment')
    # A clean dialogue-only file contains no original speech at any position.
    mixed=row['music'] or row['effects'] or row['speech_over_music']
    has_nonverbal=any(not s.get('ru','').strip() for s in row['segments'])
    if not mixed and not has_nonverbal:
        bed=np.zeros(length,dtype=np.float32)
    else:
        replacement=np.zeros(length,dtype=np.float32)
        # Music between lines (the forest elf's flute solo) stays untouched in
        # the original track. Separation is needed only under spoken lines.
        if row['speech_over_music']:
            stem=args.backgrounds/Path(name).stem/'background.wav'
            info=stem.with_name('metadata.json')
            if not stem.exists() or not info.exists(): raise ValueError('missing separated background')
            if json.loads(info.read_text())['sha256']!=sha(original): raise ValueError('background source hash changed')
            qa_file=args.background_review/(Path(name).stem+'.json')
            if not qa_file.exists():raise ValueError('missing background residual-speech QA')
            qa=json.loads(qa_file.read_text())
            if qa.get('sha256')!=sha(stem):raise ValueError('background QA hash changed')
            if qa.get('intelligible_speech') is not False or qa.get('acceptable_under_dubbing') is not True:
                raise ValueError('background QA rejected: possible original speech/artifacts')
            replacement=decode(stem)
            if abs(len(replacement)-length)>48: raise ValueError('separated background duration mismatch')
            replacement=np.pad(replacement[:length],(0,max(0,length-len(replacement))))
        mask=np.zeros(length,dtype=np.float32)
        for start,end in edits:
            mask[start:end]=1
        # Fade only at union boundaries, avoiding holes between adjacent edits.
        n=int(.02*RATE)
        for start in np.flatnonzero(np.diff(np.r_[0,mask])==1):
            mask[max(0,start-n):start]=np.linspace(0,1,min(n,start),endpoint=False)
        for end in np.flatnonzero(np.diff(np.r_[mask,0])==-1)+1:
            count=min(n,length-end)
            if count:mask[end:end+count]=np.linspace(1,0,count,endpoint=False)
        bed=original_audio*(1-mask)+replacement*mask
    if name=='intro_2sec.wav':
        credit=decode(args.credit)
        # First original verbal reaction starts at 32 s; the credit is at 2 s.
        events.append((2*RATE,fade(credit),'opening-credit'))
        metadata.insert(0,{'file':'opening-credit','sha256':sha(args.credit),
            'text':'Озвучка игры выполнена телеграм-каналом «Точки над ИИ».',
            'start_sample':2*RATE,'end_sample':2*RATE+len(credit),'tempo':1.0,'gain':1.0})
    spoken,used=place_events(length,events)
    # Duck just the background under Russian dialogue; smooth the gain change.
    duck=np.convolve(used.astype(np.float32),np.ones(481,dtype=np.float32)/481,mode='same')
    mixed_audio=bed*(1-.45*duck)+spoken
    peak=float(np.max(np.abs(mixed_audio)))
    if peak>.98:mixed_audio*=.98/peak
    return mixed_audio,{'file':name,'text':' '.join(m['text'] for m in metadata),
        'kind':'dubbed','source_sha256':sha(original),'events':metadata,
        'original_frames_at_24000':length,'mix_gain':min(1,.98/max(peak,1e-6))}


def main():
    p=argparse.ArgumentParser()
    for key in ('source','review','units','unit_review','backgrounds','output'):p.add_argument(key,type=Path)
    p.add_argument('--credit',type=Path,required=True);p.add_argument('--max-tempo',type=float,default=1.25)
    p.add_argument('--background-review',type=Path,required=True)
    a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
    rows=[r for f in sorted(a.review.glob('review-[0-9][0-9][0-9].json')) for r in json.loads(f.read_text())]
    plan=units(a.review);found={r['file']:r for r in json.loads((a.units/'manifest.json').read_text())};q=clip_qa(a.unit_review)
    manifest=[];issues=[]
    for row in rows:
        try:
            planned=sorted([u for u in plan if u['source_file']==row['file']],key=lambda x:x['start'])
            audio,entry=assemble(row,planned,found,q,a)
            target=a.output/row['file'];write(target,audio,RATE)
            entry['audio_sha256']=sha(target);manifest.append(entry)
        except (ValueError,KeyError) as e:issues.append({'file':row['file'],'issue':str(e)})
    source_names={f.name for f in a.source.glob('*.wav')}
    missing=source_names-{r['file'] for r in rows}
    issues += [{'file':name,'issue':'missing source review'} for name in sorted(missing)]
    (a.output/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    (a.output/'issues.json').write_text(json.dumps(issues,ensure_ascii=False,indent=2)+'\n')
    print('Assembled',len(manifest),'of',len(source_names),'files; blocked',len(issues))
    for issue in issues:print(issue['file'],issue['issue'])
    if issues:raise SystemExit(1)


if __name__=='__main__':main()
