// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "numbers.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "brlsym.hpp"
#include "spirit/qi/primitive/brl.hpp"

namespace music { namespace braille {

template <typename Iterator>
upper_number_grammar<Iterator>::upper_number_grammar()
: upper_number_grammar::base_type(start)
{
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_val_type _val;

  start = eps[_val = 0] >> +upper_digit_sign[_val = _val * 10 + _1];
}

template <typename Iterator>
lower_number_grammar<Iterator>::lower_number_grammar()
: lower_number_grammar::base_type(start)
{
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_val_type _val;

  start = eps[_val = 0] >> +lower_digit_sign[_val = _val * 10 + _1];
}

template <typename Iterator>
key_signature_grammar<Iterator>::key_signature_grammar()
: key_signature_grammar::base_type(start)
{
  boost::spirit::qi::_pass_type _pass;
  boost::spirit::qi::_val_type _val;
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::eps_type eps;
  boost::spirit::qi::attr_type attr;
  boost::spirit::qi::repeat_type repeat;
  using boost::phoenix::size;
  brl_type brl;

  start = repeat(1, 3)[sharp_sign] [_val = size(_1)]
        | brl(3456) >> upper_number[_pass = _1 > unsigned(2),
                                    _val = _1] >> sharp_sign
        | repeat(1, 3)[flat_sign]  [_val = -size(_1)]
        | brl(3456) >> upper_number[_pass = _1 > unsigned(2),
                                    _val = -_1] >> flat_sign
        | eps                        [_val = 0]
        ;

  flat_sign = brl(126) >> attr(-1);
  sharp_sign = brl(146) >> attr(1);
}

template <typename Iterator>
time_signature_grammar<Iterator>::time_signature_grammar()
: time_signature_grammar::base_type(start)
{
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_a_type _a;
  boost::spirit::qi::_val_type _val;
  music::braille::brl_type brl;
  using boost::phoenix::construct;
  start = brl(3456)
       >> upper_number[_a = _1]
       >> lower_number[_val = construct<music::time_signature>(_a, _1)]
        ;
}

}}
