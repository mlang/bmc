#ifndef BRLSYM_HPP
#define BRLSYM_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_symbols.hpp>
#include "ambiguous.hpp"
#include "ttb.h"
#include "unicode.h"

namespace music { namespace braille {

struct tst_braillify {
  template <typename Char> Char operator()(Char ch) const {
    return UNICODE_BRAILLE_ROW | (convertCharacterToDots(textTable, ch)&0X3F);
  }
};

template <typename T = boost::spirit::unused_type>
struct brl_symbols : boost::spirit::qi::symbols<wchar_t, T, boost::spirit::qi::tst<wchar_t, T>, tst_braillify> {};

struct upper_digit_symbols : brl_symbols<unsigned>
{
  upper_digit_symbols();
};
extern upper_digit_symbols upper_digit_sign;

struct lower_digit_symbols : brl_symbols<unsigned>
{
  lower_digit_symbols();
};
extern lower_digit_symbols lower_digit_sign;

struct dots_123_symbols : brl_symbols<>
{
  dots_123_symbols();
};
extern dots_123_symbols dots_123;

struct accidental_symbols : brl_symbols<accidental>
{
  accidental_symbols();
};
extern accidental_symbols accidental_sign;

struct octave_symbols : brl_symbols<unsigned>
{
  octave_symbols();
};
extern octave_symbols octave_sign;

struct pitch_and_value_symbols : brl_symbols<ambiguous::pitch_and_value>
{
  pitch_and_value_symbols();
};
extern pitch_and_value_symbols pitch_and_value_sign;

struct rest_symbols : brl_symbols<ambiguous::value>
{
  rest_symbols();
};
extern rest_symbols rest_sign;

struct interval_symbols : brl_symbols<interval>
{
  interval_symbols();
};
extern interval_symbols interval_sign;

struct finger_symbols : brl_symbols<unsigned>
{
  finger_symbols();
};
extern finger_symbols finger_sign;

}}

#endif
