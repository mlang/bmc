// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "compiler.hpp"
#include <cmath>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/range/numeric.hpp>
#include <boost/range/algorithm_ext/insert.hpp>

namespace music { namespace braille {

compiler::result_type
compiler::operator()(ambiguous::score& score)
{
  if (score.time_sig) {
    global_time_signature = *(score.time_sig);
  }

  BOOST_FOREACH(ambiguous::part& part, score.parts) {
    BOOST_FOREACH(ambiguous::staff& staff, part) {
      bool ok = true;
      ambiguous::staff::iterator iterator(staff.begin());
      while (ok && iterator != staff.end())
        ok = boost::apply_visitor(*this, *iterator++);
      if (not ok) return false;

      if (not anacrusis.empty()) {
        std::wcout << L"Unfinished anacrusis" << std::endl;
        return false;
      }
      calculate_octaves.clear();
    }
  }
  return true;
}

compiler::result_type
compiler::operator()(ambiguous::measure& measure)
{ return disambiguate(measure) && calculate_octaves(measure); }

compiler::result_type
compiler::disambiguate(ambiguous::measure& measure)
{
  measure_interpretations interpretations(measure, global_time_signature);

  if (not interpretations.contains_complete_measure() and
      not interpretations.empty()) {
    if (anacrusis.empty()) {
      anacrusis = interpretations;
      return true;
    } else {
      if (anacrusis.completes_uniquely(interpretations)) {
        BOOST_FOREACH(proxied_measure& lhs, anacrusis) {
          BOOST_FOREACH(proxied_measure& rhs, interpretations) {
            if (duration(lhs) + duration(rhs) == global_time_signature) {
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
    BOOST_FOREACH(proxied_measure const& measure, interpretations) {
      rational const score(harmonic_mean(measure));
      s << std::endl << boost::rational_cast<float>(score) << L": " << measure;
    }
    report_error(measure.id, s.str());
  }
  return false;
}

}}

