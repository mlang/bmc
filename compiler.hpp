// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_COMPILER_HPP
#define BMC_COMPILER_HPP

#include "ambiguous.hpp"
#include "disambiguate.hpp"
#include <boost/function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

class octave_calculator : public boost::static_visitor<bool>
{
  boost::function<void(int tag, std::wstring const& what)> const& report_error;
  ambiguous::note const* prev;

public:
  octave_calculator(boost::function< void(int tag
                                   , std::wstring const& what)
                                   > const& report_error
                   )
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
    if (note.octave) {
      note.real_octave = *note.octave;
    } else {
      if (prev) {
        if ((note.step == C and (prev->step == B or prev->step == A)) or
            (note.step == D and prev->step == B)) {
          note.real_octave = prev->real_octave + 1;
        } else if ((note.step == B and (prev->step == C or prev->step == D)) or
                   (note.step == A and prev->step == C)) {
          note.real_octave = prev->real_octave - 1;
        } else {
          note.real_octave = prev->real_octave;
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
  { return (*this)(chord.base); }

  template<typename Sign>
  result_type operator()(Sign&) const
  { return true; }
};

class compiler
{
  boost::function<void(int tag, std::wstring const& what)> report_error;
  measure_interpretations anacrusis;
  octave_calculator calculate_octaves;
public:
  music::time_signature global_time_signature;
  typedef bool result_type;
  template<typename ErrorHandler>
  compiler(ErrorHandler& error_handler)
  : global_time_signature(4, 4)
  , anacrusis()
  , calculate_octaves(report_error)
  {
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;
    report_error =
      boost::phoenix::function<ErrorHandler>(error_handler)
      (L"Error", _2, boost::phoenix::cref(error_handler.iters)[_1]);
  }

  result_type operator()(ambiguous::score& score);
  result_type operator()(ambiguous::measure& measure);

private:
  result_type disambiguate(ambiguous::measure& measure);
};

}}


#endif
