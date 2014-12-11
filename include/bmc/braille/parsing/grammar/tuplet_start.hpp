// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_TUPLET_START_HPP
#define BMC_TUPLET_START_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "bmc/braille/ast/ast.hpp"
#include "bmc/braille/parsing/grammar/numbers.hpp"
#include "bmc/braille/parsing/error_handler.hpp"

namespace bmc { namespace braille {

/**
 * \brief A grammar for parsing a single braille music measure.
 *
 * \ingroup grammar
 */
template <typename Iterator>
struct tuplet_start_grammar : boost::spirit::qi::grammar<Iterator, ast::tuplet_start(), boost::spirit::qi::locals<unsigned>>
{
  tuplet_start_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ast::tuplet_start(), boost::spirit::qi::locals<unsigned>> start;
  lower_number_grammar<Iterator> lower_number;
};

}}

#endif
