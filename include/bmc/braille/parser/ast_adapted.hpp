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
  ::bmc::braille::parser::ast::clef,
  (::bmc::braille::parser::ast::clef::type, sign)
  (::boost::optional<unsigned>, line)
  (bool, other_staff)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::tuplet_start,
  (unsigned, number)
  (bool, simple_triplet)
  (bool, doubled)
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
  (::bmc::braille::parser::ast::fingering, fingers)
//(boost::optional<::bmc::braille::ast::tie>, tie)
//(std::vector<::bmc::braille::ast::stem>, extra_stems)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::rest,
  (bool, by_transcriber)
  (::bmc::braille::parser::ast::ambiguous_value, ambiguous_value)
  (unsigned, dots)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::interval,
  (boost::optional<::bmc::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (::bmc::interval, steps)
  (::bmc::braille::parser::ast::fingering, fingers)
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
  ::bmc::braille::parser::ast::value_distinction,
  (::bmc::braille::parser::ast::value_distinction::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::tie,
  (::bmc::braille::parser::ast::tie::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::hand_sign,
  (::bmc::braille::parser::ast::hand_sign::type, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::simile,
  (boost::optional<unsigned>, octave_spec)
  (unsigned, count)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<::bmc::braille::parser::ast::voice>, voices)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::key_and_time_signature,
  (::bmc::braille::parser::ast::key_signature, key)
  (::bmc::braille::parser::ast::time_signature, time)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::measure_specification,
  (::bmc::braille::parser::ast::measure_specification::number_type, number)
  (boost::optional<::bmc::braille::parser::ast::measure_specification::number_type>, alternative)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::measure_range,
  (::bmc::braille::parser::ast::measure_specification, first)
  (::bmc::braille::parser::ast::measure_specification, last)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::section,
  (boost::optional<::bmc::braille::parser::ast::key_and_time_signature>, key_and_time_sig)
  (boost::optional<::bmc::braille::parser::ast::section::number_type>, number)
  (boost::optional<::bmc::braille::parser::ast::measure_range>, range)
  (std::vector<::bmc::braille::parser::ast::paragraph>, paragraphs)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::parser::ast::score,
//(::bmc::braille::parser::ast::key_signature, key_sig)
//(std::vector<::bmc::braille::parser::ast::time_signature>, time_sigs)
  (std::vector<::bmc::braille::parser::ast::part>, parts)
)

#endif
