#ifndef BMC_SCORE_HPP
#define BMC_SCORE_HPP

#include "config.hpp"
#include <boost/spirit/include/qi_grammar.hpp>
#include "ambiguous.hpp"
#include "error_handler.hpp"
#include "measure.hpp"
#include "numbers.hpp"

namespace music { namespace braille {

template<typename Iterator>
struct score_grammar : boost::spirit::qi::grammar<Iterator, ambiguous::score()>
{
  score_grammar(error_handler<Iterator>&);

  boost::spirit::qi::rule<Iterator, ambiguous::score()> start;
  boost::spirit::qi::rule<Iterator, ambiguous::part()> solo_part, keyboard_part;
  boost::spirit::qi::rule<Iterator, ambiguous::part()> keyboard_paragraph;
  boost::spirit::qi::rule<Iterator, ambiguous::staff()> staff;
  measure_grammar<Iterator> measure;
  time_signature_grammar<Iterator> time_signature;
  boost::spirit::qi::rule<Iterator> right_hand_sign, left_hand_sign;
  boost::spirit::qi::rule<Iterator> eom;
  boost::spirit::qi::rule<Iterator> optional_dot;
};

}}

#endif
