// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_ALTERATION_CALCULATOR_HPP
#define BMC_ALTERATION_CALCULATOR_HPP

#include <map>
#include "bmc/braille/ast/visitor.hpp"
#include "compiler_pass.hpp"

namespace bmc { namespace braille {

/**
 * \brief Calculate the alteration of note and interval pitches.
 *
 * Given the current key signature this compiler pass calculates the
 * alteration (an integer from -3 to 3) value for each note in a measure
 * taking the accidentals into account.
 *
 * Accidental memory is reset at each measure beginning.
 *
 * \ingroup compilation
 */
class alteration_calculator
: public ast::visitor<alteration_calculator>
, public compiler_pass
{
  ::bmc::accidental memory[10][steps_per_octave];
  ::bmc::key_signature key_sig;
  ::std::multimap<rational const, ast::sign&> signs;
  rational voice_position, position;

public:
  alteration_calculator(report_error_type const& report_error)
  : compiler_pass(report_error)
  , key_sig()
  {}

  void operator()(ast::measure &measure) {
    bool const ok = traverse_measure(measure);
    BOOST_ASSERT(ok);
  }

  /**
   * \brief Set the current key signature.
   */
  void set(::bmc::key_signature sig) { key_sig = sig; }

  bool visit_measure(ast::measure &measure) {
    reset_memory();
    signs.clear();

    return true;
  }

  bool end_of_measure(ast::measure &measure) {
    for (auto &pair: signs) apply_visitor(sign_visitor, pair.second);

    return true;
  }

  bool visit_voice(ast::voice &) {
    voice_position = 0;

    return true;
  }

  bool end_of_partial_measure(ast::partial_measure &pm) {
    voice_position += duration(pm);

    return true;
  }

  bool visit_partial_voice(ast::partial_voice &) {
    position = voice_position;

    return true;
  }

  bool traverse_sign(ast::sign &sign) {
    signs.insert(std::pair<rational const, ast::sign &>{position, sign});
    position += duration(sign);

    return true;
  }

  bool visit_note(ast::note &note) {
    note.alter = to_alter(note.acc, note.octave, note.step);

    return true;
  }

  bool visit_interval(ast::interval &interval) {
    interval.alter = to_alter(interval.acc, interval.octave, interval.step);

    return true;
  }

private:
  void reset_memory()
  {
    for (std::size_t octave = 0; octave < 10; ++octave) {
      for (std::size_t step = C; step < steps_per_octave; ++step)
        memory[octave][step] = natural;
      int count = steps_per_octave;
      switch (key_sig) {
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
  int to_alter( boost::optional<::bmc::accidental> const& accidental
              , unsigned octave
              , ::bmc::diatonic_step step
              )
  {
    BOOST_ASSERT(octave < 10);
    if (accidental) memory[octave][step] = *accidental;

    switch (memory[octave][step]) {
    default: std::cerr << "mem: " << (accidental? *accidental: 0) << ' ' << memory[octave][step] << std::endl;BOOST_ASSERT(false);
    case natural:      return 0;
    case flat:         return -1;
    case double_flat:  return -2;
    case triple_flat:  return -3;
    case sharp:        return 1;
    case double_sharp: return 2;
    case triple_sharp: return 3;
    }
  }
};

}}

#endif
