// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "brlsym.hpp"

namespace bmc { namespace braille {

upper_digit_symbols::upper_digit_symbols()
{
  add(L"\u281A", 0)
     (L"\u2801", 1) (L"\u2803", 2) (L"\u2809", 3)
     (L"\u2819", 4) (L"\u2811", 5) (L"\u280B", 6)
     (L"\u281B", 7) (L"\u2813", 8) (L"\u280A", 9)
     ;
}

upper_digit_symbols upper_digit_sign;

lower_digit_symbols::lower_digit_symbols()
{
  add(L"\u2834", 0)
     (L"\u2802", 1) (L"\u2806", 2) (L"\u2812", 3)
     (L"\u2832", 4) (L"\u2822", 5) (L"\u2816", 6)
     (L"\u2836", 7) (L"\u2826", 8) (L"\u2814", 9)
     ;
}

lower_digit_symbols lower_digit_sign;

dots_123_symbols::dots_123_symbols()
{
  for (int i = 1; i < 0X40; ++i)
    if (i & 0X07) {
      wchar_t const pattern[2] = { 0X2800 | i, 0 };
      add(pattern);
    }
}

dots_123_symbols dots_123;

accidental_symbols::accidental_symbols()
{
  add(L"\u2821", natural)
     (L"\u2823", flat)
     (L"\u2829", sharp)
     ;
}

accidental_symbols accidental_sign;

octave_symbols::octave_symbols()
{
  add(L"\u2808\u2808", 1)
     (L"\u2808", 2)
     (L"\u2818", 3)
     (L"\u2838", 4)
     (L"\u2810", 5)
     (L"\u2828", 6)
     (L"\u2830", 7)
     (L"\u2820", 8)
     (L"\u2820\u2820", 9)
     ;
}

octave_symbols octave_sign;

step_and_value_symbols::step_and_value_symbols()
{
  add(L"\u283D", std::make_pair(C, ast::whole_or_16th))
     (L"\u2835", std::make_pair(D, ast::whole_or_16th))
     (L"\u282F", std::make_pair(E, ast::whole_or_16th))
     (L"\u283F", std::make_pair(F, ast::whole_or_16th))
     (L"\u2837", std::make_pair(G, ast::whole_or_16th))
     (L"\u282E", std::make_pair(A, ast::whole_or_16th))
     (L"\u283E", std::make_pair(B, ast::whole_or_16th))
     (L"\u281D", std::make_pair(C, ast::half_or_32th))
     (L"\u2815", std::make_pair(D, ast::half_or_32th))
     (L"\u280F", std::make_pair(E, ast::half_or_32th))
     (L"\u281F", std::make_pair(F, ast::half_or_32th))
     (L"\u2817", std::make_pair(G, ast::half_or_32th))
     (L"\u280E", std::make_pair(A, ast::half_or_32th))
     (L"\u281E", std::make_pair(B, ast::half_or_32th))
     (L"\u2839", std::make_pair(C, ast::quarter_or_64th))
     (L"\u2831", std::make_pair(D, ast::quarter_or_64th))
     (L"\u282B", std::make_pair(E, ast::quarter_or_64th))
     (L"\u283B", std::make_pair(F, ast::quarter_or_64th))
     (L"\u2833", std::make_pair(G, ast::quarter_or_64th))
     (L"\u282A", std::make_pair(A, ast::quarter_or_64th))
     (L"\u283A", std::make_pair(B, ast::quarter_or_64th))
     (L"\u2819", std::make_pair(C, ast::eighth_or_128th))
     (L"\u2811", std::make_pair(D, ast::eighth_or_128th))
     (L"\u280B", std::make_pair(E, ast::eighth_or_128th))
     (L"\u281B", std::make_pair(F, ast::eighth_or_128th))
     (L"\u2813", std::make_pair(G, ast::eighth_or_128th))
     (L"\u280A", std::make_pair(A, ast::eighth_or_128th))
     (L"\u281A", std::make_pair(B, ast::eighth_or_128th))
     ;
}

step_and_value_symbols step_and_value_sign;

rest_symbols::rest_symbols()
{
  add(L"\u280D", ast::value(ast::whole_or_16th))
     (L"\u2825", ast::value(ast::half_or_32th))
     (L"\u2827", ast::value(ast::quarter_or_64th))
     (L"\u282D", ast::value(ast::eighth_or_128th))
     ;
}

rest_symbols rest_sign;

interval_symbols::interval_symbols()
{
  add(L"\u280C", second)
     (L"\u282C", third)
     (L"\u283C", fourth)
     (L"\u2814", fifth)
     (L"\u2834", sixth)
     (L"\u2812", seventh)
     (L"\u2824", octave)
     ;
}

interval_symbols interval_sign;

hand_symbols::hand_symbols()
{
  add(L"\u2828\u281C", ast::hand_sign::right_hand)
     (L"\u2838\u281C", ast::hand_sign::left_hand)
     ;
}

hand_symbols hand_sign;

slur_symbols::slur_symbols()
{
  add(L"\u2809",  ast::slur::type::single)
     (L"\u2810\u2809", ast::slur::type::cross_staff)
     ;
}

slur_symbols slur_sign;

tie_symbols::tie_symbols()
{
  add(L"\u2808\u2809", ast::tie::type::single)
     (L"\u2828\u2809", ast::tie::type::chord)
     (L"\u2818\u2809", ast::tie::type::arpeggio)
     ;
}

tie_symbols tie_sign;

articulation_symbols::articulation_symbols()
{
  add(L"\u2822", short_appoggiatura)
     (L"\u2810\u2822", appoggiatura)
     (L"\u2826", staccato)
     (L"\u2820\u2826", staccatissimo)
     (L"\u2810\u2826", mezzo_staccato)
     (L"\u2838\u2826", agogic_accent)
     (L"\u2828\u2826", accent)
     (L"\u2832", turn_between_notes)
     (L"\u2820\u2832", turn_above_or_below_note)
     (L"\u2832\u2807", inverted_turn_between_notes)
     (L"\u2820\u2832\u2807", inverted_turn_above_or_below_note)
     (L"\u2810\u2816", short_trill)
     (L"\u2830\u2816", extended_short_trill)
     (L"\u2810\u2816\u2807", mordent)
     (L"\u2830\u2816\u2807", extended_mordent)
     (L"\u281C\u2805", arpeggio_up)
     (L"\u2810\u281C\u2805", arpeggio_up_multi_staff)
     (L"\u281C\u2805\u2805", arpeggio_down)
     (L"\u2810\u281C\u2805\u2805", arpeggio_down_multi_staff)
     ;
}

articulation_symbols articulation_sign;

barline_symbols::barline_symbols()
{
  add(L"\u2823\u2836", ast::begin_repeat)
     (L"\u2823\u2806", ast::end_repeat)
     (L"\u2823\u2805\u2804", ast::end_part)
     ;
}

barline_symbols barline_sign;

stem_symbols::stem_symbols()
{
  add(L"\u2838\u2804", rational(1, 1))
     (L"\u2838\u2805", rational(1, 2))
     (L"\u2838\u2801", rational(1, 4))
     (L"\u2838\u2803", rational(1, 8))
     (L"\u2838\u2807", rational(1, 16))
     (L"\u2838\u2802", rational(1, 32))
     ;
}

stem_symbols stem_sign;

}}

