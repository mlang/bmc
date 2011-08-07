#ifndef AMBIGUOUS_HPP
#define AMBIGUOUS_HPP
#include <vector>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "music.hpp"
#include <cmath>

namespace music { namespace braille { namespace ambiguous {

enum value
{
  whole_or_16th, half_or_32th, quarter_or_64th, eighth_or_128th, unknown
};

struct pitch_and_value {
  pitch_and_value() {}
  pitch_and_value(diatonic_step step, value val) : step(step), value_(val) {}
  diatonic_step step;
  value value_;
};

struct locatable
{
  std::size_t id;
};

struct rhythmic_base : locatable {
  value ambiguous_value;
  unsigned dots;
  rational type;
};

struct rhythmic
{
  virtual rational as_rational() const = 0;
};

enum articulation {
  appoggiatura, short_appoggiatura,
  short_trill, extended_short_trill,
  turn_between_notes, turn_above_or_below_note,
  inverted_turn_between_notes, inverted_turn_above_or_below_note,
  mordent, extended_mordent
};

typedef std::pair<unsigned, unsigned> finger_change;
typedef boost::variant<unsigned, finger_change> fingering;

struct slur {};

struct note : rhythmic_base, rhythmic
{
  std::vector<articulation> articulations;
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave;
  diatonic_step step;
  std::vector<slur> slurs;
  boost::optional<fingering> finger;
  bool tied;
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct rest : rhythmic_base, rhythmic
{
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct interval {
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave;
  music::interval steps;
  boost::optional<fingering> finger;
};

struct chord : locatable, rhythmic {
  note base;
  std::vector<interval> intervals;
  virtual rational as_rational() const
  { return base.as_rational(); }
};

enum value_distinction { distinct, large_follows, small_follows };

enum hand_sign { right_hand, left_hand };

struct simile {
  boost::optional<unsigned> octave;
};

enum barline { begin_repeat, end_repeat };

typedef boost::variant< note, rest, chord
                      , value_distinction, hand_sign, simile, barline> sign;
typedef std::vector<sign> partial_voice;
typedef std::vector<partial_voice> partial_measure;
typedef std::vector<partial_measure> voice;

struct measure : std::vector<voice>, locatable
{
};

typedef std::vector< boost::variant<measure> > staff;

typedef std::vector<staff> part;

struct score {
  boost::optional<time_signature> time_sig;
  std::vector<part> parts;
};

}}}

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::pitch_and_value,
  (music::diatonic_step, step)
  (music::braille::ambiguous::value, value_)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::rest,
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::note,
  (std::vector<music::braille::ambiguous::articulation>, articulations)
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave)
  (music::diatonic_step, step)
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
  (std::vector<music::braille::ambiguous::slur>, slurs)
  (boost::optional<music::braille::ambiguous::fingering>, finger)
  (bool, tied)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::interval,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave)
  (music::interval, steps)
  (boost::optional<music::braille::ambiguous::fingering>, finger)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::chord,
  (music::braille::ambiguous::note, base)
  (std::vector<music::braille::ambiguous::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::simile,
  (boost::optional<unsigned>, octave)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::score,
  (boost::optional<music::time_signature>, time_sig)
  (std::vector<music::braille::ambiguous::part>, parts)
)

#endif
