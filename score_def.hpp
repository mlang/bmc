#ifndef BMC_SCORE_DEF_HPP
#define BMC_SCORE_DEF_HPP
#include "score.hpp"
#include "brl.hpp"
#include "brlsym.hpp"
#include "annotation.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

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

  start = +(keyboard_part | solo_part);
  boost::spirit::standard_wide::space_type space;
  boost::spirit::qi::eol_type eol;
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_val_type _val;
  solo_part = staff >> eom >> -eol;
  keyboard_paragraph = eps[resize(_val, 2)]
  >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  >> eol
  >> left_hand_sign
  >> staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  >> eol
  ;
  keyboard_part = eps[resize(_val, 2)]
  >> *keyboard_paragraph[insert(front(_val), end(front(_val)),
                                begin(front(_1)), end(front(_1))),
                         insert(back(_val), end(back(_val)),
                                begin(back(_1)), end(back(_1)))]
  >> right_hand_sign
  >> staff[insert(front(_val), end(front(_val)), begin(_1), end(_1))]
  >> eom >> eol
  >> left_hand_sign
  >> staff[insert(back(_val), end(back(_val)), begin(_1), end(_1))]
  >> eom >> -eol
  ;
  staff = measure % (space | eol);

  music::braille::brl_type brl;
  right_hand_sign = brl(46) >> brl(345) > optional_dot;
  left_hand_sign = brl(456) >> brl(345) > optional_dot;
  eom = brl(126) >> brl(13);
  optional_dot = !dots_123 | &(brl(3) >> dots_123) > brl(3);
}

}}

#endif
