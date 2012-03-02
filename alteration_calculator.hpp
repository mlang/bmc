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

class alteration_calculator
: public boost::static_visitor<void>
, public compiler_pass
{
  report_error_type const& report_error;
  music::accidental memory[10][B + 1];
  music::key_signature key_sig;
public:
  alteration_calculator(report_error_type const& report_error)
  : report_error(report_error)
  {}

  void set(music::key_signature sig)
  { key_sig = sig; }

  result_type operator()(ambiguous::measure& measure)
  {
    std::multimap<rational, ambiguous::sign&> map;
    for (ambiguous::voice& voice: measure.voices) {
      rational voice_position;
      for (ambiguous::partial_measure& part: voice) {
        for (ambiguous::partial_voice& partial_voice: part) {
          rational position(voice_position);
          for (ambiguous::sign& sign: partial_voice) {
            map.insert(std::pair<rational, ambiguous::sign&>(position, sign));
            position += music::duration(sign);
          }
        }
        voice_position += music::duration(part);
      }
    }
    for (int octave = 0; octave < 10; ++octave) {
      for (int step = C; step <= B; ++step) memory[octave][step] = natural;
      switch (key_sig) {
      case 7:  memory[octave][B] = sharp;
      case 6:  memory[octave][E] = sharp;
      case 5:  memory[octave][A] = sharp;
      case 4:  memory[octave][D] = sharp;
      case 3:  memory[octave][G] = sharp;
      case 2:  memory[octave][C] = sharp;
      case 1:  memory[octave][F] = sharp;
      case 0:  break;
      case -7: memory[octave][F] = flat;
      case -6: memory[octave][C] = flat;
      case -5: memory[octave][G] = flat;
      case -4: memory[octave][D] = flat;
      case -3: memory[octave][A] = flat;
      case -2: memory[octave][E] = flat;
      case -1: memory[octave][B] = flat;
      }
    }
    for (auto& value: map) boost::apply_visitor(*this, value.second);
  }

  result_type operator()(ambiguous::note& note)
  {
    if (note.acc) {
      switch (*note.acc) {
      case flat: note.alter = -1; break;
      case double_flat: note.alter = -2; break;
      case sharp: note.alter = 1; break;
      case double_sharp: note.alter = 2; break;
      }
      memory[note.octave][note.step] = *note.acc;
    } else {
      switch (memory[note.octave][note.step]) {
      case flat: note.alter = -1; break;
      case double_flat: note.alter = -2; break;
      case sharp: note.alter = 1; break;
      case double_sharp: note.alter = 2; break;
      }
    }
  }

  result_type operator()(ambiguous::chord& chord)
  {
    (*this)(chord.base);
    // ...
  }

  template<typename Sign>
  result_type operator()(Sign&) const
  { }
};

}}

#endif
