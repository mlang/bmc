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
  int last_octave, last_step;
  int measure_id;

public:
  octave_calculator(boost::function< void(int tag
                                   , std::wstring const& what)
                                   > const& report_error
                   )
  : report_error(report_error)
  { clear(); }

  void clear()
  { last_step = -1, last_octave = -1; }

  result_type operator()(ambiguous::measure& measure)
  {
    measure_id = measure.id;
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
    if (not note.octave and last_octave == -1) {
      report_error(measure_id, L"Missing octave mark");
      return false;
    }

    if (note.octave) {
      last_octave = note.real_octave = *note.octave;
    } else {
      if ((note.step == 0 and (last_step == 6 or last_step == 5)) or
          (note.step == 1 and last_step == 6)) {
        last_octave = note.real_octave = last_octave + 1;
      } else if ((note.step == 6 and (last_step == 0 or last_step == 1)) or
                 (note.step == 5 and last_step == 0)) {
        last_octave = note.real_octave = last_octave - 1;
      } else {
        note.real_octave = last_octave;
      }
    }
    last_step = note.step;
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
  int last_octave;
  int last_step;
public:
  music::time_signature global_time_signature;
  typedef bool result_type;
  template<typename ErrorHandler>
  compiler(ErrorHandler& error_handler)
  : global_time_signature(4, 4)
  , anacrusis()
  , last_octave(-1), last_step(-1)
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
