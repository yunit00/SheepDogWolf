#!/usr/bin/env python3
"""Audio QA: transcript and boundaries, kept as evidence rather than assumed success."""
import argparse
import base64
import json
import wave
from pathlib import Path
from gemini_voice import request

p=argparse.ArgumentParser()
p.add_argument("audio",type=Path)
p.add_argument("script",type=Path)
p.add_argument("output",type=Path)
a=p.parse_args()
script=json.loads(a.script.read_text())
with wave.open(str(a.audio)) as wav:
    duration=wav.getnframes()/wav.getframerate()
prompt="""Listen to the generated Russian game dubbing. Return JSON object with:
{transcript: exact actually spoken Russian, character_voice: description, lisp: absent/mild/clear/excessive,
issues:[...], clips:[{file,start,end,transcript,complete}]}. Times are seconds from beginning of the attached audio.
Use decimal SECONDS, NEVER MMSS: one minute 10.65 seconds is 70.65, NOT 110.65.
For each intended clip below find its spoken text. Do not copy intended text if audio differs.
Exclude spoken КОНЕЦ РЕПЛИКИ boundary markers from clip boundaries. Include all first and last syllables.
Report omitted words, chopped endings, extra speech, unintelligibility, artifacts or incorrect pronunciation.
Do not claim a clip exists if it is missing. If one single text is intended, use file credit.wav.
Reference script:\n"""+json.dumps(script.get('clips',{'file':'credit.wav','text':script.get('text','')}),ensure_ascii=False)+f'\nExact file duration: {duration:.3f} seconds. No timestamp may exceed this duration.'
r=request('models/gemini-3.8-flash:generateContent',{'contents':[{'parts':[{'text':prompt},{'inlineData':{
    'mimeType':'audio/wav','data':base64.b64encode(a.audio.read_bytes()).decode()}}]}],
    'generationConfig':{'responseMimeType':'application/json','temperature':0.1,'maxOutputTokens':12000}})
c=r['candidates'][0]
if c.get('finishReason')!='STOP':raise RuntimeError('Incomplete audio review')
out=json.loads(''.join(x.get('text','') for x in c['content']['parts']))
a.output.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n')
print(json.dumps(out,ensure_ascii=False,indent=2))
