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

/**
 * \brief A grammar for parsing a sequence of braille digits written in the
 *        upper part (dots 1 2 4 5) of a 6-dot braille cell as a (unsigned) number.
 *
 * \ingroup grammar
 * \see lower_number_grammar
 */
template <typename Iterator>
struct upper_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  upper_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

/**
 * \brief A grammar for parsing a sequence of braille digits written in the
 *        lower part (dots 2 3 5 6) of a 6-dot braille cell as a (unsigned) number.
 *
 * \ingroup grammar
 * \see upper_number_grammar
 */
template <typename Iterator>
struct lower_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  lower_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

}}

#endif

