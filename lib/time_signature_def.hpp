// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <bmc/braille/parsing/grammar/time_signature.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include "brlsym.hpp"
#include <bmc/braille/parsing/qi/primitive/brl.hpp>

namespace bmc { namespace braille {

template <typename Iterator>
time_signature_grammar<Iterator>::time_signature_grammar()
: time_signature_grammar::base_type(start)
{
  boost::spirit::qi::_1_type _1;
  boost::spirit::qi::_a_type _a;
  boost::spirit::qi::_val_type _val;
  ::bmc::braille::brl_type brl;
  using boost::phoenix::construct;
  start = brl(3456)
       >> upper_number[_a = _1]
       >> lower_number[_val = construct<::bmc::time_signature>(_a, _1)]
        | brl(46)
       >> brl(14)[_val = construct<::bmc::time_signature>(4, 4)]
        | brl(456)
       >> brl(14)[_val = construct<::bmc::time_signature>(4, 4)]
        ;
}

}}
