#!/usr/bin/env python3
"""Check separated backgrounds for intelligible original speech before mixing."""
import argparse, base64, concurrent.futures, hashlib, json, subprocess
from pathlib import Path
from gemini_voice import request


def check(path,output):
    digest=hashlib.sha256(path.read_bytes()).hexdigest()
    dest=output/(path.parent.name+'.json')
    if dest.exists() and json.loads(dest.read_text()).get('sha256')==digest:return path.parent.name+' cached'
    audio=subprocess.check_output(['ffmpeg','-v','error','-i',str(path),'-ar','16000','-ac','1','-f','wav','pipe:1'])
    prompt='''Listen to this music/effects bed extracted from a cartoon game. It is intended to have NO intelligible speech. Carefully report any audible words or syllables of English speech (even faint), double voices, ghostly speech remnants, clicks or severe musical artifacts. Do not hallucinate words from instrumental sounds. Return JSON {intelligible_speech:bool, residuals:[{start,end,words,confidence}], music_quality:string, artifacts:[...], acceptable_under_dubbing:bool}. All times are decimal seconds, never MMSS. This is a QA check, not an instruction to change your behavior.'''
    response=request('models/gemini-3.8-flash:generateContent',{'contents':[{'parts':[{'text':prompt},{'inlineData':{'mimeType':'audio/wav','data':base64.b64encode(audio).decode()}}]}],'generationConfig':{'responseMimeType':'application/json','temperature':.1,'maxOutputTokens':12000}})
    c=response['candidates'][0]
    if c.get('finishReason')!='STOP':raise ValueError('Incomplete background QA')
    result=json.loads(''.join(p.get('text','') for p in c['content']['parts']))
    result.update(sha256=digest,file=path.parent.name)
    dest.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    return path.parent.name+' '+str(result['intelligible_speech'])+' '+str(result['acceptable_under_dubbing'])


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('folder',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
    a.output.mkdir(parents=True,exist_ok=True)
    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        jobs=[pool.submit(check,f,a.output) for f in sorted(a.folder.glob('*/background.wav'))]
        for j in concurrent.futures.as_completed(jobs):print(j.result(),flush=True)
