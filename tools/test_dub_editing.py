#!/usr/bin/env python3
"""Regression checks for duplicated samples, overlapping speech and unsafe cuts."""
import json
import tempfile
import unittest
from unittest.mock import patch
from pathlib import Path
import numpy as np
from assemble_dub import place_events, validate_unit
from split_dub_batches import extract, norm, quiet_cut, read, write


class DubEditingTests(unittest.TestCase):
    def test_review_hash_describes_sent_bytes_even_if_file_changes(self):
        import hashlib
        from review_dub_files import check
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp);out=folder/'qa';out.mkdir()
            path=folder/'test.wav';write(path,np.ones(1000)*.1,24000)
            before=hashlib.sha256(path.read_bytes()).hexdigest()
            def reply(*args):
                write(path,np.ones(1000)*.2,24000)
                return {'candidates':[{'finishReason':'STOP','content':{'parts':[{'text':json.dumps([{'file':'test.wav','complete':True}])}]}}]}
            with patch('review_dub_files.request',reply):
                result=check([{'file':'test.wav','text':'Привет','audio_sha256':before}],folder,out)
            report=json.loads((out/result).read_text())
            self.assertEqual(report['inputs']['test.wav'],before)
            self.assertNotEqual(report['inputs']['test.wav'],hashlib.sha256(path.read_bytes()).hexdigest())

    def test_incomplete_pack_cannot_be_installed(self):
        from install_dub import validate
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);source=root/'original';source.mkdir();candidate=root/'candidate';candidate.mkdir()
            write(source/'one.wav',np.zeros(1000),24000)
            (candidate/'manifest.json').write_text('[]')
            with self.assertRaisesRegex(ValueError,'cover each original'):
                validate(candidate,source,root/'qa')

    def test_stutter_variation_does_not_hide_repeated_words(self):
        self.assertEqual(norm('Н-не б-беспокойся'),norm('Не беспокойся'))
        self.assertNotEqual(norm('Иди иди сюда'),norm('Иди сюда'))
        self.assertNotEqual(norm('Ты не хочешь'),norm('Не хочешь'))

    def test_overlapping_speech_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'overlapping dialogue'):
            place_events(100,[(0,np.ones(60),'first'),(59,np.ones(20),'second')])

    def test_touching_intervals_do_not_repeat_a_sample(self):
        mixed,used=place_events(100,[(10,np.ones(20),'first'),(30,np.ones(40)*2,'second')])
        np.testing.assert_array_equal(mixed[10:30],np.ones(20))
        np.testing.assert_array_equal(mixed[30:70],np.ones(40)*2)
        self.assertEqual(used.sum(),60)
        self.assertEqual(mixed[70],0)

    def test_clip_outside_original_file_is_rejected(self):
        for start in [-1,81]:
            with self.assertRaisesRegex(ValueError,'outside original duration'):
                place_events(100,[(start,np.ones(20),'clip')])

    def test_voice_without_silence_cannot_be_cut(self):
        _,quiet=quiet_cut(np.ones(24000,dtype=np.float32)*.2,24000,.2,.5)
        self.assertFalse(quiet)

    def test_tiny_gap_is_rejected_without_empty_argmin(self):
        _,quiet=quiet_cut(np.zeros(24000,dtype=np.float32),24000,.2,.225)
        self.assertFalse(quiet)

    def test_independent_review_is_mandatory(self):
        with tempfile.TemporaryDirectory() as tmp:
            folder=Path(tmp);path=folder/'test.wav';write(path,np.ones(1000)*.1,24000)
            from assemble_dub import sha
            unit={'file':path.name,'source_file':'original.wav','segments':[0],'text':'Привет'}
            actual={**unit,'audio_sha256':sha(path)}
            with self.assertRaisesRegex(ValueError,'missing independent clip QA'):
                validate_unit(unit,actual,folder,{})
            bad={'complete':True,'first_word_clear':True,'last_word_clear':True,'english_speech':False,'issues':['repeated word']}
            with self.assertRaisesRegex(ValueError,'QA rejected'):
                validate_unit(unit,actual,folder,{(path.name,sha(path)):bad})

    def test_two_tones_are_split_without_neighbor_leak_or_duplication(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);folder=root/'batch';folder.mkdir();out=root/'clips';rate=24000
            x=np.zeros(rate*5,dtype=np.float32)
            t=np.arange(rate)/rate
            x[rate//2:rate+rate//2]=.2*np.sin(2*np.pi*440*t)
            x[rate*3:rate*4]=.2*np.sin(2*np.pi*880*t)
            write(folder/'example.wav',x,rate)
            clips=[{'file':'one.wav','text':'Один'},{'file':'two.wav','text':'Два'}]
            (folder/'example.script.json').write_text(json.dumps({'clips':clips}))
            review={'issues':[],'clips':[
                {'file':'one.wav','transcript':'Один','complete':True,'start':.5,'end':1.5},
                {'file':'two.wav','transcript':'Два','complete':True,'start':3.,'end':4.}]}
            (folder/'example.review.json').write_text(json.dumps(review))
            extract(folder,out)
            rows=json.loads((out/'manifest.json').read_text())
            self.assertEqual(len(rows),2)
            self.assertLessEqual(rows[0]['batch_sample_range'][1],rows[1]['batch_sample_range'][0])
            for name,expected,neighbor in [('one.wav',440,880),('two.wav',880,440)]:
                audio,_=read(out/name)
                spectrum=np.abs(np.fft.rfft(audio));freq=np.fft.rfftfreq(len(audio),1/rate)
                own=spectrum[np.abs(freq-expected)<3].max()
                wrong=spectrum[np.abs(freq-neighbor)<3].max()
                self.assertGreater(own,wrong*100)
                # Two PCM16 writes introduce quantization, but must not lose or
                # duplicate a meaningful fraction of either tone.
                expected_energy=float(np.sum(x**2))/2
                self.assertLess(abs(float(np.sum(audio**2))-expected_energy)/expected_energy,.001)


if __name__=='__main__':unittest.main()
