// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

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
  add(L"⠈⠈", 1)
     (L"⠈", 2)
     (L"⠘", 3)
     (L"⠸", 4)
     (L"⠐", 5)
     (L"⠨", 6)
     (L"⠰", 7)
     (L"⠠", 8)
     (L"⠠⠠", 9)
     ;
}

octave_symbols octave_sign;

pitch_and_value_symbols::pitch_and_value_symbols()
{
  add(L"⠽", ambiguous::pitch_and_value(C, ambiguous::whole_or_16th))
     (L"⠵", ambiguous::pitch_and_value(D, ambiguous::whole_or_16th))
     (L"⠯", ambiguous::pitch_and_value(E, ambiguous::whole_or_16th))
     (L"⠿", ambiguous::pitch_and_value(F, ambiguous::whole_or_16th))
     (L"⠷", ambiguous::pitch_and_value(G, ambiguous::whole_or_16th))
     (L"⠮", ambiguous::pitch_and_value(A, ambiguous::whole_or_16th))
     (L"⠾", ambiguous::pitch_and_value(B, ambiguous::whole_or_16th))
     (L"⠝", ambiguous::pitch_and_value(C, ambiguous::half_or_32th))
     (L"⠕", ambiguous::pitch_and_value(D, ambiguous::half_or_32th))
     (L"⠏", ambiguous::pitch_and_value(E, ambiguous::half_or_32th))
     (L"⠟", ambiguous::pitch_and_value(F, ambiguous::half_or_32th))
     (L"⠗", ambiguous::pitch_and_value(G, ambiguous::half_or_32th))
     (L"⠎", ambiguous::pitch_and_value(A, ambiguous::half_or_32th))
     (L"⠞", ambiguous::pitch_and_value(B, ambiguous::half_or_32th))
     (L"⠹", ambiguous::pitch_and_value(C, ambiguous::quarter_or_64th))
     (L"⠱", ambiguous::pitch_and_value(D, ambiguous::quarter_or_64th))
     (L"⠫", ambiguous::pitch_and_value(E, ambiguous::quarter_or_64th))
     (L"⠻", ambiguous::pitch_and_value(F, ambiguous::quarter_or_64th))
     (L"⠳", ambiguous::pitch_and_value(G, ambiguous::quarter_or_64th))
     (L"⠪", ambiguous::pitch_and_value(A, ambiguous::quarter_or_64th))
     (L"⠺", ambiguous::pitch_and_value(B, ambiguous::quarter_or_64th))
     (L"⠙", ambiguous::pitch_and_value(C, ambiguous::eighth_or_128th))
     (L"⠑", ambiguous::pitch_and_value(D, ambiguous::eighth_or_128th))
     (L"⠋", ambiguous::pitch_and_value(E, ambiguous::eighth_or_128th))
     (L"⠛", ambiguous::pitch_and_value(F, ambiguous::eighth_or_128th))
     (L"⠓", ambiguous::pitch_and_value(G, ambiguous::eighth_or_128th))
     (L"⠊", ambiguous::pitch_and_value(A, ambiguous::eighth_or_128th))
     (L"⠚", ambiguous::pitch_and_value(B, ambiguous::eighth_or_128th))
     ;
}

pitch_and_value_symbols pitch_and_value_sign;

rest_symbols::rest_symbols()
{
  add(L"⠍", ambiguous::value(ambiguous::whole_or_16th))
     (L"⠥", ambiguous::value(ambiguous::half_or_32th))
     (L"⠧", ambiguous::value(ambiguous::quarter_or_64th))
     (L"⠭", ambiguous::value(ambiguous::eighth_or_128th))
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

value_distinction_symbols::value_distinction_symbols()
{
  add(L"⠣⠂", ambiguous::distinct)
     (L"⠠⠣⠂", ambiguous::small_follows)
     (L"⠘⠣⠂", ambiguous::large_follows)
     ;
}

value_distinction_symbols value_distinction_sign;

articulation_symbols::articulation_symbols()
{
  add(L"⠢", short_appoggiatura)
     (L"⠐⠢", appoggiatura)
     (L"⠦", staccato)
     (L"⠠⠦", staccatissimo)
     (L"⠐⠦", mezzo_staccato)
     (L"⠸⠦", agogic_accent)
     (L"⠨⠦", accent)
     (L"⠲", turn_between_notes)
     (L"⠠⠲", turn_above_or_below_note)
     (L"⠲⠇", inverted_turn_between_notes)
     (L"⠠⠲⠇", inverted_turn_above_or_below_note)
     (L"⠐⠖", short_trill)
     (L"⠰⠖", extended_short_trill)
     (L"⠐⠖⠇", mordent)
     (L"⠰⠖⠇", extended_mordent)
     (L"⠜⠅", arpeggio_up)
     (L"⠐⠜⠅", arpeggio_up_multi_staff)
     (L"⠜⠅⠅", arpeggio_down)
     (L"⠐⠜⠅⠅", arpeggio_down_multi_staff)
     ;
}

articulation_symbols articulation_sign;

tie_symbol::tie_symbol()
{
  add(L"⠈⠉");
}

tie_symbol tie_sign;

barline_symbols::barline_symbols()
{
  add(L"⠣⠶", ambiguous::begin_repeat)
     (L"⠣⠆", ambiguous::end_repeat)
     ;
}

barline_symbols barline_sign;

}}

