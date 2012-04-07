// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BMC_KEY_SIGNATURE_HPP)
#define BMC_KEY_SIGNATURE_HPP

#include "config.hpp"
#include "music.hpp"
#include "numbers.hpp"
#include <boost/spirit/include/qi_grammar.hpp>

namespace music { namespace braille {

/**
 * \brief A grammar for parsing a braille key signature.
 *
 * \ingroup grammar
 */
template<typename Iterator>
struct key_signature_grammar : boost::spirit::qi::grammar<Iterator, music::key_signature()>
{
  key_signature_grammar();

  boost::spirit::qi::rule<Iterator, music::key_signature()> start;
  boost::spirit::qi::rule<Iterator, int()> flat_sign, sharp_sign;
  upper_number_grammar<Iterator> upper_number;
};

}}

#endif

