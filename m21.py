# -*- coding: utf-8 -*-
from music21.key import Key
from music21.tinyNotation import TinyNotationStream
from os import system
from sys import argv

convertible_snippets = [
# In theory, an ambiguous measure since we have half and 32th notes in 4/4 time.
# However, since the 32th notes fall on a beat, music21 correctly transforms
# them into a notegroup, which ends up as unambiguous.  Neat special case!
  (None, "4/4", "C2 C32 D E F E F G A B4", None)

# music21 correctly recognizes the repeated measure and uses a simile sign to
# indicate it.  This is correctly recognized by BMC and transformed into the
# equivalent "\repeat unfold" in LilyPond.
, (None, "3/4", "C4 E G C E G C'2.", None)

# music21 transcribes the tie and makes notegroups out of the second and third
# beat (⠐⠷⠿⠓⠈⠉⠷⠑⠋⠛⠷⠊⠚⠩⠙), which BMC handles correctly.
# The key signature is also handled properly on all sides.
# FIXME: BMC should preserve notegroups such that beaming can be extracted from
# them.
, ("G", "3/4", "g16 f# g8~ g16 d e f# g a b c'#", [(0, '4'), (7, '1')])
]

def objectToBMC(object):
    return system('%s %s' % (argv[1], object.write('braille'))) == 0

if __name__ == "__main__":
    for (key, time, snippet, fingering) in convertible_snippets:
        notation = TinyNotationStream(snippet, time)
        if key is not None:
            notation.insert(0, Key(key))
        if fingering is not None:
            for (note_index, finger) in fingering:
                notation.notes[note_index].fingering = finger
        assert(objectToBMC(notation))
