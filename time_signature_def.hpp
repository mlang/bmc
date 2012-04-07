// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "time_signature.hpp"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "brlsym.hpp"
#include "spirit/qi/primitive/brl.hpp"

namespace music { namespace braille {

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
