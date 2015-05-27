#if !defined(BMC_BRAILLE_PARSER_AST_ADAPTED_HPP)
#define BMC_BRAILLE_PARSER_AST_ADAPTED_HPP

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  bmc::braille::parser::ast::time_signature,
  (int, numerator)
  (int, denominator)
)

BOOST_FUSION_ADAPT_STRUCT(
  bmc::braille::parser::ast::key_signature,
  (int, fifths)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::note,
//(std::vector<::bmc::articulation>, articulations)
  (boost::optional<::bmc::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (::bmc::diatonic_step, step)
  (::bmc::braille::parser::ast::ambiguous_value, ambiguous_value)
  (unsigned, dots)
//(std::vector<::bmc::braille::ast::slur>, slurs)
//(::bmc::braille::fingering_list, fingers)
//(boost::optional<::bmc::braille::ast::tie>, tie)
//(std::vector<::bmc::braille::ast::stem>, extra_stems)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::interval,
  (boost::optional<::bmc::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (::bmc::interval, steps)
//(::bmc::braille::fingering_list, fingers)
//(boost::optional<::bmc::braille::ast::tie>, tie)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::chord,
  (::bmc::braille::parser::ast::note, base)
  (std::vector<::bmc::braille::parser::ast::interval>, intervals)
  (bool, all_tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::moving_note,
  (::bmc::braille::parser::ast::note, base)
  (std::vector<::bmc::braille::parser::ast::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<::bmc::braille::parser::ast::voice>, voices)
)

#endif
