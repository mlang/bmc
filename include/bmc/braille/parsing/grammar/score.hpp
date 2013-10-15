// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_SCORE_HPP
#define BMC_SCORE_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "bmc/braille/ast/ast.hpp"
#include "bmc/braille/parsing/error_handler.hpp"
#include "bmc/braille/parsing/grammar/measure.hpp"
#include "bmc/braille/parsing/grammar/key_signature.hpp"
#include "bmc/braille/parsing/grammar/time_signature.hpp"

namespace music { namespace braille {

/**
 * \brief Top-level grammar for parsing braille music scores.
 *
 * \ingroup grammar
 */
template<typename Iterator>
struct score_grammar : boost::spirit::qi::grammar<Iterator, ast::score()>
{
  score_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ast::score()> start;
  boost::spirit::qi::rule<Iterator, ast::part()> solo_part, keyboard_part;
  boost::spirit::qi::rule<Iterator, ast::section()> keyboard_section, last_keyboard_section;
  boost::spirit::qi::rule<Iterator, ast::section()> solo_section, last_solo_section;
  boost::spirit::qi::rule<Iterator, ast::paragraph()> paragraph;
  boost::spirit::qi::rule<Iterator, ast::section::number_type()> section_number;
  boost::spirit::qi::rule<Iterator, ast::measure_range()> measure_range;
  boost::spirit::qi::rule<Iterator, ast::measure_specification()> measure_specification;
  measure_grammar<Iterator> measure;
  time_signature_grammar<Iterator> time_signature;
  key_signature_grammar<Iterator> key_signature;
  upper_number_grammar<Iterator> upper_number;
  lower_number_grammar<Iterator> lower_number;
  boost::spirit::qi::rule<Iterator, ast::key_and_time_signature()> key_and_time_signature;
  boost::spirit::qi::rule<Iterator> right_hand_sign, left_hand_sign;
  boost::spirit::qi::rule<Iterator> eom;
  boost::spirit::qi::rule<Iterator> optional_dot, whitespace, indent;
};

}}

#endif
