// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BMC_NUMBERS_HPP)
#define BMC_NUMBERS_HPP

#include "config.hpp"
#include "music.hpp"
#include <boost/spirit/include/qi_grammar.hpp>

namespace music { namespace braille {

template <typename Iterator>
struct upper_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  upper_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

template <typename Iterator>
struct lower_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  lower_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

template<typename Iterator>
struct key_signature_grammar : boost::spirit::qi::grammar<Iterator, key_signature()>
{
  key_signature_grammar();

  boost::spirit::qi::rule<Iterator, key_signature()> start;
  boost::spirit::qi::rule<Iterator, int()> flat_sign, sharp_sign;
  upper_number_grammar<Iterator> upper_number;
};

}}

#endif

