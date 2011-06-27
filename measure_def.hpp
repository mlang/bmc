#ifndef MEASURE_DEF_HPP
#define MEASURE_DEF_HPP
#include "measure.hpp"
#include "brl.hpp"
#include "brlsym.hpp"
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

template <typename Iterator>
measure_grammar<Iterator>::measure_grammar()
  : measure_grammar::base_type(start, "measure")
{
  start = fmia % fmia_separator;
  fmia = pmia % pmia_sign;
  pmia = pmia_voice % pmia_voice_sign;
  pmia_voice = +(chord | note | rest);

  note = -accidental_sign
      >> -octave_sign
      >> pitch_and_value_sign
      >> dots
      >> -finger_sign;
       ;
  rest = rest_sign >> dots;
  chord = note >> +interval;
  interval = -accidental_sign >> -octave_sign >> interval_sign >> finger_sign;

  boost::spirit::standard_wide::space_type space;
  boost::spirit::eol_type eol;
  boost::spirit::eps_type eps;
  boost::spirit::_val_type _val;
  music::braille::brl_type brl;
  whitespace = space | brl(0);
  dots = eps[_val = 0] >> *(brl(3)[_val += 1]);

  fmia_separator = brl(126) >> brl(345) >> -(+eol >> *whitespace);
  pmia_sign = brl(46) >> brl(13) >> -(+eol >> *whitespace);
  pmia_voice_sign = brl(5) >> brl(2) >> -(+eol >> *whitespace);
}

}}

#endif
