// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MEASURE_DEF_HPP
#define BMC_MEASURE_DEF_HPP

#include <bmc/braille/parsing/grammar/measure.hpp>
#include <bmc/braille/ast/fusion_adapt.hpp>
#include <bmc/braille/parsing/qi/primitive/brl.hpp>
#include "brlsym.hpp"
#include <bmc/braille/parsing/error_handler.hpp>
#include <bmc/braille/parsing/annotation.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/phoenix/statement/sequence.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace bmc { namespace braille {

template<typename Iterator>
measure_grammar<Iterator>::measure_grammar(error_handler<Iterator>& error_handler)
: measure_grammar::base_type(start, "measure")
, partial_voice_sign(error_handler)
{
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;

  ::bmc::braille::brl_type brl;

  start = -ending >> (voice % full_measure_in_accord);

  voice = partial_measure % partial_measure_sign;
  partial_measure = partial_voice % partial_measure_in_accord;
  partial_voice = +partial_voice_sign;

  boost::spirit::_val_type _val;
  boost::spirit::_1_type _1;
  boost::spirit::_3_type _3;
  boost::spirit::repeat_type repeat;
  using boost::phoenix::at_c;
  boost::spirit::eol_type eol;

  full_measure_in_accord = brl(126) >> brl(345) >> *eol;
  partial_measure_sign = brl(46) >> brl(13) >> *eol;
  partial_measure_in_accord = brl(5) >> brl(2) >> *eol;

  optional_dot = (!dots_123) | (&(brl(3) >> dots_123) > brl(3));
  ending = (brl(3456) >> lower_digit_sign) > optional_dot;

#define BMC_LOCATABLE_SET_ID(rule) \
  boost::spirit::qi::on_success(rule,\
                                annotation_function(error_handler.ranges)\
                                (_val, _1, _3))
  BMC_LOCATABLE_SET_ID(start);
  BMC_LOCATABLE_SET_ID(voice);
  BMC_LOCATABLE_SET_ID(partial_measure);
  BMC_LOCATABLE_SET_ID(partial_voice);
#undef BMC_LOCATABLE_SET_ID
  
  optional_dot.name(".");
}

}}

#endif
