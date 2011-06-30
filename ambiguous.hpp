#ifndef AMBIGUOUS_HPP
#define AMBIGUOUS_HPP
#include "music.hpp"

namespace music { namespace braille { namespace ambiguous {

enum value
{
  whole_or_16th, half_or_32th, quarter_or_64th, eighth_or_128th, unknown
};

struct pitch_and_value {
  pitch_and_value() {}
  pitch_and_value(diatonic_step step, value val) : value_(val), step(step) {}
  diatonic_step step;
  value value_;
};

struct note {
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave;
  diatonic_step step;
  value ambiguous_value;
  rational type;
  unsigned dots;
  boost::optional<unsigned> finger;
};

struct rest {
  value ambiguous_value;
  rational type;
  unsigned dots;
};

struct interval {
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave;
  music::interval steps;
  boost::optional<unsigned> finger;
};

struct chord {
  note base;
  std::vector<interval> intervals;
};

enum value_distinction { distinct, large_follows, small_follows };

struct simile {
  boost::optional<unsigned> octave;
};

typedef boost::variant<note, rest, chord, value_distinction, simile> sign;
typedef std::vector<sign> partial_measure_voice;
typedef std::vector<partial_measure_voice> partial_measure;
typedef std::vector<partial_measure> voice;
typedef std::vector<voice> measure;

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
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave)
  (music::diatonic_step, step)
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
  (boost::optional<unsigned>, finger)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::interval,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave)
  (music::interval, steps)
  (boost::optional<unsigned>, finger)
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

#endif
