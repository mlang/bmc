// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_SCORE_DEF_HPP
#define BMC_SCORE_DEF_HPP
#include "bmc/braille/parsing/grammar/score.hpp"
#include "bmc/braille/ast/fusion_adapt.hpp"
#include "spirit/qi/primitive/brl.hpp"
#include "brlsym.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include "spirit/detail/info_wchar_t_io.hpp"

namespace music { namespace braille {

template<typename Iterator>
score_grammar<Iterator>::score_grammar(error_handler<Iterator>& error_handler)
: score_grammar::base_type(start, "score")
, measure(error_handler)
{
  using boost::phoenix::back;
  using boost::phoenix::begin;
  using boost::phoenix::end;
  using boost::phoenix::front;
  using boost::phoenix::insert;
  using boost::phoenix::resize;
  typedef boost::phoenix::function< braille::error_handler<Iterator> >
          error_handler_function;
  boost::spirit::qi::eol_type eol;
  music::braille::brl_type brl;
  boost::spirit::standard_wide::space_type space;
  whitespace = space | brl(0);
  indent = whitespace >> whitespace >> -whitespace;

  start = key_signature >> -time_signature >> -+eol
       >> +(keyboard_part | solo_part);
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_3_type _3;
  boost::spirit::qi::_4_type _4;
  boost::spirit::qi::_val_type _val;
  solo_part = eps[resize(_val, 1)]
           >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
           > eom > -eol;
  keyboard_paragraph = eps[resize(_val, 2)]
  >> indent >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  >> eol
  >> indent >> left_hand_sign
  > staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  > eol
  ;
  keyboard_part = eps[resize(_val, 2)]
  >> *keyboard_paragraph[insert(front(_val), end(front(_val)),
                                begin(front(_1)), end(front(_1))),
                         insert(back(_val), end(back(_val)),
                                begin(back(_1)), end(back(_1)))]
  >> indent >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  > eom > eol
  > indent > left_hand_sign
  > staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  > eom > *eol
  ;

  staff = (key_and_time_signature | measure) % (whitespace | eol);
  key_and_time_signature = key_signature >> time_signature;
  //global_key_and_time_signature = key_signature >> time_signature >> *(brl(5) >> brl(2) >> time_signature);

  right_hand_sign = brl(46) >> brl(345) > optional_dot;
  left_hand_sign = brl(456) >> brl(345) > optional_dot;
  eom = brl(126) >> brl(13) >> !brl(3);
  optional_dot = (!dots_123) | (&(brl(3) >> dots_123) > brl(3));


  boost::spirit::qi::on_error<boost::spirit::qi::fail>(start,
    error_handler_function(error_handler)
    (L"error: expecting ", _4, _3));
}

}}

#endif
