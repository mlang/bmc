// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_SIMILE_DEF_HPP
#define BMC_SIMILE_DEF_HPP

#include <bmc/braille/parsing/grammar/simile.hpp>
#include <bmc/braille/ast/fusion_adapt.hpp>
#include <bmc/braille/parsing/qi/primitive/brl.hpp>
#include "brlsym.hpp"
#include <bmc/braille/parsing/error_handler.hpp>
#include <bmc/braille/parsing/annotation.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace bmc { namespace braille {

template<typename Iterator>
simile_grammar<Iterator>::simile_grammar(error_handler<Iterator>& error_handler)
: simile_grammar::base_type(start, "partial_voice_sign")
{
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;

  ::bmc::braille::brl_type brl;
  boost::spirit::qi::_a_type _a;
  boost::spirit::_1_type _1;
  boost::spirit::_3_type _3;
  boost::spirit::_val_type _val;
  using boost::phoenix::at_c;

  boost::spirit::eps_type eps;
  start = eps[_a = 0]
       >> (-octave_sign)[at_c<0>(_val) = _1]
       >> +(brl(2356)[_a += 1])
       >> -(brl(3456) >> upper_number[_a = _1])
       >> eps[at_c<1>(_val) = _a]
        ;

#define BMC_LOCATABLE_SET_ID(rule) \
  boost::spirit::qi::on_success(rule,\
                                annotation_function(error_handler.ranges)\
                                (_val, _1, _3))
  BMC_LOCATABLE_SET_ID(start);
#undef BMC_LOCATABLE_SET_ID
}

}}

#endif
