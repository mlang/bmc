// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_SCORE_DEF_HPP
#define BMC_SCORE_DEF_HPP

#include <bmc/braille/parsing/grammar/score.hpp>
#include <bmc/braille/ast/fusion_adapt.hpp>
#include <bmc/braille/parsing/annotation.hpp>
#include <bmc/braille/parsing/qi/primitive/brl.hpp>
#include "brlsym.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include "spirit/detail/info_wchar_t_io.hpp"

namespace bmc { namespace braille {

template<typename Iterator>
score_grammar<Iterator>::score_grammar(error_handler<Iterator>& error_handler)
: score_grammar::base_type(start, "score")
, measure(error_handler)
{
  using boost::phoenix::at_c;
  using boost::phoenix::push_back;
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;
  typedef boost::phoenix::function< braille::error_handler<Iterator> >
          error_handler_function;
  boost::spirit::qi::eoi_type eoi;
  boost::spirit::qi::eol_type eol;
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_3_type _3;
  boost::spirit::qi::_4_type _4;
  boost::spirit::qi::_val_type _val;
  ::bmc::braille::brl_type brl;
  boost::spirit::standard_wide::blank_type blank;

  whitespace = blank | brl(0);
  indent = whitespace >> +whitespace;

  start = *whitespace
       >> key_signature
       >> -whitespace
       >> -(time_signature % (brl(5)>>brl(2)))
       >> *whitespace
       >> -+eol
       >> +(keyboard_part | solo_part)
        ;

  keyboard_section =
       (-(*whitespace >> key_and_time_signature >> *whitespace >> eol))[at_c<0>(_val) = _1]
    >> indent
    >> -section_number[at_c<1>(_val) = _1]
    >> -measure_range[at_c<2>(_val) = _1]
    >> right_hand_sign
    >> paragraph[push_back(at_c<3>(_val), _1)]
    >> eol
    >> indent
    >> left_hand_sign
    >> paragraph[push_back(at_c<3>(_val), _1)]
    >> eol
      ;

  last_keyboard_section =
       (-(*whitespace >> key_and_time_signature >> *whitespace >> eol))[at_c<0>(_val) = _1]
    >> indent
    >> -section_number[at_c<1>(_val) = _1]
    >> -measure_range[at_c<2>(_val) = _1]
    >> right_hand_sign
    >> paragraph[push_back(at_c<3>(_val), _1)]
    >> eom
    >> eol
    >> indent
    >> left_hand_sign
    >> paragraph[push_back(at_c<3>(_val), _1)]
    >> eom
    >> (eoi | +eol)
     ;

  keyboard_part = *keyboard_section >> last_keyboard_section;

  solo_section =
       -(*whitespace >> key_and_time_signature >> *whitespace >> eol)
    >> -indent
    >> -section_number
    >> -measure_range
    >> paragraph
    >> eol
     ;

  last_solo_section =
       -(*whitespace >> key_and_time_signature >> *whitespace >> eol)
    >> -indent
    >> -section_number
    >> -measure_range
    >> paragraph
    >> eom
    >> (eoi | +eol)
     ;

  solo_part = *solo_section >> last_solo_section;

  paragraph = (key_and_time_signature | measure) % (whitespace | eol);

  section_number = brl(3456) >> upper_number >> whitespace;
  measure_range =
       brl(3456)
    >> measure_specification >> brl(36) > measure_specification
    >> whitespace
     ;

  measure_specification = lower_number >> -(brl(3456) > lower_number);

  key_and_time_signature = key_signature >> time_signature;

  right_hand_sign = (brl(46) >> brl(345)) > optional_dot;
  left_hand_sign = (brl(456) >> brl(345)) > optional_dot;
  eom = brl(126) >> brl(13) >> !brl(3);
  optional_dot = (!dots_123) | (brl(3) > &dots_123);

#define BMC_LOCATABLE_SET_ID(rule) \
  boost::spirit::qi::on_success(rule,\
                                annotation_function(error_handler.ranges)\
                                (_val, _1, _3))
  BMC_LOCATABLE_SET_ID(measure_specification);
  BMC_LOCATABLE_SET_ID(measure_range);
  BMC_LOCATABLE_SET_ID(keyboard_section);
  BMC_LOCATABLE_SET_ID(last_keyboard_section);
  BMC_LOCATABLE_SET_ID(solo_section);
  BMC_LOCATABLE_SET_ID(last_solo_section);

  boost::spirit::qi::on_error<boost::spirit::qi::fail>(start,
    error_handler_function(error_handler)
    (L"error: expecting ", _4, _3));
}

}}

#endif
