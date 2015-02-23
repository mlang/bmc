// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_OCTAVE_CALCULATOR_HPP
#define BMC_OCTAVE_CALCULATOR_HPP

#include <boost/variant/static_visitor.hpp>
#include "bmc/braille/ast.hpp"
#include "bmc/braille/ast/visitor.hpp"
#include "compiler_pass.hpp"

namespace bmc { namespace braille {

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
: public ast::visitor<octave_calculator>
, public compiler_pass
{
  ast::note const* prev;
  ::bmc::braille::interval_direction interval_direction;
  ast::voice const *current_voice;
  ast::partial_measure const *current_partial_measure;

public:
  octave_calculator(report_error_type const& report_error)
  : compiler_pass(report_error)
  , prev(nullptr)
  {}

  void set(::bmc::braille::interval_direction interval_dir)
  { interval_direction = interval_dir; }

  void reset()
  { prev = nullptr; interval_direction = braille::interval_direction::down; }

  bool operator()(ast::measure &measure) { return traverse_measure(measure); }
  bool visit_voice(ast::voice &v) {
    current_voice = &v;
    return true;
  }
  bool visit_partial_measure(ast::partial_measure &pm) {
    current_partial_measure = &pm;
    return true;
  }
  bool end_of_partial_measure(ast::partial_measure &) {
    if (current_voice->size() > 1) prev = nullptr;
    return true;
  }
  bool end_of_partial_voice(ast::partial_voice &) {
    if (current_partial_measure->size() > 1) prev = nullptr;
    return true;
  }
  bool visit_note(ast::note &note) {
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

  bool traverse_chord(ast::chord& chord) {
    if (traverse_note(chord.base)) {
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

  bool traverse_moving_note(ast::moving_note &chord) {
    if (traverse_note(chord.base)) {
      BOOST_ASSERT(not chord.intervals.empty());
      for (auto& interval: chord.intervals) {
        int step = chord.base.step;
        unsigned octave = chord.base.octave;
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

  // The note following a clef sign must always have its proper octave mark.
  bool visit_clef(ast::clef &) {
    prev = nullptr;
    return true;
  }
};

}}

#endif
