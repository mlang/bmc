// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <sstream>
#include "bmc/braille/semantic_analysis/value_disambiguator.hpp"
#include "bmc/braille/semantic_analysis/value_disambiguation.hpp"

namespace bmc { namespace braille {

value_disambiguator::value_disambiguator(report_error_type const& report_error)
: compiler_pass(report_error)
, time_signature(4, 4)
, prev_duration(0)
{}

value_disambiguator::result_type
value_disambiguator::operator()(ast::measure& measure)
{
  value_disambiguation::measure_interpretations
  interpretations(measure, time_signature, prev_duration, prev_doubled_tuplets);

  if (not interpretations.contains_complete_measure() and
      not interpretations.empty()) {
    if (not anacrusis) {
      anacrusis = interpretations;
      prev_duration = 0;
      prev_doubled_tuplets.clear();
      return true;
    } else {
      if (anacrusis->completes_uniquely(interpretations)) {
        for (auto& lhs: *anacrusis) {
          for (auto& rhs: interpretations) {
            if (duration(lhs) + duration(rhs) == time_signature) {
              lhs.accept(), rhs.accept();
              prev_duration = duration(rhs);
              prev_doubled_tuplets = rhs.get_doubled_tuplets();
              anacrusis.reset();
              return true;
            }
          }
        }
      }
    }
  }

  if (interpretations.size() == 1) {
    auto &proxied_measure = interpretations.front();
    proxied_measure.accept();
    prev_duration = duration(proxied_measure);
    prev_doubled_tuplets = proxied_measure.get_doubled_tuplets();
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
  if (anacrusis and anacrusis->size() > 1) {
    std::wstringstream msg;
    msg << "Unterminated anacrusis";
    report_error(anacrusis->get_measure_id(), msg.str());
    return false;
  }
  if (anacrusis and not anacrusis->empty()) anacrusis->front().accept();
  return true;
}

}}
