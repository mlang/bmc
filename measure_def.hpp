#ifndef MEASURE_DEF_HPP
#define MEASURE_DEF_HPP
#include "measure.hpp"
#include "brl.hpp"
#include "brlsym.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace music { namespace braille {

template <typename Iterator>
measure_grammar<Iterator>::measure_grammar()
  : measure_grammar::base_type(start, "measure")
{
  start = fmia % fmia_separator;
  fmia = pmia % pmia_sign;
  pmia = pmia_voice % pmia_voice_sign;
  pmia_voice = +(chord | note | rest | value_distinction_sign | simile);

  boost::spirit::_val_type _val;
  boost::spirit::_1_type _1;
  using boost::phoenix::at_c;
  note = -accidental_sign    [at_c<0>(_val) = _1]
      >> -octave_sign        [at_c<1>(_val) = _1]
      >> pitch_and_value_sign[at_c<2>(_val) = at_c<0>(_1),
                              at_c<3>(_val) = at_c<1>(_1)]
      >> dots                [at_c<4>(_val) = _1]
      >> -finger_sign        [at_c<5>(_val) = _1]
       ;
  rest = rest_sign >> dots;
  chord = note >> +interval;
  interval = -accidental_sign >> -octave_sign >> interval_sign >> finger_sign;

  boost::spirit::standard_wide::space_type space;
  boost::spirit::eol_type eol;
  boost::spirit::eps_type eps;
  music::braille::brl_type brl;

  simile = -octave_sign >> brl(2356);
  whitespace = space | brl(0);
  dots = eps[_val = 0] >> *(brl(3)[_val += 1]);

  fmia_separator = brl(126) >> brl(345) >> -(+eol >> *whitespace);
  pmia_sign = brl(46) >> brl(13) >> -(+eol >> *whitespace);
  pmia_voice_sign = brl(5) >> brl(2) >> -(+eol >> *whitespace);
}

}}

#endif
