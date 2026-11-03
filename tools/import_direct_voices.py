#!/usr/bin/env python3
"""Replace a whole role with direct recordings; never mix backends for that role."""
import argparse, hashlib, json, shutil
from pathlib import Path
from dub_batch import units

p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('direct',type=Path);p.add_argument('output',type=Path)
p.add_argument('--speaker',default='Daffy Duck')
p.add_argument('--overlay',action='store_true',help='Replace only supplied retakes, keeping the rest of the same role')
p.add_argument('--skip-obsolete',action='store_true',help='Do not import older text superseded by the current translation')
a=p.parse_args()
plan={u['file']:u for u in units(a.review) if u['speaker']==a.speaker}
original=json.loads((a.output/'manifest.json').read_text())
direct=json.loads((a.direct/'manifest.json').read_text())
names={r['file'] for r in direct}
merged=[r for r in original if r['file'] not in names] if a.overlay else [r for r in original if r['speaker']!=a.speaker]
for row in direct:
    if row['speaker']!=a.speaker:raise ValueError('wrong role in direct folder')
    u=plan.get(row['file'])
    if not u or u['text']!=row['text']:
        if a.skip_obsolete:continue
        raise ValueError('obsolete direct text '+row['file'])
    path=a.direct/row['file']
    digest=hashlib.sha256(path.read_bytes()).hexdigest()
    if row['audio_sha256']!=digest:raise ValueError('direct WAV changed '+row['file'])
    shutil.copy2(path,a.output/row['file'])
    merged.append({**row,'batch':'openai-direct','batch_cut':[0,row['seconds']],
                   'batch_sample_range':[0,round(row['seconds']*24000)]})
(a.output/'manifest.json').write_text(json.dumps(merged,ensure_ascii=False,indent=2)+'\n')
print('Direct role files',len(merged)-len([r for r in original if r['speaker']!=a.speaker]),'of',len(plan))
