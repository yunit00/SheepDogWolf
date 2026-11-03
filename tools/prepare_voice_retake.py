#!/usr/bin/env python3
"""Prepare a new take without overwriting the previous audio or its QA evidence."""
import argparse, hashlib, json
from pathlib import Path
from dub_batch import CAST, units


def main():
    p=argparse.ArgumentParser();p.add_argument('review',type=Path);p.add_argument('batches',type=Path)
    p.add_argument('files',nargs='+');p.add_argument('--reason',required=True);p.add_argument('--direction',default='')
    a=p.parse_args();planned={u['file']:u for u in units(a.review)}
    missing=set(a.files)-set(planned)
    if missing:p.error('Unknown or held utterances: '+', '.join(sorted(missing)))
    override_file=a.batches/'overrides.json'
    overrides=json.loads(override_file.read_text()) if override_file.exists() else {}
    for name in a.files:
        u=planned[name];voice,style=CAST[u['speaker']]
        budget=max(.7,u['end']-u['start'])
        spec={'speaker':u['speaker'],'voice':voice,'clips':[u],
            'replacement_reason':a.reason,
            'prompt':f'Russian dialogue for a fictional family cartoon game. Original character interpretation, not a real actor imitation. {style} '
                f'Natural clear brisk delivery, finish the words in about {budget:.1f} seconds. '
                'Speak every written word once and fully; no added words, laughs, introduction, conclusion or numbering. '
                'Preserve only stutters explicitly written in the text. No music or effects. '
                'Leave 0.4 seconds of silence before the first word and after the last word. '
                'Do not clip the first or last consonants. '+a.direction+'\n\n'+u['text']}
        digest=hashlib.sha256(json.dumps(spec,ensure_ascii=False,sort_keys=True).encode()).hexdigest()[:12]
        prefix=u['speaker'].split()[0].lower()+'-retake-'+digest
        target=a.batches/(prefix+'.script.json')
        if not target.exists():target.write_text(json.dumps(spec,ensure_ascii=False,indent=2)+'\n')
        overrides[name]=prefix;print(target,flush=True)
    override_file.write_text(json.dumps(overrides,ensure_ascii=False,indent=2)+'\n')


if __name__=='__main__':main()
