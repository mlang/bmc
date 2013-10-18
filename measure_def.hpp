// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MEASURE_DEF_HPP
#define BMC_MEASURE_DEF_HPP

#include "bmc/braille/parsing/grammar/measure.hpp"
#include "bmc/braille/ast/fusion_adapt.hpp"
#include "spirit/qi/primitive/brl.hpp"
#include "brlsym.hpp"
#include "bmc/braille/parsing/error_handler.hpp"
#include "bmc/braille/parsing/annotation.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/phoenix/function/function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/phoenix/operator/arithmetic.hpp>
#include <boost/phoenix/operator/self.hpp>
#include <boost/phoenix/statement/sequence.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace music { namespace braille {

template<typename Iterator>
measure_grammar<Iterator>::measure_grammar(error_handler<Iterator>& error_handler)
: measure_grammar::base_type(start, "measure")
{
  using boost::phoenix::begin;
  using boost::phoenix::end;
  using boost::phoenix::insert;

  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;

  music::braille::brl_type brl;
  boost::spirit::qi::_a_type _a;

  start = -ending >> (voice % full_measure_in_accord);

  voice = partial_measure % partial_measure_sign;
  partial_measure = partial_voice % partial_measure_in_accord;
  partial_voice = +( newline
                   | moving_note | chord | note | rest
                   | value_distinction | tie
                   | hand_sign
                   | simile
                   | barline_sign
                   )
                ;

  boost::spirit::_val_type _val;
  boost::spirit::_1_type _1;
  boost::spirit::_2_type _2;
  boost::spirit::repeat_type repeat;
  using boost::phoenix::at_c;
  note = (*articulation_sign)   [at_c<0>(_val) = _1]
      >> (-accidental_sign)     [at_c<1>(_val) = _1]
      >> (-octave_sign)         [at_c<2>(_val) = _1]
      >> step_and_value_sign    [at_c<3>(_val) = at_c<0>(_1),
                                 at_c<4>(_val) = at_c<1>(_1)]
      >> dots                   [at_c<5>(_val) = _1]
      >> (repeat(0, 2)[slur])   [at_c<6>(_val) = _1]
      >> fingering              [at_c<7>(_val) = _1]
      >> repeat(0, 2)[slur]     [insert(at_c<6>(_val), end(at_c<6>(_val)), begin(_1), end(_1))]
      >> (-simple_tie)          [at_c<8>(_val) = _1]
      >> (*stem)                [at_c<9>(_val) = _1]
       ;

  stem = stem_sign >> dots >> -tie;

  rest = -brl(6) >> rest_sign >> dots >> -(brl(5) >> brl(14));

  chord_tied_sign = brl(46) >> brl(14);
  chord = note >> +interval >> -chord_tied_sign;
  moving_note = note >> moving_intervals;
  moving_intervals = +(interval >> brl(6)) >> interval;
  interval = -accidental_sign
          >> -octave_sign
          >> interval_sign
          >> fingering
          >> -simple_tie;

  slur = slur_sign;
  tie = tie_sign;

  value_distinction = value_distinction_sign;

  finger_change = finger_sign >> brl(14) >> finger_sign;
  fingering = *(finger_change | finger_sign);

  boost::spirit::standard_wide::space_type space;
  boost::spirit::eol_type eol;
  boost::spirit::eps_type eps;
  boost::spirit::attr_type attr;
  simple_tie = brl(4) >> brl(14) >> attr(ast::tie::single);
  simile = eps[_a = 0]
        >> (-octave_sign)[at_c<0>(_val) = _1]
        >> +(brl(2356)[_a += 1])
        >> -(brl(3456) >> upper_number[_a = _1])
        >> eps[at_c<1>(_val) = _a];

  dots = eps[_val = 0] >> *(brl(3)[_val += 1]);

  full_measure_in_accord = brl(126) >> brl(345) >> -+eol;
  partial_measure_sign = brl(46) >> brl(13) >> -+eol;
  partial_measure_in_accord = brl(5) >> brl(2) >> -+eol;
  optional_dot = (!dots_123) | (&(brl(3) >> dots_123) > brl(3));
  hand_sign = (brl(46) >> brl(345) > optional_dot > attr(braille::right_hand))
            | (brl(456) >> brl(345) > optional_dot > attr(braille::left_hand));
  ending = brl(3456) >> lower_digit_sign > optional_dot;

  newline = brl(5) >> eol;
#define BMC_LOCATABLE_SET_ID(rule) \
  boost::spirit::qi::on_success(rule,\
                                annotation_function(error_handler.iters)\
                                (_val, _1, _2))
  BMC_LOCATABLE_SET_ID(start);
  BMC_LOCATABLE_SET_ID(voice);
  BMC_LOCATABLE_SET_ID(partial_measure);
  BMC_LOCATABLE_SET_ID(partial_voice);
  BMC_LOCATABLE_SET_ID(note);
  BMC_LOCATABLE_SET_ID(rest);
  BMC_LOCATABLE_SET_ID(interval);
  BMC_LOCATABLE_SET_ID(chord);
  BMC_LOCATABLE_SET_ID(value_distinction);
  BMC_LOCATABLE_SET_ID(simile);
  BMC_LOCATABLE_SET_ID(tie);
#undef BMC_LOCATABLE_SET_ID
  
  note.name("note");
  interval.name("interval");
  fingering.name("fingering");
}

}}

#endif
