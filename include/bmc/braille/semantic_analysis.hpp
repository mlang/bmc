// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_BRAILLE_SEMANTIC_ANALYSIS_HPP_INCLUDED
#define BMC_BRAILLE_SEMANTIC_ANALYSIS_HPP_INCLUDED

#include "bmc/braille/ast.hpp"
#include "bmc/braille/semantic_analysis/location_calculator.hpp"
#include "bmc/braille/semantic_analysis/value_disambiguator.hpp"
#include "bmc/braille/semantic_analysis/octave_calculator.hpp"
#include "bmc/braille/semantic_analysis/alteration_calculator.hpp"
#include "boost_range/algorithm/all_of.hpp"

#include <boost/function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

class sign_converter: public boost::static_visitor<bool>
{
  ast::unfolded::partial_voice &target;
public:
  sign_converter(ast::unfolded::partial_voice &target)
  : target(target)
  {}

  result_type operator() (ast::value_distinction const &) const
  {
    return true;
  }
  result_type operator() (ast::simile const &) const
  {
    std::clog << "partial_measure_simile" << std::endl;
    return true;
  }

  template<typename T>
  result_type operator() (T const &t) const
  {
    target.emplace_back(t);
    return true;
  }
};

class staff_converter: public boost::static_visitor<bool>
{
  ast::unfolded::staff &target;
public:
  staff_converter( ast::unfolded::staff &target )
  : target(target)
  {}

  result_type operator() (ast::measure const &measure) const
  {
    ast::unfolded::measure unfolded_measure;
    unfolded_measure.ending = measure.ending;
    for (ast::voice const &voice: measure.voices) {
      unfolded_measure.voices.emplace_back();
      ast::unfolded::voice &new_voice = unfolded_measure.voices.back();
      for (ast::partial_measure const &partial_measure: voice) {
        new_voice.emplace_back();
        ast::unfolded::partial_measure &new_partial_measure = new_voice.back();
        for (ast::partial_voice const &partial_voice: partial_measure) {
          new_partial_measure.emplace_back();
          ast::unfolded::partial_voice &new_partial_voice = new_partial_measure.back();
          sign_converter convert(new_partial_voice);
          if (not std::all_of( partial_voice.begin(), partial_voice.end()
                             , apply_visitor(convert)
                             )
             )
            return false;
        }
      }
    }
    target.emplace_back(unfolded_measure);
    return true;
  }
  result_type operator() (ast::key_and_time_signature const &key_and_time_signature) const
  {
    target.emplace_back(key_and_time_signature);
    return true;
  }
};

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
  music::time_signature global_time_signature;

public:
  compiler( ErrorHandler& error_handler
          , music::time_signature const& time_signature = music::time_signature(4, 4)
          )
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
  , global_time_signature(time_signature)
  {
  }

  result_type operator()(ast::score& score)
  {
    if (score.time_sig) {
      global_time_signature = *score.time_sig;
    }

    for (ast::part& part: score.parts) {
      for (std::size_t staff_index = 0;
           staff_index < part.size();
           ++staff_index) {
        music::braille::interval_direction
        interval_direction = music::braille::interval_direction::down;
        switch (staff_index) {
        case 0:
          interval_direction = music::braille::interval_direction::down;
          break;
        case 1:
          interval_direction = music::braille::interval_direction::up;
          break;
        default: BOOST_ASSERT(false);
        }
        calculate_octaves.set(interval_direction);
        calculate_alterations.set(score.key_sig);
        if (not (*this)(part[staff_index])) return false;
        calculate_octaves.reset();
      }
    }
    return unfold(score);
  }

  result_type unfold (ast::score &score)
  {
    for (ast::part const &part: score.parts) {
      score.unfolded_part.emplace_back();
      if (not unfold(part, score.unfolded_part.back(), score)) return false;
    }
    return true;
  }
  result_type unfold ( ast::part const &source
                     , ast::unfolded::part &target
                     , ast::score const &score
                     )
  {
    for (ast::staff const &staff: source) {
      target.emplace_back();
      if (not unfold(staff, target.back(), score)) return false;
    }
    return true;
  }
  result_type unfold ( ast::staff const &source
                     , ast::unfolded::staff &target
                     , ast::score const &score
                     )
  {
    staff_converter convert(target);
    return std::all_of(source.begin(), source.end(), boost::apply_visitor(convert));
  }

  result_type operator() (ast::staff& staff)
  { return all_of(staff, boost::apply_visitor(*this)) && disambiguate_values.end_of_staff(); }

  result_type operator()(ast::measure& measure)
  {
    disambiguate_values.set(global_time_signature);
    if (disambiguate_values(measure))
      if (calculate_octaves(measure)) {
        calculate_alterations(measure);
        calculate_locations(measure);
        return true;
      }
    return false;
  }
  result_type operator()(ast::key_and_time_signature& key_and_time_sig)
  {
    calculate_locations(key_and_time_sig);
    disambiguate_values.set(key_and_time_sig.time);
    return true;
  }
};

}}

#endif
