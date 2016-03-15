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
#include "bmc/braille/semantic_analysis/doubling_decoder.hpp"

#include <future>

namespace bmc { namespace braille {

enum class sign_conversion_result
{
  ok, full_measure_simile, failed
};

class sign_converter: public boost::static_visitor<sign_conversion_result>
{
  ast::unfolded::partial_voice &target;
  std::size_t const voice_index, voice_count
                  , partial_measure_index
                  , partial_voice_index
                  ;
  ast::unfolded::measure *prev_unfolded_measure;
  std::size_t simile_start = 0;
public:
  sign_converter( ast::unfolded::partial_voice &target
                , std::size_t voice_index, std::size_t voice_count
                , std::size_t partial_measure_index
                , std::size_t partial_voice_index
                , ast::unfolded::measure *prev_unfolded_measure
                )
  : target{target}
  , voice_index(voice_index)
  , voice_count(voice_count)
  , partial_measure_index(partial_measure_index)
  , partial_voice_index(partial_voice_index)
  , prev_unfolded_measure(prev_unfolded_measure)
  {}

  // Value distinction signs, music hyphens and tuplet indicators are irrelevant from here on.
  result_type operator() (ast::value_prefix const &) const
  { return sign_conversion_result::ok; }
  result_type operator() (ast::hyphen const &) const
  { return sign_conversion_result::ok; }
  result_type operator() (ast::tuplet_start const &) const
  { return sign_conversion_result::ok; }

  result_type operator() (ast::simile const &simile)
  {
    if (!duration(target)) {
      if (prev_unfolded_measure) {
        if (voice_count == 1) {
          prev_unfolded_measure->count += simile.count - 1;
          return sign_conversion_result::full_measure_simile;
        }
        target.insert( target.end()
                     , prev_unfolded_measure->voices[voice_index][partial_measure_index][partial_voice_index].begin()
                     , prev_unfolded_measure->voices[voice_index][partial_measure_index][partial_voice_index].end());
      }
    } else {
      ast::unfolded::partial_voice repeated {
        std::next(target.begin(), simile_start), target.end()
      };
      if (duration(repeated) == (simile.duration / rational::int_type(simile.count))) {
        for (unsigned i = 0; i < simile.count; ++i)
          target.insert(target.end(), repeated.begin(), repeated.end());
        simile_start = target.size();
      }
    }
    return sign_conversion_result::ok;
  }

  template<typename T>
  result_type operator() (T const &t) const
  {
    target.emplace_back(t);
    return sign_conversion_result::ok;
  }
};

class staff_converter: public boost::static_visitor<bool>
{
  ast::unfolded::staff &target;
  ast::unfolded::measure *prev_unfolded_measure;
public:
  staff_converter( ast::unfolded::staff &target )
  : target(target)
  , prev_unfolded_measure(nullptr)
  {}

  result_type operator() (ast::measure const &measure)
  {
    ast::unfolded::measure unfolded_measure;
    unfolded_measure.ending = measure.ending;
    bool insert = true;
    for (ast::voice const &voice: measure.voices) {
      std::size_t voice_index = unfolded_measure.voices.size();
      unfolded_measure.voices.emplace_back();
      ast::unfolded::voice &new_voice = unfolded_measure.voices.back();
      for (ast::partial_measure const &partial_measure: voice) {
        std::size_t const partial_measure_index = new_voice.size();
        new_voice.emplace_back();
        ast::unfolded::partial_measure &new_partial_measure = new_voice.back();
        for (ast::partial_voice const &partial_voice: partial_measure) {
          std::size_t const partial_voice_index = new_partial_measure.size();
          new_partial_measure.emplace_back();
          sign_converter unfold( new_partial_measure.back()
                               , voice_index, measure.voices.size()
                               , partial_measure_index
                               , partial_voice_index
                               , prev_unfolded_measure
                               );
          for (ast::partial_voice::const_iterator sign = partial_voice.begin();
               sign != partial_voice.end(); ++sign) {
            switch (apply_visitor(unfold, *sign)) {
            case sign_conversion_result::failed: return false;
            case sign_conversion_result::full_measure_simile:
              if (std::next(sign) == partial_voice.end()) {
                prev_unfolded_measure->count += 1;
                insert = false;
              }
              break;
            case sign_conversion_result::ok: break;
            }
          }
        }
      }
    }
    if (insert) {
      target.emplace_back(unfolded_measure);
      prev_unfolded_measure = boost::get<ast::unfolded::measure>(&target.back());
    }
    return true;
  }
  result_type operator() (ast::key_and_time_signature const &key_and_time_signature) const
  {
    target.emplace_back(key_and_time_signature);
    return true;
  }
};

template <typename ErrorHandler>
class annotate_staff : public compiler_pass, public boost::static_visitor<bool>
{
  location_calculator<ErrorHandler> calculate_locations;
  octave_calculator calculate_octaves;
  value_disambiguator disambiguate_values;
  alteration_calculator calculate_alterations;
  ::bmc::time_signature global_time_signature;
  ::bmc::key_signature global_key_signature;

public:
  annotate_staff( ErrorHandler &error_handler
                , report_error_type const &report_error
                , ::bmc::time_signature const& time_signature = ::bmc::time_signature(4, 4)
                , ::bmc::key_signature const& key_signature = ::bmc::key_signature(0)
                )
  : compiler_pass( report_error )
  , calculate_locations(report_error, error_handler)
  , calculate_octaves(report_error)
  , disambiguate_values(report_error)
  , calculate_alterations(report_error)
  , global_time_signature(time_signature)
  , global_key_signature{key_signature}
  {
  }

  result_type operator() (std::size_t staff_index, ast::part &part)
  {
    interval_direction interval_dir = interval_direction::down;
    switch (staff_index) {
    case 0:
      interval_dir = interval_direction::down;
      break;
    case 1:
      interval_dir = interval_direction::up;
      break;
    default:
      //report_error(part.id, L"No support for parts with more then two staves yet");
      BOOST_ASSERT(false);
    }
    calculate_octaves.set(interval_dir);
    disambiguate_values.set(global_time_signature);
    calculate_alterations.set(global_key_signature);

    for (ast::section &section: part) {
      ast::paragraph &paragraph = section.paragraphs[staff_index];
      if (!std::all_of( std::begin(paragraph), std::end(paragraph)
	              , apply_visitor(*this)))
	return false;
    }

    return disambiguate_values.end_of_staff();
  }

  result_type operator()(ast::measure& measure)
  {
    if (calculate_octaves(measure)) {
      if (disambiguate_values(measure))
        calculate_alterations(measure);
        calculate_locations(measure);
        return true;
      }
    return false;
  }

  result_type operator()(ast::key_and_time_signature &key_and_time_sig)
  {
    calculate_locations(key_and_time_sig);
    disambiguate_values.set(key_and_time_sig.time);
    calculate_alterations.set(key_and_time_sig.key);
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
  ErrorHandler error_handler;
  location_calculator<ErrorHandler> calculate_locations;
  octave_calculator calculate_octaves;
  value_disambiguator disambiguate_values;
  alteration_calculator calculate_alterations;
  ::bmc::time_signature global_time_signature;

public:
  compiler( ErrorHandler& error_handler
          , ::bmc::time_signature const& time_signature = ::bmc::time_signature(4, 4)
          )
  : compiler_pass( [&error_handler](int tag, std::wstring const &what)
                   { error_handler(L"Error", what, error_handler.ranges[tag].begin()); }
                 )
  , error_handler(error_handler)
  , calculate_locations(report_error, error_handler)
  , calculate_octaves(report_error)
  , disambiguate_values(report_error)
  , calculate_alterations(report_error)
  , global_time_signature(time_signature)
  {
  }

  result_type operator()(ast::score& score)
  {
    if (!score.time_sigs.empty()) global_time_signature = score.time_sigs.front();

    {
      std::vector<std::future<bool>> staves;
      for (ast::part &part: score.parts) {
        if (!part.empty()) {
          std::size_t const staff_count{part.front().paragraphs.size()};
          for (std::size_t staff_index = 0; staff_index < staff_count;
               ++staff_index) {
            staves.emplace_back
            ( async( std::launch::async
                   , std::move(annotate_staff<ErrorHandler>( error_handler
                                                           , report_error
                                                           , global_time_signature
                                                           , score.key_sig
                                                           ))
                   , staff_index, std::ref(part))
            );
          }
        }
      }
      if (!all_of( begin(staves), end(staves)
                 , mem_fun_ref(&std::future<bool>::get)))
        return false;
    }

    return unfold(score);
  }

  result_type unfold (ast::score &score)
  {
    for (ast::part const &part: score.parts) {
      score.unfolded_part.emplace_back();
      if (!unfold(part, score.unfolded_part.back())) return false;
    }
    return true;
  }
  result_type unfold ( ast::part const &source
                     , ast::unfolded::part &target
                     )
  {
    BOOST_ASSERT(!source.empty());
    size_t const staves{source.front().paragraphs.size()};
    BOOST_ASSERT(std::all_of(std::next(source.begin()), source.end(),
                             [staves](ast::section const &section) -> bool {
                               return section.paragraphs.size() == staves;
                             }));
    for (unsigned int i = 0; i < staves; ++i) target.emplace_back();
    for (ast::section const &section: source) {
      int i = 0;
      for (ast::paragraph const &paragraph: section.paragraphs) {
        if (!unfold(paragraph, target.at(i++))) return false;
      }
    }

    std::size_t staff_nr = 0;
    for (ast::unfolded::staff &staff: target) {
      doubling_decoder undouble(report_error);
      if (!std::all_of(staff.begin(), staff.end(), apply_visitor(undouble)))
        return false;
      staff_nr += 1;
    }

    return true;
  }
  result_type unfold ( ast::paragraph const &source
                     , ast::unfolded::staff &target
                     )
  {
    staff_converter unfold(target);
    return std::all_of(source.begin(), source.end(), apply_visitor(unfold));
  }

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
