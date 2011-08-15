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
compiler::operator()(ambiguous::measure& measure)
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
            }
          }
        }
        anacrusis.clear();
        return true;
      }
    }
  }
  if (interpretations.size() != 1) {
    if (interpretations.empty()) {
      report_error(measure.id, L"No possible interpretations");
    } else {
      std::wstringstream s;
      s << interpretations.size() << L" possible interpretations:";
      BOOST_FOREACH(proxied_measure const& measure, interpretations) {
        s << std::endl << measure;
      }
      report_error(measure.id, s.str());
    }
    return false;
  }

  accept(interpretations.front());
  return true;
}

compiler::result_type
compiler::operator()(ambiguous::score& score)
{
  bool success = true;

  if (score.time_sig) {
    global_time_signature = *(score.time_sig);
  }

  BOOST_FOREACH(ambiguous::part& part, score.parts)
    BOOST_FOREACH(ambiguous::staff& staff, part)
    {
      ambiguous::staff::iterator iterator(staff.begin());
      while (success && iterator != staff.end()) 
        success = boost::apply_visitor(*this, *iterator++);
    }
  return success;
}

}}

