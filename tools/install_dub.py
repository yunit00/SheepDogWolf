#!/usr/bin/env python3
"""Install only a complete, independently reviewed, hash-bound voice pack.

Generated/mixed Russian files are archived in the source repository. Purely
original nonverbal files are copied from the user's local game at install time.
The existing language pack remains untouched until all checks have passed.
"""
import argparse,hashlib,json,shutil,wave
from pathlib import Path


def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def validate(candidate,source,reviews):
    rows=json.loads((candidate/'manifest.json').read_text())
    names=[r['file'] for r in rows]
    if len(names)!=len(set(names)) or set(names)!={f.name for f in source.glob('*.wav')}:
        raise ValueError('Voice pack does not cover each original WAV exactly once')
    qa={}
    for p in sorted(reviews.glob('*.json')):
        data=json.loads(p.read_text())
        if data.get('snapshot_version')!=1:continue
        for row in data.get('files',[]):
            qa[(row['file'],data['inputs'][row['file']],data['references'][row['file']])]=row
    for row in rows:
        name=row['file']
        if Path(name).name!=name:raise ValueError('Unsafe file name')
        path=candidate/name
        if row['audio_sha256']!=digest(path):raise ValueError('Changed candidate '+name)
        if row.get('source_sha256') and row['source_sha256']!=digest(source/name):raise ValueError('Changed original '+name)
        q=qa.get((name,row['audio_sha256'],row['text']))
        if not q or not all(q.get(k) is True for k in ('complete','first_word_clear','last_word_clear')) or q.get('english_speech') or q.get('issues'):
            raise ValueError('Final audio QA missing/failed: '+name)
        with wave.open(str(path)) as wav:
            if (wav.getnchannels(),wav.getsampwidth(),wav.getframerate())!=(1,2,24000):raise ValueError('Unexpected WAV format '+name)
            frames=wav.getnframes()
        with wave.open(str(source/name)) as wav:
            expected=round(wav.getnframes()/wav.getframerate()*24000)
        if abs(frames-expected)>2:raise ValueError('Changed scene duration '+name)
        end=0
        for e in sorted(row.get('events',[]),key=lambda e:e['start_sample']):
            if e['start_sample']<end or e['end_sample']>frames or e['end_sample']<=e['start_sample']:
                raise ValueError('Overlapping or invalid dialogue event '+name)
            end=e['end_sample']
    return rows


def main():
    p=argparse.ArgumentParser();p.add_argument('candidate',type=Path);p.add_argument('source',type=Path)
    p.add_argument('reviews',type=Path);p.add_argument('language',type=Path);p.add_argument('archive',type=Path)
    p.add_argument('--check-only',action='store_true');a=p.parse_args()
    rows=validate(a.candidate,a.source,a.reviews)
    if a.check_only:print('Complete reviewed voice pack:',len(rows));return
    target=a.language/'voices';stage=a.language/'voices-pending'
    if target.exists() or stage.exists() or a.archive.exists():raise ValueError('Preserve existing pack; choose fresh destinations')
    stage.mkdir(parents=True);a.archive.mkdir(parents=True)
    for row in rows:
        shutil.copy2(a.candidate/row['file'],stage/row['file'])
        if row['kind']=='dubbed':shutil.copy2(a.candidate/row['file'],a.archive/row['file'])
    (a.archive/'manifest.json').write_text(json.dumps(rows,ensure_ascii=False,indent=2)+'\n')
    stage.rename(target)
    config=a.language/'language.txt';text=config.read_text()
    text='\n'.join(line for line in text.splitlines() if not line.strip().lower().startswith('voices ='))+'\n'
    config.write_text(text)
    print('Installed',len(rows),'reviewed voice files; archived generated/mixed files in',a.archive)


if __name__=='__main__':main()
