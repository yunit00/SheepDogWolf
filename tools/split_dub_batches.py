#!/usr/bin/env python3
"""Extract reviewed batch utterances at quiet boundaries; report rather than hide failures."""
import argparse, difflib, hashlib, json, re, wave
from pathlib import Path
import numpy as np

def read(path):
    with wave.open(str(path)) as w:
        assert w.getnchannels()==1 and w.getsampwidth()==2
        rate=w.getframerate();x=np.frombuffer(w.readframes(w.getnframes()),dtype='<i2').astype(np.float32)/32768
    return x,rate

def write(path,x,rate):
    with wave.open(str(path),'wb') as w:
        w.setnchannels(1);w.setsampwidth(2);w.setframerate(rate);w.writeframes((np.clip(x,-1,1)*32767).astype('<i2').tobytes())

def norm(s):
    # Porky's repeated onset consonants are performance directions, not extra
    # lexical words. Keep repeated full words significant ("иди иди" != "иди").
    s=s.lower().replace('ё','е')
    while True:
        expanded=re.sub(r'\b([а-я]{1,3})-(?=\1)', '', s)
        if expanded==s:break
        s=expanded
    text=re.sub(r'[^а-яa-z0-9]','',s.lower().replace('ё','е'))
    # Written length of a wordless pondering sound is not a missing syllable.
    return 'хм' if re.fullmatch('хм+',text) else text

def quiet_cut(x,rate,left,right):
    left=max(0,int(left*rate));right=min(len(x),int(right*rate));width=int(.025*rate)
    if right-left<=width:return (left+right)//2,False
    candidates=list(range(left+width//2,right-width//2,max(1,int(.005*rate))))
    scores=[float(np.mean(x[c-width//2:c+width//2]**2)) for c in candidates]
    idx=int(np.argmin(scores));return candidates[idx], scores[idx]<0.00002

def extract(folder,output):
    output.mkdir(parents=True,exist_ok=True);manifest=[];issues=[]
    overrides=json.loads((folder/'overrides.json').read_text()) if (folder/'overrides.json').exists() else {}
    decisions=json.loads((folder/'review-decisions.json').read_text()) if (folder/'review-decisions.json').exists() else {}
    source_review=folder.parent/'source-review'
    planned=None
    if source_review.exists():
        from dub_batch import units
        planned={u['file'] for u in units(source_review)}
    for script in sorted(folder.glob('*.script.json')):
        prefix=script.name.removesuffix('.script.json');qa=folder/(prefix+'.review.json');audio=folder/(prefix+'.wav')
        if not qa.exists() or not audio.exists():continue
        spec=json.loads(script.read_text());review=json.loads(qa.read_text());clips=review.get('clips',[])
        active_files={c['file'] for c in spec['clips'] if overrides.get(c['file'],prefix)==prefix and (planned is None or c['file'] in planned)}
        if not active_files:continue
        if len(clips)!=len(spec['clips']) or [c['file'] for c in clips]!=[c['file'] for c in spec['clips']]:
            issues.append({'batch':prefix,'issue':'QA clip coverage/order mismatch'});continue
        accepted=decisions.get(prefix,{})
        active_files -= set(accepted.get('excluded_files',[]))
        unaccepted=[i for i in review.get('issues',[]) if i not in accepted.get('accepted_issues',[])]
        if unaccepted:
            issues.append({'batch':prefix,'issue':'QA reported problems','details':review['issues']});continue
        x,rate=read(audio);duration=len(x)/rate
        cuts=[0];valid=True
        for i in range(len(clips)-1):
            a,b=clips[i],clips[i+1]
            if a['end']>b['start']+.1 or a['start']>=a['end']:
                valid=False;issues.append({'batch':prefix,'issue':'invalid or overlapping QA times'});break
            window=accepted.get('cut_windows',{}).get(a['file'],[a['end']-.08,b['start']+.08])
            if not (a['start']<window[0]<window[1]<b['end']):
                valid=False;issues.append({'batch':prefix,'issue':'invalid reviewed cut window'});break
            cut,quiet=quiet_cut(x,rate,*window)
            if not quiet:
                valid=False;issues.append({'batch':prefix,'issue':'no quiet cut','after':a['file']});break
            cuts.append(cut)
        if not valid:continue
        cuts.append(len(x))
        if any(a>=b for a,b in zip(cuts,cuts[1:])):
            issues.append({'batch':prefix,'issue':'non-increasing cut positions'});continue
        for i,(reference,actual) in enumerate(zip(spec['clips'],clips)):
            if reference['file'] not in active_files:continue
            similarity=difflib.SequenceMatcher(None,norm(reference['text']),norm(actual['transcript'])).ratio()
            if not actual.get('complete') or similarity<.90 or (actual['end']>duration+.25 and not (i==len(clips)-1 and actual['start']<duration and actual['end']<duration+3)):
                issues.append({'file':reference['file'],'issue':'incomplete/mismatching speech','similarity':similarity});continue
            samples=x[cuts[i]:cuts[i+1]]
            # Remove only near-silent exterior padding. Preserve breath/consonant tails.
            win=max(1,int(.01*rate));power=np.array([np.sqrt(np.mean(samples[k:k+win]**2)) for k in range(0,len(samples),win)])
            active=np.flatnonzero(power>max(.002,float(power.max())*.025))
            if not len(active):issues.append({'file':reference['file'],'issue':'silent'});continue
            start=max(0,int(active[0]*win-.12*rate));end=min(len(samples),int((active[-1]+1)*win+.22*rate))
            samples=samples[start:end]
            if any(m['file']==reference['file'] for m in manifest):
                raise RuntimeError('Duplicate output utterance: '+reference['file'])
            target=output/reference['file'];write(target,samples,rate)
            manifest.append({**reference,'batch':prefix,'batch_cut':[cuts[i]/rate,cuts[i+1]/rate],
                'batch_sample_range':[cuts[i]+start,cuts[i]+end],
                'audio_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),
                'seconds':len(samples)/rate,'transcript':actual['transcript'],'similarity':similarity,
                'accepted_review_notes':accepted,
                'qa':'batch reviewed; extracted clip still needs independent review'})
    (output/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    (output/'issues.json').write_text(json.dumps(issues,ensure_ascii=False,indent=2)+'\n')
    print('extracted',len(manifest),'issues',len(issues));print(json.dumps(issues,ensure_ascii=False))

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('batches',type=Path);p.add_argument('output',type=Path);a=p.parse_args();extract(a.batches,a.output)
