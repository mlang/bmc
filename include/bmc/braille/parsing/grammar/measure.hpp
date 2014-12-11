// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MEASURE_HPP
#define BMC_MEASURE_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "bmc/braille/ast/ast.hpp"
#include "bmc/braille/parsing/grammar/numbers.hpp"
#include "bmc/braille/parsing/grammar/partial_voice_sign.hpp"
#include "bmc/braille/parsing/error_handler.hpp"

namespace bmc { namespace braille {

/**
 * \brief A grammar for parsing a single braille music measure.
 *
 * \ingroup grammar
 */
template <typename Iterator>
struct measure_grammar : boost::spirit::qi::grammar<Iterator, ast::measure()>
{
  measure_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ast::measure()> start;
  boost::spirit::qi::rule<Iterator, ast::voice()> voice;
  boost::spirit::qi::rule<Iterator, ast::partial_measure()> partial_measure;
  boost::spirit::qi::rule<Iterator, ast::partial_voice()> partial_voice;
  partial_voice_sign_grammar<Iterator> partial_voice_sign;
  boost::spirit::qi::rule<Iterator>
  full_measure_in_accord, partial_measure_sign, partial_measure_in_accord,
  optional_dot;
  upper_number_grammar<Iterator> upper_number;
  lower_number_grammar<Iterator> lower_number;
  boost::spirit::qi::rule<Iterator, unsigned()> ending;
};

}}

#endif
