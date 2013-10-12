from music21 import *
import os
import sys

convertible_snippets = [
# In theory, an ambiguous measure since we have half and 32th notes in 4/4 time.
# However, since the 32th notes fall on a beat, music21 correctly transforms
# them into a note group, which ends up as unambiguous.  Neat special case!
  '4/4 C2 C32 D E F E F G A B4'

# music21 correctly recognizes the repeated measure and uses a simile sign to
# indicate it.  This is correctly recognized by BMC and transformed into the
# equivalent "\repeat unfold" in LilyPond.
, '3/4 C4 E G C E G CC2.'
]

def objectToBMC(object):
    if os.system('%s %s' % (sys.argv[1], object.write('braille'))) == 0:
        return True
    else:
        return False

if __name__ == "__main__":
    for snippet in convertible_snippets:
        assert(objectToBMC(converter.parse('tinynotation: ' + snippet)))
