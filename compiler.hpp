// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_COMPILER_HPP
#define BMC_COMPILER_HPP

#include "ambiguous.hpp"

#include "location_calculator.hpp"
#include "value_disambiguator.hpp"
#include "octave_calculator.hpp"
#include "alteration_calculator.hpp"

#include <boost/function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

/**
 * \brief The <code>compiler</code> processes the raw parsed syntax tree to fill
 *        in musical information implied by the given input.
 *
 * As a first step, the actual values of notes and rests are found with a
 * (quite computationally expensive) recursive algorithm.
 *
 * In another step, the octave values of those notes which do not have a octave
 * sign in front of them is calculated, and all notes are assigned a definitive
 * octave.
 *
 * Depending on value disambiguation having succeded, alteration of note pitches
 * is calculated in yet another step.  This step needs to order the notes
 * in a measure along the time axis to correctly interpret accidental markings,
 * therefore it depends on value disambiguation already having taken place.
 *
 * \ingroup compilation
 * \todo Expand simile signs (unrolling)
 */
template <typename ErrorHandler>
class compiler : public compiler_pass, public boost::static_visitor<bool>
{
  location_calculator<ErrorHandler> calculate_locations;
  octave_calculator calculate_octaves;
  value_disambiguator disambiguate_values;
  alteration_calculator calculate_alterations;

public:
  music::time_signature global_time_signature;

  compiler(ErrorHandler& error_handler)
  : compiler_pass( boost::phoenix::function<ErrorHandler>(error_handler)
                   ( L"Error"
                   , boost::phoenix::arg_names::_2
                   , boost::phoenix::cref(error_handler.iters)
                     [boost::phoenix::arg_names::_1]
                   )
                 )
  , calculate_locations(report_error, error_handler)
  , calculate_octaves(report_error)
  , disambiguate_values(report_error)
  , calculate_alterations(report_error)
  , global_time_signature(4, 4)
  {
  }

  result_type operator()(ambiguous::score& score)
  {
    if (score.time_sig) {
      global_time_signature = *score.time_sig;
    }

    for (ambiguous::part& part: score.parts) {
      for (ambiguous::staff& staff: part) {
        calculate_alterations.set(score.key_sig);
        if (not (*this)(staff)) return false;
        calculate_octaves.reset();
      }
    }
    return true;
  }
  result_type operator()(ambiguous::staff& staff)
  {
    return std::all_of(staff.begin(), staff.end(), boost::apply_visitor(*this));
  }
  result_type operator()(ambiguous::measure& measure)
  {
    if (disambiguate_values(measure, global_time_signature))
      if (calculate_octaves(measure)) {
        calculate_alterations(measure);
        calculate_locations(measure);
        return true;
      }
    return false;
  }
};

}}

#endif
