// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef MEASURE_DEF_HPP
#define MEASURE_DEF_HPP
#include "measure.hpp"
#include "spirit/qi/primitive/brl.hpp"
#include "brlsym.hpp"
#include "error_handler.hpp"
#include "annotation.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace music { namespace braille {

template<typename Iterator>
measure_grammar<Iterator>::measure_grammar(error_handler<Iterator>& error_handler)
: measure_grammar::base_type(start, "measure")
{
  typedef boost::phoenix::function< braille::error_handler<Iterator> >
          error_handler_function;
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;

  music::braille::brl_type brl;

  start = -ending >> (voice % full_measure_in_accord);

  voice = partial_measure % partial_measure_sign;
  partial_measure = partial_voice % partial_measure_in_accord;
  partial_voice = +( newline
                   | chord | note | rest
                   | value_distinction_sign
                   | hand_sign
                   | simile
                   | barline_sign
                   )
                ;

  boost::spirit::_val_type _val;
  boost::spirit::_1_type _1;
  boost::spirit::repeat_type repeat;
  using boost::phoenix::at_c;
  note = (*articulation_sign)   [at_c<0>(_val) = _1]
      >> -accidental_sign       [at_c<1>(_val) = _1]
      >> -octave_sign           [at_c<2>(_val) = _1]
      >> step_and_value_sign    [at_c<3>(_val) = at_c<0>(_1),
                                 at_c<4>(_val) = at_c<1>(_1)]
      >> dots                   [at_c<5>(_val) = _1]
      >> repeat(0, 2)[slur_sign][at_c<6>(_val) = _1]
      >> fingering              [at_c<7>(_val) = _1]
      >> repeat(0, 2)[slur_sign][at_c<6>(_val) = _1]
      >> (-tie_sign             [at_c<8>(_val) = true])
       ;

  rest = -brl(6) >> rest_sign >> dots >> -(brl(5) >> brl(14));
  chord = note >> +interval;
  interval = -accidental_sign >> -octave_sign >> interval_sign >> fingering;

  slur_sign = brl(14);

  finger_change = finger_sign >> brl(14) >> finger_sign;
  fingering = *(finger_change | finger_sign);

  boost::spirit::standard_wide::space_type space;
  boost::spirit::eol_type eol;
  boost::spirit::eps_type eps;
  boost::spirit::attr_type attr;
  simile = -octave_sign >> brl(2356);
  whitespace = space | brl(0);
  dots = eps[_val = 0] >> *(brl(3)[_val += 1]);

  full_measure_in_accord = brl(126) >> brl(345) >> -+eol;
  partial_measure_sign = brl(46) >> brl(13) >> -+eol;
  partial_measure_in_accord = brl(5) >> brl(2) >> -+eol;
  optional_dot = (!dots_123) | (&(brl(3) >> dots_123) > brl(3));
  hand_sign = (brl(46) >> brl(345) > optional_dot > attr(braille::right_hand))
            | (brl(456) >> brl(345) > optional_dot > attr(braille::left_hand));
  ending = brl(3456) >> lower_digit_sign > optional_dot;

  newline = brl(5) >> eol;
  boost::spirit::qi::on_success(start,
  				annotation_function(error_handler.iters)(_val, _1));
  boost::spirit::qi::on_success(note,
  				annotation_function(error_handler.iters)(_val, _1));
  boost::spirit::qi::on_success(rest,
  				annotation_function(error_handler.iters)(_val, _1));

  note.name("note");
  interval.name("interval");
  fingering.name("fingering");
}

}}

#endif
