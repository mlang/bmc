#include "value_disambiguator.hpp"

namespace music { namespace braille {

value_disambiguator::result_type
value_disambiguator::operator()( ambiguous::measure& measure
                               , time_signature const& time_sig
                               )
{
  value_disambiguation::measure_interpretations interpretations(measure, time_sig);

  if (not interpretations.contains_complete_measure() and
      not interpretations.empty()) {
    if (anacrusis.empty()) {
      anacrusis = interpretations;
      return true;
    } else {
      if (anacrusis.completes_uniquely(interpretations)) {
        for (auto& lhs: anacrusis) {
          for (auto& rhs: interpretations) {
            if (duration(lhs) + duration(rhs) == time_sig) {
              accept(lhs), accept(rhs);
              anacrusis.clear();
              return true;
            }
          }
        }
      }
    }
  }

  if (interpretations.size() == 1) {
    accept(interpretations.front());
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

}}
