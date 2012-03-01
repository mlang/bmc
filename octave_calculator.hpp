// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_OCTAVE_CALCULATOR_HPP
#define BMC_OCTAVE_CALCULATOR_HPP

#include <boost/foreach.hpp>
#include <boost/variant/static_visitor.hpp>
#include "ambiguous.hpp"
#include "compiler_pass.hpp"

namespace music { namespace braille {

class octave_calculator
: public boost::static_visitor<bool>
, public compiler_pass
{
  report_error_type const& report_error;
  ambiguous::note const* prev;

public:
  octave_calculator(report_error_type const& report_error)
  : report_error(report_error)
  , prev(0)
  {}

  void clear()
  { prev = 0; }

  result_type operator()(ambiguous::measure& measure)
  {
    BOOST_FOREACH(ambiguous::voice& voice, measure.voices) {
      BOOST_FOREACH(ambiguous::partial_measure& part, voice) {
        BOOST_FOREACH(ambiguous::partial_voice& partial_voice, part) {
          ambiguous::partial_voice::iterator iter = partial_voice.begin();
          bool ok = true;
          while (ok && iter != partial_voice.end())
            ok = boost::apply_visitor(*this, *iter++);
          if (not ok) return false;
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
      // ...
      return true;
    }
    return false;
  }

  template<typename Sign>
  result_type operator()(Sign&) const
  { return true; }
};

}}

#endif
