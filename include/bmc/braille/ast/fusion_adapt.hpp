#ifndef BMC_AST_FUSION_ADAPT_HPP
#define BMC_AST_FUSION_ADAPT_HPP

#include "bmc/braille/ast/ast.hpp"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::slur,
  (music::braille::ast::slur::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::tie,
  (music::braille::ast::tie::type, value)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::rest,
  (music::braille::ast::value, ambiguous_value)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::stem,
  (music::rational, type)
  (unsigned, dots)
  (boost::optional<music::braille::ast::tie>, tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::note,
  (std::vector<music::articulation>, articulations)
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::diatonic_step, step)
  (music::braille::ast::value, ambiguous_value)
  (unsigned, dots)
  (std::vector<music::braille::ast::slur>, slurs)
  (music::braille::fingering_list, fingers)
  (boost::optional<music::braille::ast::tie>, tie)
  (std::vector<music::braille::ast::stem>, extra_stems)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::interval,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::interval, steps)
  (music::braille::fingering_list, fingers)
  (boost::optional<music::braille::ast::tie>, tie)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::chord,
  (music::braille::ast::note, base)
  (std::vector<music::braille::ast::interval>, intervals)
  (bool, all_tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::moving_note,
  (music::braille::ast::note, base)
  (std::vector<music::braille::ast::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::value_distinction,
  (music::braille::ast::value_distinction::type, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::simile,
  (boost::optional<unsigned>, octave_spec)
  (unsigned, count)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<music::braille::ast::voice>, voices)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::key_and_time_signature,
  (music::key_signature, key)
  (music::time_signature, time)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::measure_specification,
  (music::braille::ast::measure_specification::number_type, number)
  (boost::optional<music::braille::ast::measure_specification::number_type>, alternative)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::measure_range,
  (music::braille::ast::measure_specification, first)
  (music::braille::ast::measure_specification, last)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::section,
  (boost::optional<music::braille::ast::key_and_time_signature>, key_and_time_sig)
  (boost::optional<music::braille::ast::section::number_type>, number)
  (boost::optional<music::braille::ast::measure_range>, range)
  (std::vector<music::braille::ast::paragraph>, paragraphs)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ast::score,
  (music::key_signature, key_sig)
  (std::vector<music::time_signature>, time_sigs)
  (std::vector<music::braille::ast::part>, parts)
)

#endif
