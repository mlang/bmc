#ifndef BMC_AST_FUSION_ADAPT_HPP
#define BMC_AST_FUSION_ADAPT_HPP

#include "bmc/braille/ast/ast.hpp"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::clef,
  sign, line, other_staff
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::slur,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::tie,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::hand_sign,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::rest,
  by_transcriber, ambiguous_value, dots
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::stem,
  type, dots, tied
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::note,
  articulations, acc, octave_spec, step, ambiguous_value, dots, slurs, fingers,
  tie, extra_stems
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::interval,
  acc, octave_spec, steps, fingers, tie
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::chord,
  base, intervals, all_tied
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::moving_note,
  base, intervals
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::value_prefix,
  value
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::simile,
  octave_spec, count
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::measure,
  ending, voices
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::key_and_time_signature,
  key, time
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::measure_specification,
  number, alternative
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::measure_range,
  first, last
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::section,
  key_and_time_sig, number, range, paragraphs
)

BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::score,
  key_sig, time_sigs, parts
)

#endif
