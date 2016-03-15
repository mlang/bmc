// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_PARTIAL_VOICE_SIGN_HPP
#define BMC_PARTIAL_VOICE_SIGN_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "bmc/braille/ast/ast.hpp"
#include "bmc/braille/parsing/grammar/numbers.hpp"
#include "bmc/braille/parsing/grammar/simile.hpp"
#include "bmc/braille/parsing/grammar/tuplet_start.hpp"
#include "bmc/braille/parsing/error_handler.hpp"

namespace bmc { namespace braille {

/**
 * \brief A grammar for parsing a single braille music measure.
 *
 * \ingroup grammar
 */
template <typename Iterator>
struct partial_voice_sign_grammar : boost::spirit::qi::grammar<Iterator, ast::sign()>
{
  partial_voice_sign_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ast::sign()> start;
  boost::spirit::qi::rule<Iterator, ast::note()> note;
  boost::spirit::qi::rule<Iterator, ast::stem()> stem;
  boost::spirit::qi::rule<Iterator, bool()> added_by_transcriber;
  boost::spirit::qi::rule<Iterator, ast::rest()> rest;
  boost::spirit::qi::rule<Iterator, ast::chord()> chord;
  boost::spirit::qi::rule<Iterator, ast::moving_note()> moving_note;
  boost::spirit::qi::rule<Iterator, std::vector<ast::interval>()> moving_intervals;
  boost::spirit::qi::rule<Iterator, ast::interval()> interval;
  boost::spirit::qi::rule<Iterator, unsigned()> finger_sign;
  boost::spirit::qi::rule<Iterator, braille::finger_change()> finger_change;
  boost::spirit::qi::rule<Iterator, braille::fingering_list()> fingering;
  tuplet_start_grammar<Iterator> tuplet_start;
  boost::spirit::qi::rule<Iterator, ast::hand_sign()> hand_sign;
  boost::spirit::qi::rule<Iterator, ast::clef()> clef;
  simile_grammar<Iterator> simile;
  boost::spirit::qi::rule<Iterator, unsigned()> dots;
  boost::spirit::qi::rule<Iterator, ast::slur()> slur;
  boost::spirit::qi::rule<Iterator, ast::tie()> tie, simple_tie;
  boost::spirit::qi::rule<Iterator> chord_tied_sign;
  boost::spirit::qi::rule<Iterator, ast::value_prefix()> value_prefix;
  boost::spirit::qi::rule<Iterator, ast::hyphen()> hyphen;
  boost::spirit::qi::rule<Iterator> optional_dot;
};

}}

#endif
