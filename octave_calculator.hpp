// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_OCTAVE_CALCULATOR_HPP
#define BMC_OCTAVE_CALCULATOR_HPP

#include <boost/variant/static_visitor.hpp>
#include "boost_range/algorithm/all_of.hpp"
#include "ambiguous.hpp"
#include "compiler_pass.hpp"

namespace music { namespace braille {

/**
 * \brief Calcualte octaves for every note in a section of music.
 *
 * Braille music does only specify octave signs if they are necessary,
 * pretty similar to relative music in LilyPond input files.
 * This compiler pass processes braille music sequentially, filling in the
 * actual octave of every note, no matter if it is proceeded by an octave sign
 * or not.
 *
 * \ingroup compilation
 */
class octave_calculator
: public boost::static_visitor<bool>
, public compiler_pass
{
  ambiguous::note const* prev;
  music::braille::interval_direction interval_direction;

public:
  octave_calculator(report_error_type const& report_error)
  : compiler_pass(report_error)
  , prev(nullptr)
  {}

  void set(music::braille::interval_direction interval_dir)
  { interval_direction = interval_dir; }

  void reset()
  { prev = nullptr; interval_direction = braille::interval_direction::down; }

  result_type operator()(ambiguous::measure& measure)
  {
    for (ambiguous::voice& voice: measure.voices) {
      for (ambiguous::partial_measure& part: voice) {
        for (ambiguous::partial_voice& partial_voice: part) {
          if (not all_of(partial_voice, boost::apply_visitor(*this)))
            return false;
        }
      }
    }
    return true;
  }

  result_type operator()(ambiguous::note& note)
  {
    if (note.octave_spec) {
      note.octave = *note.octave_spec;
    } else {
      if (prev) {
        if ((note.step == C and (prev->step == B or prev->step == A)) or
            (note.step == D and prev->step == B)) {
          note.octave = prev->octave + 1;
        } else if ((note.step == B and (prev->step == C or prev->step == D)) or
                   (note.step == A and prev->step == C)) {
          note.octave = prev->octave - 1;
        } else {
          note.octave = prev->octave;
        }
      } else {
        report_error(note.id, L"Missing octave mark");
        return false;
      }
    }
    prev = &note;
    return true;
  }

  result_type operator()(ambiguous::chord& chord)
  {
    if ((*this)(chord.base)) {
      BOOST_ASSERT(not chord.intervals.empty());
      int step = chord.base.step;
      unsigned octave = chord.base.octave;
      for (auto& interval: chord.intervals) {
        if (interval_direction == braille::interval_direction::down) {
          step -= interval.steps;
        } else {
          step += interval.steps;
        }
        while (step > B) { octave++; step -= steps_per_octave; }
        while (step < 0) { octave--; step += steps_per_octave; }
        if (interval.octave_spec) octave = *interval.octave_spec;
        interval.octave = octave;
        interval.step = static_cast<diatonic_step>(step);
      }
      return true;
    }
    return false;
  }

  template <typename Sign>
  result_type operator()(Sign&) const
  { return true; }
};

}}

#endif
