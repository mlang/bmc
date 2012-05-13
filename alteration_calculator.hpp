// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_ALTERATION_CALCULATOR_HPP
#define BMC_ALTERATION_CALCULATOR_HPP

#include <map>
#include <boost/variant/static_visitor.hpp>
#include "ambiguous.hpp"
#include "compiler_pass.hpp"

namespace music { namespace braille {

/**
 * \brief Calculate the alteration of note pitches.
 *
 * Given the current key signature this compiler pass calculates the
 * alteration (an integer from -2 to 2) value for each note in a measure
 * taking the accidentals into account.
 *
 * Accidental memory is reset at each measure beginning.
 *
 * \ingroup compilation
 */
class alteration_calculator
: public boost::static_visitor<void>
, public compiler_pass
{
  music::accidental memory[10][steps_per_octave];
  music::key_signature key_sig;

  struct signmap: std::multimap<rational const, ambiguous::sign&>
  {
    signmap(ambiguous::measure& measure, rational const& measure_position = rational())
    {
      for (ambiguous::voice& voice: measure.voices) {
        rational voice_position(measure_position);
        for (ambiguous::partial_measure& partial_measure: voice) {
          for (ambiguous::partial_voice& partial_voice: partial_measure) {
            rational position(voice_position);
            for (ambiguous::sign& sign: partial_voice) {
              insert(value_type(position, sign));
              position += duration(sign);
            }
          }
          voice_position += duration(partial_measure);
        }
      }
    }
  };
  int to_alter( boost::optional<music::accidental> const& accidental
              , unsigned octave
              , music::diatonic_step step
              )
  {
    if (accidental) memory[octave][step] = *accidental;

    return to_alter(memory[octave][step]);
  }

public:
  alteration_calculator(report_error_type const& report_error)
  : compiler_pass(report_error)
  {}

  /**
   * \brief Set the current key signature.
   */
  void set(music::key_signature sig)
  { key_sig = sig; }

  result_type operator() (ambiguous::measure& measure)
  {
    reset_memory(key_sig);

    // Visit all elements of this measure in chronological order
    for (auto& value: signmap(measure)) boost::apply_visitor(*this, value.second);
  }

  result_type operator()(ambiguous::note& note)
  { note.alter = to_alter(note.acc, note.octave, note.step); }

  result_type operator()(ambiguous::chord& chord)
  {
    (*this)(chord.base);
    for (auto& interval: chord.intervals) {
      interval.alter = to_alter(interval.acc, interval.octave, interval.step);
    }
  }

  template <typename Sign>
  result_type operator()(Sign&) const { }

private:
  int to_alter(music::accidental accidental) const
  {
    switch (accidental) {
    case natural:
    default:           return 0;
    case flat:         return -1;
    case double_flat:  return -2;
    case triple_flat:  return -3;
    case sharp:        return 1;
    case double_sharp: return 2;
    case triple_sharp: return 3;
    }
  }

  void reset_memory(music::key_signature key_signature)
  {
    for (std::size_t octave = 0; octave < 10; ++octave) {
      for (std::size_t step = C; step < steps_per_octave; ++step)
        memory[octave][step] = natural;
      int count = steps_per_octave;
      switch (key_signature) {
      case  14: memory[octave][B] = double_sharp; if (not --count) break;
      case  13: memory[octave][E] = double_sharp; if (not --count) break;
      case  12: memory[octave][A] = double_sharp; if (not --count) break;
      case  11: memory[octave][D] = double_sharp; if (not --count) break;
      case  10: memory[octave][G] = double_sharp; if (not --count) break;
      case   9: memory[octave][C] = double_sharp; if (not --count) break;
      case   8: memory[octave][F] = double_sharp; if (not --count) break;

      case   7: memory[octave][B] = sharp;        if (not --count) break;
      case   6: memory[octave][E] = sharp;        if (not --count) break;
      case   5: memory[octave][A] = sharp;        if (not --count) break;
      case   4: memory[octave][D] = sharp;        if (not --count) break;
      case   3: memory[octave][G] = sharp;        if (not --count) break;
      case   2: memory[octave][C] = sharp;        if (not --count) break;
      case   1: memory[octave][F] = sharp;        if (not --count) break;

      case   0: break;

      case -14: memory[octave][F] = double_flat; if (not --count) break;
      case -13: memory[octave][C] = double_flat; if (not --count) break;
      case -12: memory[octave][G] = double_flat; if (not --count) break;
      case -11: memory[octave][D] = double_flat; if (not --count) break;
      case -10: memory[octave][A] = double_flat; if (not --count) break;
      case  -9: memory[octave][E] = double_flat; if (not --count) break;
      case  -8: memory[octave][B] = double_flat; if (not --count) break;

      case  -7: memory[octave][F] = flat;        if (not --count) break;
      case  -6: memory[octave][C] = flat;        if (not --count) break;
      case  -5: memory[octave][G] = flat;        if (not --count) break;
      case  -4: memory[octave][D] = flat;        if (not --count) break;
      case  -3: memory[octave][A] = flat;        if (not --count) break;
      case  -2: memory[octave][E] = flat;        if (not --count) break;
      case  -1: memory[octave][B] = flat;        if (not --count) break;
      }
    }
  }
};

}}

#endif
