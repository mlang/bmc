from music21 import *
import os
import sys

def objectToBMC(object):
    if os.system('%s %s' % (sys.argv[1], object.write('braille'))) == 0:
        return True
    else:
        return False

if __name__ == "__main__":
    assert(objectToBMC(converter.parse('tinynotation: 4/4 C2 C32 D E F E F G A B4')))

