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
#include "duration.hpp"

namespace music { namespace braille {

/**
 * \brief Calculates the alteration of note pitches.
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
  report_error_type const& report_error;
  music::accidental memory[10][steps_per_octave];
  music::key_signature key_sig;
public:
  alteration_calculator(report_error_type const& report_error)
  : report_error(report_error)
  {}

  /**
   * \brief Set the current key signature.
   */
  void set(music::key_signature sig)
  { key_sig = sig; }

  result_type operator()(ambiguous::measure& measure)
  {
    reset_memory(key_sig);

    // Flatten and sort all elements along the time axis
    typedef std::multimap<rational, ambiguous::sign&> eventmap;
    eventmap map;
    for (ambiguous::voice& voice: measure.voices) {
      rational voice_position;
      for (ambiguous::partial_measure& part: voice) {
        for (ambiguous::partial_voice& partial_voice: part) {
          rational position(voice_position);
          for (ambiguous::sign& sign: partial_voice) {
            map.insert(eventmap::value_type(position, sign));
            position += music::duration(sign);
          }
        }
        voice_position += music::duration(part);
      }
    }

    // Visit all elements of this measure in chronological order
    for (auto& value: map) boost::apply_visitor(*this, value.second);
  }

  result_type operator()(ambiguous::note& note)
  {
    if (note.acc) {
      note.alter = to_alter(*note.acc);
      memory[note.octave][note.step] = *note.acc;
    } else {
      note.alter = to_alter(memory[note.octave][note.step]);
    }
  }

  result_type operator()(ambiguous::chord& chord)
  {
    (*this)(chord.base);
    // ...
  }

  template<typename Sign> result_type operator()(Sign&) const { }

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
      switch (key_signature) {
      case  7: memory[octave][B] = sharp;
      case  6: memory[octave][E] = sharp;
      case  5: memory[octave][A] = sharp;
      case  4: memory[octave][D] = sharp;
      case  3: memory[octave][G] = sharp;
      case  2: memory[octave][C] = sharp;
      case  1: memory[octave][F] = sharp;
      case  0: break;
      case -7: memory[octave][F] = flat;
      case -6: memory[octave][C] = flat;
      case -5: memory[octave][G] = flat;
      case -4: memory[octave][D] = flat;
      case -3: memory[octave][A] = flat;
      case -2: memory[octave][E] = flat;
      case -1: memory[octave][B] = flat;
      }
    }
  }
};

}}

#endif
