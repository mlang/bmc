// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AMBIGUOUS_HPP
#define BMC_AMBIGUOUS_HPP
#include <list>
#include <vector>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "music.hpp"
#include "braille_music.hpp"
#include <cmath>

namespace music { namespace braille {

/**
 * \brief Data types required to store the parse results.
 *
 * Some of the missing values are filled in by the compiler translation unit
 * right now.  However, it is likely that we will need two different ways of
 * storing our data, one form immediately after parsing, and another as a result
 * of the necessary transformations on top of that.
 *
 * The namespace is called 'ambiguous' because it resembles the pure,
 * unprocessed parse results which are by the nature of braille music code
 * ambiguous (exact note and rest values need to be calculated).
 */
namespace ambiguous {

enum value
{
  whole_or_16th, half_or_32th, quarter_or_64th, eighth_or_128th, unknown
};

struct locatable
{
  std::size_t id;
};

struct rhythmic_data : locatable
{
  ambiguous::value ambiguous_value;
  unsigned dots;
  rational type; // filled in by disambiguate.hpp
};

struct rhythmic
{
  virtual ~rhythmic() {}
  virtual rational as_rational() const = 0;
};

struct slur {};

struct note : rhythmic_data, rhythmic
{
  std::vector<articulation> articulations;
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave_spec;
  unsigned octave; // filled in by octave_calculator.hpp
  diatonic_step step;
  int alter;       // filled in by alteration_calculator.hpp
  std::vector<slur> slurs;
  fingering_list fingers;
  bool tied;

  note(): octave(0), alter(0), tied(false) {}
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct rest : rhythmic_data, rhythmic
{
  rest() : whole_measure(false) {}
  bool whole_measure; // filled in by disambiguate.hpp
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct interval : locatable {
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave_spec;
  music::interval steps;
  fingering_list fingers;
};

struct chord : locatable, rhythmic {
  note base;
  std::vector<interval> intervals;
  virtual rational as_rational() const
  { return base.as_rational(); }
};

enum value_distinction { distinct, large_follows, small_follows };

struct simile {
  boost::optional<unsigned> octave_spec;
};

enum barline { begin_repeat, end_repeat };

typedef boost::variant< note, rest, chord
                      , value_distinction, hand_sign, simile, barline> sign;
typedef std::vector<sign> partial_voice;
typedef std::vector<partial_voice> partial_measure;
typedef std::vector<partial_measure> voice;

struct measure : locatable
{
  boost::optional<unsigned> ending;
  std::vector<voice> voices;
};

typedef std::vector< boost::variant<measure> > staff;

typedef std::vector<staff> part;

struct score {
  key_signature key_sig;
  boost::optional<time_signature> time_sig;
  std::vector<part> parts;
};

}}}

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::rest,
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::note,
  (std::vector<music::articulation>, articulations)
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::diatonic_step, step)
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
  (std::vector<music::braille::ambiguous::slur>, slurs)
  (music::braille::fingering_list, fingers)
  (bool, tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::interval,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::interval, steps)
  (music::braille::fingering_list, fingers)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::chord,
  (music::braille::ambiguous::note, base)
  (std::vector<music::braille::ambiguous::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::simile,
  (boost::optional<unsigned>, octave_spec)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<music::braille::ambiguous::voice>, voices)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::score,
  (music::key_signature, key_sig)
  (boost::optional<music::time_signature>, time_sig)
  (std::vector<music::braille::ambiguous::part>, parts)
)

#endif
