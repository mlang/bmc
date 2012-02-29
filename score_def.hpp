// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_SCORE_DEF_HPP
#define BMC_SCORE_DEF_HPP
#include "score.hpp"
#include "spirit/qi/primitive/brl.hpp"
#include "brlsym.hpp"
#include "annotation.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
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
  typedef boost::phoenix::function< annotation<Iterator> >
          annotation_function;
  boost::spirit::qi::eol_type eol;
  boost::spirit::qi::omit_type omit;
  boost::spirit::standard_wide::space_type space;

  start = key_signature >> -time_signature >> -+eol
       >> +(keyboard_part | solo_part);
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_2_type _2;
  boost::spirit::qi::_3_type _3;
  boost::spirit::qi::_4_type _4;
  boost::spirit::qi::_val_type _val;
  solo_part = eps[resize(_val, 1)]
           >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
           > eom > -eol;
  keyboard_paragraph = eps[resize(_val, 2)]
  >> space >> space >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  >> eol
  >> space >> space >> left_hand_sign
  > staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  > eol
  ;
  keyboard_part = eps[resize(_val, 2)]
  >> *keyboard_paragraph[insert(front(_val), end(front(_val)),
                                begin(front(_1)), end(front(_1))),
                         insert(back(_val), end(back(_val)),
                                begin(back(_1)), end(back(_1)))]
  >> space >> space >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  > eom > eol
  > space > space > left_hand_sign
  > staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  > eom > -eol
  ;
  music::braille::brl_type brl;
  staff = measure % (space | brl(0) | eol);

  right_hand_sign = brl(46) >> brl(345) > optional_dot;
  left_hand_sign = brl(456) >> brl(345) > optional_dot;
  eom = brl(126) >> brl(13);
  optional_dot = !dots_123 | &(brl(3) >> dots_123) > brl(3);


  boost::spirit::qi::on_error<boost::spirit::qi::fail>(start,
    error_handler_function(error_handler)
    (L"error: expecting ", _4, _3));
}

}}

#endif
