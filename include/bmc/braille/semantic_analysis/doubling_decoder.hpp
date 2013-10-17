// Copyright (C) 2013  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_DOUBLING_DECODER_HPP
#define BMC_DOUBLING_DECODER_HPP

#include <boost/variant/static_visitor.hpp>
#include "bmc/braille/ast.hpp"
#include "compiler_pass.hpp"

namespace music { namespace braille {

class doubling_decoder : public compiler_pass, public boost::static_visitor<bool>
{
  std::size_t staff_number;
  struct info
  {
    enum state { active, final, none };
    enum state slur = none;
    enum state staccato = none;
  }; 
  std::map<std::size_t, info> states;
  info *current;
public:
  doubling_decoder( report_error_type const &report_error
                  , std::size_t staff_number
                  )
  : compiler_pass{report_error}
  , staff_number{staff_number}, states{}, current{nullptr}
  {}

  result_type operator()(ast::unfolded::measure &measure) {
    unsigned full_voice_nr = 0;
    for (ast::unfolded::voice &voice: measure.voices) {
      std::vector<std::size_t> measure_part_voices;
      for (ast::unfolded::partial_measure &partial_measure: voice) {
        if (not partial_measure.empty()) {
          measure_part_voices.push_back(0);
          for (ast::unfolded::partial_voice &partial_voice: partial_measure) {
            current = &states[full_voice_nr + measure_part_voices.back()];

            if (not std::all_of(partial_voice.begin(), partial_voice.end(),
                                apply_visitor(*this))) return false;

            measure_part_voices.back() += 1;
          }
        }
      }

      auto max_voices = std::max_element(measure_part_voices.begin(), measure_part_voices.end());
      // A full measure voice without at least one partial_voice inside should
      // never happen.
      BOOST_ASSERT(max_voices != measure_part_voices.end());
      full_voice_nr += *max_voices;
    }
    return true;
  }
  result_type operator()(ast::key_and_time_signature &) {
    return true;
  }

  result_type operator()(ast::note &note)
  {
    if (note.slurs.size() == 2) {
    }
    if (std::count(note.articulations.begin(), note.articulations.end(), articulation::staccato) > 2) {
      report_error(note.id, L"Too many staccati.");
      return false;
    }
    if (std::count(note.articulations.begin(), note.articulations.end(), articulation::staccato) == 2) {
      if (current->staccato != info::none) {
        report_error(note.id, L"Starting double staccato while already active");
        return false;
      }
      note.articulations.erase(std::find( note.articulations.begin()
                                        , note.articulations.end()
                                        , articulation::staccato));
      BOOST_ASSERT(std::count( note.articulations.begin()
                             , note.articulations.end()
                             , articulation::staccato) == 1);
      current->staccato = info::active;
    } else {
      if (current->staccato != info::none) {
        if (std::count(note.articulations.begin(), note.articulations.end(), articulation::staccato) == 0) {
          note.articulations.push_back(articulation::staccato);
        } else {
          current->staccato = info::none;
        }
      }
    }
    return true;
  }
  template<typename T>
  result_type operator()(T &) { return true; }
};

}}

#endif
