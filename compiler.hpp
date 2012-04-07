// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_COMPILER_HPP
#define BMC_COMPILER_HPP

#include "ambiguous.hpp"
#include "disambiguate.hpp"
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
 * \todo Expand simile signs (unrolling)
 */
class compiler : public boost::static_visitor<bool>
{
  boost::function<void(int tag, std::wstring const& what)> report_error;
  octave_calculator calculate_octaves;
  value_disambiguator disambiguate_values;
  alteration_calculator calculate_alterations;

public:
  music::time_signature global_time_signature;

  template<typename ErrorHandler>
  compiler(ErrorHandler& error_handler)
  : report_error( boost::phoenix::function<ErrorHandler>(error_handler)
                  ( L"Error"
                  , boost::phoenix::arg_names::_2
                  , boost::phoenix::cref(error_handler.iters)
                    [boost::phoenix::arg_names::_1]
                  )
                )
  , calculate_octaves(report_error)
  , disambiguate_values(report_error)
  , calculate_alterations(report_error)
  , global_time_signature(4, 4)
  {
  }

  result_type operator()(ambiguous::score& score);
  result_type operator()(ambiguous::measure& measure);
};

}}


#endif
