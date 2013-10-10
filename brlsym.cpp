// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "brlsym.hpp"

namespace music { namespace braille {

upper_digit_symbols::upper_digit_symbols()
{
  add(L"⠚", 0)
     (L"⠁", 1) (L"⠃", 2) (L"⠉", 3)
     (L"⠙", 4) (L"⠑", 5) (L"⠋", 6)
     (L"⠛", 7) (L"⠓", 8) (L"⠊", 9)
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

step_and_value_symbols::step_and_value_symbols()
{
  add(L"⠽", std::make_pair(C, ast::whole_or_16th))
     (L"⠵", std::make_pair(D, ast::whole_or_16th))
     (L"⠯", std::make_pair(E, ast::whole_or_16th))
     (L"⠿", std::make_pair(F, ast::whole_or_16th))
     (L"⠷", std::make_pair(G, ast::whole_or_16th))
     (L"⠮", std::make_pair(A, ast::whole_or_16th))
     (L"⠾", std::make_pair(B, ast::whole_or_16th))
     (L"⠝", std::make_pair(C, ast::half_or_32th))
     (L"⠕", std::make_pair(D, ast::half_or_32th))
     (L"⠏", std::make_pair(E, ast::half_or_32th))
     (L"⠟", std::make_pair(F, ast::half_or_32th))
     (L"⠗", std::make_pair(G, ast::half_or_32th))
     (L"⠎", std::make_pair(A, ast::half_or_32th))
     (L"⠞", std::make_pair(B, ast::half_or_32th))
     (L"⠹", std::make_pair(C, ast::quarter_or_64th))
     (L"⠱", std::make_pair(D, ast::quarter_or_64th))
     (L"⠫", std::make_pair(E, ast::quarter_or_64th))
     (L"⠻", std::make_pair(F, ast::quarter_or_64th))
     (L"⠳", std::make_pair(G, ast::quarter_or_64th))
     (L"⠪", std::make_pair(A, ast::quarter_or_64th))
     (L"⠺", std::make_pair(B, ast::quarter_or_64th))
     (L"⠙", std::make_pair(C, ast::eighth_or_128th))
     (L"⠑", std::make_pair(D, ast::eighth_or_128th))
     (L"⠋", std::make_pair(E, ast::eighth_or_128th))
     (L"⠛", std::make_pair(F, ast::eighth_or_128th))
     (L"⠓", std::make_pair(G, ast::eighth_or_128th))
     (L"⠊", std::make_pair(A, ast::eighth_or_128th))
     (L"⠚", std::make_pair(B, ast::eighth_or_128th))
     ;
}

step_and_value_symbols step_and_value_sign;

rest_symbols::rest_symbols()
{
  add(L"⠍", ast::value(ast::whole_or_16th))
     (L"⠥", ast::value(ast::half_or_32th))
     (L"⠧", ast::value(ast::quarter_or_64th))
     (L"⠭", ast::value(ast::eighth_or_128th))
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
  add(L"⠣⠂", ast::value_distinction::distinct)
     (L"⠠⠣⠂", ast::value_distinction::small_follows)
     (L"⠘⠣⠂", ast::value_distinction::large_follows)
     ;
}

value_distinction_symbols value_distinction_sign;

slur_symbols::slur_symbols()
{
  add(L"⠉",  ast::slur::type::single)
     (L"⠐⠉", ast::slur::type::cross_staff)
     ;
}

slur_symbols slur_sign;

tie_symbols::tie_symbols()
{
  add(L"⠈⠉", ast::tie::type::single)
     (L"⠨⠉", ast::tie::type::chord)
     (L"⠘⠉", ast::tie::type::arpeggio)
     ;
}

tie_symbols tie_sign;

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

barline_symbols::barline_symbols()
{
  add(L"⠣⠶", ast::begin_repeat)
     (L"⠣⠆", ast::end_repeat)
     (L"⠣⠅⠄", ast::end_part)
     ;
}

barline_symbols barline_sign;

stem_symbols::stem_symbols()
{
  add(L"⠸⠄", rational(1, 1))
     (L"⠸⠅", rational(1, 2))
     (L"⠸⠁", rational(1, 4))
     ;
}

stem_symbols stem_sign;

}}

