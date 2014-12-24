# -*- coding: utf-8 -*-
from music21.key import Key
from music21.tinyNotation import TinyNotationStream
from os import system
from sys import argv, exit

class Snippet(TinyNotationStream):
    def __init__(self, key=None, time=None, melody='', fingering=None):
        TinyNotationStream.__init__(self, melody, time)
        if key is not None:
            self.insert(0, Key(key))
        if fingering is not None:
            for (note_index, finger) in fingering:
                self.notes[note_index].fingering = finger

convertible_snippets = [
# In theory, an ambiguous measure since we have half and 32th notes in 4/4 time.
# However, since the 32th notes fall on a beat, music21 correctly transforms
# them into a notegroup, which ends up as unambiguous.  Neat special case!
  Snippet(melody="C2 C32 D E F E F G A B4")

# music21 correctly recognizes the repeated measure and uses a simile sign to
# indicate it.  This is correctly recognized by BMC and transformed into the
# equivalent "\repeat unfold" in LilyPond.
, Snippet(time="3/4", melody="C4 E G C E G c2.")

# music21 transcribes the tie and makes notegroups out of the second and third
# beat (⠐⠷⠿⠓⠈⠉⠷⠑⠋⠛⠷⠊⠚⠩⠙), which BMC handles correctly including beaming output.
# The key signature is also handled properly on all sides.
, Snippet("G", "3/4", "g16 f# g8~ g16 d e f# g a b c'#", [(0, '4'), (7, '1')])
]

if __name__ == "__main__":
    if len(argv) < 2:
        print "Missing path to BMC."
        exit(1)
    bmc = argv[1]
    for stream in convertible_snippets:
        assert(system('%s --lilypond %s' % (bmc, stream.write('braille'))) == 0)
