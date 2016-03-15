#if !defined(BMC_BRAILLE_PARSER_AST_ADAPTED_HPP)
#define BMC_BRAILLE_PARSER_AST_ADAPTED_HPP

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::time_signature,
  numerator, denominator
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::key_signature,
  fifths
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::clef,
  sign, line, other_staff
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::tuplet_start,
  (unsigned, number)
  (bool, simple_triplet)
  (bool, doubled)
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::note,
  articulations, acc, octave_spec, step, ambiguous_value, dots,
  fingers
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::rest,
  by_transcriber, ambiguous_value, dots
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::interval,
  acc, octave_spec, steps, fingers
)
BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::chord,
  base, intervals, all_tied
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::moving_note,
  base, intervals
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::value_distinction,
  value
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::tie,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::parser::ast::hand_sign,
  value
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::barline,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::parser::ast::simile,
  octave_spec, count
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::measure,
  ending, voices
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::key_and_time_signature,
  key, time
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::measure_specification,
  number, alternative
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::measure_range,
  first, last
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::section,
  key_and_time_sig, number, range, paragraphs
)

BOOST_FUSION_ADAPT_STRUCT(bmc::braille::parser::ast::score,
  parts
)

#endif
