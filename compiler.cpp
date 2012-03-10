// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "compiler.hpp"
#include <cmath>
#include <sstream>
#include <boost/range/numeric.hpp>
#include <boost/range/algorithm_ext/insert.hpp>

namespace music { namespace braille {

compiler::result_type
compiler::operator()(ambiguous::score& score)
{
  if (score.time_sig) {
    global_time_signature = *score.time_sig;
  }

  for (ambiguous::part& part: score.parts) {
    for (ambiguous::staff& staff: part) {
      calculate_alterations.set(score.key_sig);
      bool ok = true;
      ambiguous::staff::iterator iterator(staff.begin());
      while (ok && iterator != staff.end())
        ok = boost::apply_visitor(*this, *iterator++);
      if (not ok) return false;

      calculate_octaves.clear();
    }
  }
  return true;
}

compiler::result_type
compiler::operator()(ambiguous::measure& measure)
{
  if (disambiguate_values(measure, global_time_signature))
    if (calculate_octaves(measure)) {
      calculate_alterations(measure);
      return true;
    }
  return false;
}

}}

