// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "numbers.hpp"
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_attr.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
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

}}
