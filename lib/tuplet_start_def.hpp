// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_TUPLET_START_DEF_HPP
#define BMC_TUPLET_START_DEF_HPP

#include <bmc/braille/parsing/grammar/tuplet_start.hpp>
#include <bmc/braille/ast/fusion_adapt.hpp>
#include <bmc/braille/parsing/qi/primitive/brl.hpp>
#include "brlsym.hpp"
#include <bmc/braille/parsing/error_handler.hpp>
#include <bmc/braille/parsing/annotation.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace bmc { namespace braille {

template<typename Iterator>
tuplet_start_grammar<Iterator>::tuplet_start_grammar(error_handler<Iterator>& error_handler)
: tuplet_start_grammar::base_type(start, "measure")
{
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;

  ::bmc::braille::brl_type brl;
  boost::spirit::qi::_a_type _a;
  boost::spirit::_1_type _1;
  boost::spirit::_3_type _3;
  boost::spirit::_val_type _val;
  boost::spirit::qi::_pass_type _pass;
  using boost::phoenix::construct;

  start = brl(23) >> brl(23)[_val = construct<ast::tuplet_start>(true)]
        | brl(23)[_val = construct<ast::tuplet_start>(false)]
        | brl(456) >> lower_number[_a = _1]
       >> brl(456) >> lower_number[_pass = _a == _1]
       >> brl(3)[_val = construct<ast::tuplet_start>(_a, true)]
        | brl(456) >> lower_number[_a = _1]
       >> brl(3)[_val = construct<ast::tuplet_start>(_a, false)]
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
