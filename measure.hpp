// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef MEASURE_HPP
#define MEASURE_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "ambiguous.hpp"
#include "numbers.hpp"
#include "error_handler.hpp"

namespace music { namespace braille {

template <typename Iterator>
struct measure_grammar : boost::spirit::qi::grammar<Iterator, ambiguous::measure()>
{
  measure_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ambiguous::measure()> start;
  boost::spirit::qi::rule<Iterator, ambiguous::voice()> fmia;
  boost::spirit::qi::rule<Iterator, ambiguous::partial_measure()> pmia;
  boost::spirit::qi::rule<Iterator, ambiguous::partial_voice()> pmia_voice;
  boost::spirit::qi::rule<Iterator, ambiguous::note()> note;
  boost::spirit::qi::rule<Iterator, ambiguous::rest()> rest;
  boost::spirit::qi::rule<Iterator, ambiguous::chord()> chord;
  boost::spirit::qi::rule<Iterator, ambiguous::interval()> interval;
  boost::spirit::qi::rule<Iterator, ambiguous::finger_change()> finger_change;
  boost::spirit::qi::rule<Iterator, std::list<ambiguous::fingering>()> fingering;
  boost::spirit::qi::rule<Iterator, ambiguous::hand_sign()> hand_sign;
  boost::spirit::qi::rule<Iterator, ambiguous::simile()> simile;
  boost::spirit::qi::rule<Iterator, unsigned()> dots;
  boost::spirit::qi::rule<Iterator, ambiguous::slur()> slur_sign;
  boost::spirit::qi::rule<Iterator>
  fmia_separator, pmia_sign, pmia_voice_sign, optional_dot, whitespace, newline;
  lower_number_grammar<Iterator> lower_number;
  boost::spirit::qi::rule<Iterator, unsigned()> ending;
};

}}

#endif
