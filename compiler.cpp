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
{ return disambiguate(measure) && fill_octaves(measure); }

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

struct is_pitched : boost::static_visitor<bool>
{
  result_type operator()(ambiguous::note const&) const { return true; }
  result_type operator()(ambiguous::chord const&) const { return true; }
  template<typename Sign>
  result_type operator()(Sign const&) const { return false; }
};

struct has_octave : boost::static_visitor<bool>
{
  result_type operator()(ambiguous::note const& note) const
  { return note.octave; }
  result_type operator()(ambiguous::chord const& chord) const
  { return (*this)(chord.base); }
  template<typename Sign>
  result_type operator()(Sign const&) const { return false; }
};

struct get_octave : boost::static_visitor<int>
{
  result_type operator()(ambiguous::note const& note) const
  { return *note.octave; }
  result_type operator()(ambiguous::chord const& chord) const
  { return (*this)(chord.base); }
  template<typename Sign>
  result_type operator()(Sign const&) const { return -1; }
};

class set_octave : public boost::static_visitor<int>
{
  unsigned octave;
public:
  set_octave(unsigned octave) : octave(octave) {}
  result_type operator()(ambiguous::note& note) const
  { return note.real_octave = octave; }
  result_type operator()(ambiguous::chord& chord) const
  { return (*this)(chord.base); }
  template<typename Sign>
  result_type operator()(Sign const&) const { return -1; }
};

struct get_step : boost::static_visitor<int>
{
  result_type operator()(ambiguous::note const& note) const
  { return note.step; }
  result_type operator()(ambiguous::chord const& chord) const
  { return (*this)(chord.base); }
  template<typename Sign>
  result_type operator()(Sign const&) const { return -1; }
};

compiler::result_type
compiler::fill_octaves(ambiguous::measure& measure)
{
  bool octave_required = measure.voices.size() > 1;
  BOOST_FOREACH(ambiguous::voice& voice, measure.voices) {
    BOOST_FOREACH(ambiguous::partial_measure& part, voice) {
      octave_required = octave_required || part.size() > 1;
      BOOST_FOREACH(ambiguous::partial_voice& partial_voice, part) {
        bool first = true;
        BOOST_FOREACH(ambiguous::sign& sign, partial_voice) {
          if (boost::apply_visitor(is_pitched(), sign) && first) {
            if (not boost::apply_visitor(has_octave(), sign) && last_octave == -1) {
              report_error(measure.id, L"Missing octave mark?");
              return false;
            }
            first = false;
          }
          if (boost::apply_visitor(is_pitched(), sign)) {
            if (boost::apply_visitor(has_octave(), sign)) {
              last_octave = boost::apply_visitor(get_octave(), sign);
              boost::apply_visitor(set_octave(last_octave), sign);
              last_step = boost::apply_visitor(get_step(), sign);
            } else {
              int const step(boost::apply_visitor(get_step(), sign));
              if ((step == 0 && (last_step == 6 || last_step == 5)) ||
                  (step == 1 && last_step == 6)) {
                last_octave = boost::apply_visitor(set_octave(last_octave + 1),
                                                   sign);
              } else if ((step == 6 && (last_step == 0 || last_step == 1)) ||
                         (step == 5 && last_step == 0)) {
                last_octave = boost::apply_visitor(set_octave(last_octave - 1),
                                                   sign);
              } else {
                boost::apply_visitor(set_octave(last_octave), sign);
              }
              last_step = step;
            }
          }
        }
      }
    }
  }
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
      if (not anacrusis.empty()) return false;
      last_octave = -1, last_step = -1;
    }
  return success;
}

}}

