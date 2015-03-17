#ifndef BMC_AST_FUSION_ADAPT_HPP
#define BMC_AST_FUSION_ADAPT_HPP

#include "bmc/braille/ast/ast.hpp"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::slur,
  (::bmc::braille::ast::slur::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::tie,
  (::bmc::braille::ast::tie::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::hand_sign,
  (::bmc::braille::ast::hand_sign::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(::bmc::braille::ast::rest,
  (bool, by_transcriber)
  (::bmc::braille::ast::value, ambiguous_value)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::stem,
  (::bmc::rational, type)
  (unsigned, dots)
  (boost::optional<::bmc::braille::ast::tie>, tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::note,
  (std::vector<::bmc::articulation>, articulations)
  (boost::optional<::bmc::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (::bmc::diatonic_step, step)
  (::bmc::braille::ast::value, ambiguous_value)
  (unsigned, dots)
  (std::vector<::bmc::braille::ast::slur>, slurs)
  (::bmc::braille::fingering_list, fingers)
  (boost::optional<::bmc::braille::ast::tie>, tie)
  (std::vector<::bmc::braille::ast::stem>, extra_stems)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::interval,
  (boost::optional<::bmc::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (::bmc::interval, steps)
  (::bmc::braille::fingering_list, fingers)
  (boost::optional<::bmc::braille::ast::tie>, tie)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::chord,
  (::bmc::braille::ast::note, base)
  (std::vector<::bmc::braille::ast::interval>, intervals)
  (bool, all_tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::moving_note,
  (::bmc::braille::ast::note, base)
  (std::vector<::bmc::braille::ast::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::value_distinction,
  (::bmc::braille::ast::value_distinction::type, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::simile,
  (boost::optional<unsigned>, octave_spec)
  (unsigned, count)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<::bmc::braille::ast::voice>, voices)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::key_and_time_signature,
  (::bmc::key_signature, key)
  (::bmc::time_signature, time)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::measure_specification,
  (::bmc::braille::ast::measure_specification::number_type, number)
  (boost::optional<::bmc::braille::ast::measure_specification::number_type>, alternative)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::measure_range,
  (::bmc::braille::ast::measure_specification, first)
  (::bmc::braille::ast::measure_specification, last)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::section,
  (boost::optional<::bmc::braille::ast::key_and_time_signature>, key_and_time_sig)
  (boost::optional<::bmc::braille::ast::section::number_type>, number)
  (boost::optional<::bmc::braille::ast::measure_range>, range)
  (std::vector<::bmc::braille::ast::paragraph>, paragraphs)
)

BOOST_FUSION_ADAPT_STRUCT(
  ::bmc::braille::ast::score,
  (::bmc::key_signature, key_sig)
  (std::vector<::bmc::time_signature>, time_sigs)
  (std::vector<::bmc::braille::ast::part>, parts)
)

#endif
