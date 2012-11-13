// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/braille/semantic_analysis/value_disambiguator.hpp"
#include "bmc/braille/semantic_analysis/value_disambiguation.hpp"

namespace music { namespace braille {

value_disambiguator::value_disambiguator(report_error_type const& report_error)
: compiler_pass(report_error)
, time_signature(4, 4)
, prev_duration(0)
, anacrusis(new value_disambiguation::measure_interpretations())
{}

value_disambiguator::~value_disambiguator()
{
  delete anacrusis;
}

value_disambiguator::result_type
value_disambiguator::operator()(ast::measure& measure)
{
  value_disambiguation::measure_interpretations
  interpretations(measure, time_signature, prev_duration);

  if (not interpretations.contains_complete_measure() and
      not interpretations.empty()) {
    if (anacrusis->empty()) {
      *anacrusis = interpretations;
      prev_duration = 0;
      return true;
    } else {
      if (anacrusis->completes_uniquely(interpretations)) {
        for (auto& lhs: *anacrusis) {
          for (auto& rhs: interpretations) {
            if (duration(lhs) + duration(rhs) == time_signature) {
              lhs.accept(), rhs.accept();
              prev_duration = duration(rhs);
              anacrusis->clear();
              return true;
            }
          }
        }
      }
    }
  }

  if (interpretations.size() == 1) {
    interpretations.front().accept();
    prev_duration = duration(interpretations.front());
    return true;
  }

  if (interpretations.empty()) {
    report_error(measure.id, L"No possible interpretations");
  } else {
    std::wstringstream s;
    s << interpretations.size() << L" possible interpretations:";
    for (auto& interpretation: interpretations) {
      rational const score(interpretation.harmonic_mean());
      s << std::endl
        << boost::rational_cast<float>(score) << L": " << interpretation;
    }
    report_error(measure.id, s.str());
  }
  return false;
}

bool
value_disambiguator::end_of_staff() const {
  if (not anacrusis->empty())
    report_error(anacrusis->get_measure_id(), L"Unterminated anacrusis");
  return anacrusis->empty();
}

}}
