#include "brlsym.hpp"
#include "unicode.h"

namespace music { namespace braille {

upper_digit_symbols::upper_digit_symbols()
{
  add(L"⠚", 0)
     (L"⠁", 1) (L"⠃", 2) (L"⠉", 3)
     (L"⠙", 4) (L"⠑", 5) (L"⠋", 6)
     ;
}

upper_digit_symbols upper_digit_sign;

lower_digit_symbols::lower_digit_symbols()
{
  add(L"⠴", 0)
     (L"⠂", 1) (L"⠆", 2) (L"⠒", 3)
     (L"⠲", 4) (L"⠢", 5) (L"⠖", 6)
     (L"⠶", 7) (L"⠦", 8) (L"⠔", 9)
     ;
}

lower_digit_symbols lower_digit_sign;

dots_123_symbols::dots_123_symbols()
{
  for (int i = 1; i < 0X40; ++i)
    if (i & 0X07) {
      wchar_t const pattern[2] = { UNICODE_BRAILLE_ROW | i, 0 };
      add(pattern);
    }
}

dots_123_symbols dots_123;

accidental_symbols::accidental_symbols()
{
  add(L"⠡", natural)
     (L"⠣", flat)
     (L"⠩", sharp)
     ;
}

accidental_symbols accidental_sign;

octave_symbols::octave_symbols()
{
  add(L"⠈⠈", 0)
     (L"⠈", 1)
     (L"⠘", 2)
     (L"⠸", 3)
     (L"⠐", 4)
     (L"⠨", 5)
     (L"⠰", 6)
     (L"⠠", 7)
     (L"⠠⠠", 8)
     ;
}

octave_symbols octave_sign;

pitch_and_value_symbols::pitch_and_value_symbols()
{
  add(L"⠽", ambiguous::pitch_and_value(C, rational(1, 1), rational(1, 16)))
     (L"⠵", ambiguous::pitch_and_value(D, rational(1, 1), rational(1, 16)))
     (L"⠯", ambiguous::pitch_and_value(E, rational(1, 1), rational(1, 16)))
     (L"⠿", ambiguous::pitch_and_value(F, rational(1, 1), rational(1, 16)))
     (L"⠷", ambiguous::pitch_and_value(G, rational(1, 1), rational(1, 16)))
     (L"⠮", ambiguous::pitch_and_value(A, rational(1, 1), rational(1, 16)))
     (L"⠾", ambiguous::pitch_and_value(B, rational(1, 1), rational(1, 16)))
     (L"⠝", ambiguous::pitch_and_value(C, rational(1, 2), rational(1, 32)))
     (L"⠕", ambiguous::pitch_and_value(D, rational(1, 2), rational(1, 32)))
     (L"⠏", ambiguous::pitch_and_value(E, rational(1, 2), rational(1, 32)))
     (L"⠟", ambiguous::pitch_and_value(F, rational(1, 2), rational(1, 32)))
     (L"⠗", ambiguous::pitch_and_value(G, rational(1, 2), rational(1, 32)))
     (L"⠎", ambiguous::pitch_and_value(A, rational(1, 2), rational(1, 32)))
     (L"⠞", ambiguous::pitch_and_value(B, rational(1, 2), rational(1, 32)))
     (L"⠹", ambiguous::pitch_and_value(C, rational(1, 4), rational(1, 64)))
     (L"⠱", ambiguous::pitch_and_value(D, rational(1, 4), rational(1, 64)))
     (L"⠫", ambiguous::pitch_and_value(E, rational(1, 4), rational(1, 64)))
     (L"⠻", ambiguous::pitch_and_value(F, rational(1, 4), rational(1, 64)))
     (L"⠳", ambiguous::pitch_and_value(G, rational(1, 4), rational(1, 64)))
     (L"⠪", ambiguous::pitch_and_value(A, rational(1, 4), rational(1, 64)))
     (L"⠺", ambiguous::pitch_and_value(B, rational(1, 4), rational(1, 64)))
     ;
}

pitch_and_value_symbols pitch_and_value_sign;

rest_symbols::rest_symbols()
{
  add(L"⠍", ambiguous::value(rational(1, 1), rational(1, 16)))
     (L"⠥", ambiguous::value(rational(1, 2), rational(1, 32)))
     (L"⠧", ambiguous::value(rational(1, 4), rational(1, 64)))
     (L"⠭", ambiguous::value(rational(1, 8), rational(1, 128)))
     ;
}

rest_symbols rest_sign;

interval_symbols::interval_symbols()
{
  add(L"⠌", second)
     (L"⠬", third)
     (L"⠼", fourth)
     (L"⠔", fifth)
     (L"⠴", sixth)
     (L"⠒", seventh)
     (L"⠤", octave)
     ;
}

interval_symbols interval_sign;

finger_symbols::finger_symbols()
{
  add(L"⠁", 1)
     (L"⠃", 2)
     (L"⠇", 3)
     (L"⠂", 4)
     (L"⠅", 5)
     ;
}

finger_symbols finger_sign;

}}

