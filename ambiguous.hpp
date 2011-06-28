#ifndef AMBIGUOUS_HPP
#define AMBIGUOUS_HPP
#include "music.hpp"

namespace music { namespace braille { namespace ambiguous {

struct value {
  value() {}
  value(rational v1, rational v2) {
    values.push_back(v1);
    values.push_back(v2);
  }
  std::vector<rational> values;
};

struct pitch_and_value : value {
  pitch_and_value() : value(), step(C) {}
  pitch_and_value(diatonic_step step, rational v1, rational v2)
  : value(v1, v2), step(step) {}
  diatonic_step step;
};

struct note {
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave;
  pitch_and_value pitch_value;
  unsigned dots;
  boost::optional<unsigned> finger;
};

struct rest {
  value val;
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
typedef std::vector<sign> pmia_voice;
typedef std::vector<pmia_voice> pmia;
typedef std::vector<pmia> fmia;
typedef std::vector<fmia> measure;

}}}

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::rest,
  (music::braille::ambiguous::value, val)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::note,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave)
  (music::braille::ambiguous::pitch_and_value, pitch_value)
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
