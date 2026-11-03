#!/usr/bin/env python3
"""Locally separate known cinematic mixes; the installed game never needs ML tools."""
import argparse, hashlib, json
from pathlib import Path
import numpy as np
import soundfile as sf
import torch
import torchaudio.functional as AF
from demucs.pretrained import get_model
from demucs.apply import apply_model

p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path)
p.add_argument('--device',default='cpu');p.add_argument('files',nargs='+');a=p.parse_args()
a.output.mkdir(parents=True,exist_ok=True)
torch.set_num_threads(4)
# Demucs RemoteRepo downloads only its packaged, official Meta URLs and verifies
# the checkpoint's hash from the release filename before loading it.
model=get_model('htdemucs');model.eval()
for name in a.files:
    assert Path(name).name==name and name.lower().endswith('.wav')
    target=a.output/Path(name).stem
    if (target/'background.wav').exists(): print('cached',name,flush=True);continue
    target.mkdir(exist_ok=True)
    audio,rate=sf.read(a.source/name,dtype='float32',always_2d=True)
    x=torch.from_numpy(audio.T.copy())
    if x.shape[0]==1:x=x.repeat(2,1)
    x=AF.resample(x,rate,model.samplerate)
    ref=x.mean(0);mean=ref.mean();std=ref.std().clamp(min=1e-6)
    with torch.inference_mode():
        stems=apply_model(model,((x-mean)/std)[None],device=a.device,shifts=1,
                          split=True,overlap=0.25,progress=True,num_workers=0)[0]*std
    idx=model.sources.index('vocals')
    vocals=stems[idx]
    background=stems.sum(0)-vocals+mean
    for label,tensor in [('vocals',vocals),('background',background)]:
        sf.write(target/(label+'.wav'),tensor.cpu().numpy().T,model.samplerate,subtype='FLOAT')
    (target/'metadata.json').write_text(json.dumps({'model':'htdemucs','sources':model.sources,
        'source':name,'sha256':hashlib.sha256((a.source/name).read_bytes()).hexdigest(),
        'rate':model.samplerate,'device':a.device,'status':'requires listening and residual-speech review'},indent=2)+'\n')
    print('separated',name,flush=True)
