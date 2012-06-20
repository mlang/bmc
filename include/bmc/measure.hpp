// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef MEASURE_HPP
#define MEASURE_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "bmc/ast.hpp"
#include "numbers.hpp"
#include "bmc/error_handler.hpp"

namespace music { namespace braille {

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
  boost::spirit::qi::rule<Iterator, ast::note()> note;
  boost::spirit::qi::rule<Iterator, ast::rest()> rest;
  boost::spirit::qi::rule<Iterator, ast::chord()> chord;
  boost::spirit::qi::rule<Iterator, ast::interval()> interval;
  boost::spirit::qi::rule<Iterator, braille::finger_change()> finger_change;
  boost::spirit::qi::rule<Iterator, braille::fingering_list()> fingering;
  boost::spirit::qi::rule<Iterator, braille::hand_sign()> hand_sign;
  boost::spirit::qi::rule<Iterator, ast::simile()> simile;
  boost::spirit::qi::rule<Iterator, unsigned()> dots;
  boost::spirit::qi::rule<Iterator, ast::slur()> slur_sign;
  boost::spirit::qi::rule<Iterator, ast::tie()> tie_sign;
  boost::spirit::qi::rule<Iterator, ast::value_distinction()> value_distinction;
  boost::spirit::qi::rule<Iterator>
  full_measure_in_accord, partial_measure_sign, partial_measure_in_accord,
  optional_dot, whitespace, newline;
  lower_number_grammar<Iterator> lower_number;
  boost::spirit::qi::rule<Iterator, unsigned()> ending;
};

}}

#endif
