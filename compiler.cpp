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

enum value_category
{
  large, small
};

class value_proxy
{
  ambiguous::value value_type;
  value_category category;
  unsigned dots;

  rational duration;
  rational* final_type;

  void init() {
    rational const base(undotted_duration());
    duration = base * 2 - base / pow(2, dots);
  }

public:
  value_proxy(ambiguous::note& note, value_category const& category)
  : value_type(note.ambiguous_value), category(category), dots(note.dots)
  , final_type(&note.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  value_proxy(ambiguous::note& note, value_category const& category, ambiguous::value value_type)
  : value_type(value_type), category(category), dots(note.dots)
  , final_type(&note.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  value_proxy(ambiguous::rest& rest, value_category const& category)
  : value_type(rest.ambiguous_value), category(category), dots(rest.dots)
  , final_type(&rest.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  value_proxy(ambiguous::rest& rest, value_category const& category, ambiguous::value value_type)
  : value_type(value_type), category(category), dots(rest.dots)
  , final_type(&rest.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  value_proxy(ambiguous::chord& chord, value_category const& category)
  : value_type(chord.base.ambiguous_value), category(category)
  , dots(chord.base.dots)
  , final_type(&chord.base.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  value_proxy(ambiguous::chord& chord, value_category const& category, ambiguous::value value_type)
  : value_type(value_type), category(category), dots(chord.base.dots)
  , final_type(&chord.base.type)
  { init(); BOOST_ASSERT(*final_type == music::rational()); }

  rational
  undotted_duration() const
  {
    BOOST_ASSERT(category==large || category==small);
    switch (value_type) {
    case ambiguous::whole_or_16th:
      return rational(1, category==large? 1: 16);
    case ambiguous::half_or_32th:
      return rational(1, category==large? 2: 32);
    case ambiguous::quarter_or_64th:
      return rational(1, category==large? 4: 64);
    case ambiguous::eighth_or_128th:
      return rational(1, category==large? 8: 128);
    default:
      BOOST_ASSERT(false);
    }
  }

  void set_final_type()
  { *final_type = undotted_duration(); }

  rational const& as_rational() const
  { return duration; }

  bool operator==(value_proxy const& rhs) const
  { return final_type == rhs.final_type && as_rational() == rhs.as_rational(); }
};

typedef std::vector<value_proxy> proxied_partial_voice;

inline
rational
operator+(rational const& r, proxied_partial_voice::const_reference v)
{ return r + v.as_rational(); }

inline
rational
duration(proxied_partial_voice const& values)
{ return boost::accumulate(values, rational(0)); }

class partial_voice_interpretations : public std::vector<proxied_partial_voice>
{
  class is_value_distinction : public boost::static_visitor<bool>
  {
    ambiguous::value_distinction expected;
  public:
    is_value_distinction(ambiguous::value_distinction const& distinction)
    : expected(distinction) {}

    result_type operator()(ambiguous::value_distinction const& distinction) const
    { return distinction == expected; }

    template<class Sign>
    result_type operator()(Sign const&) const
    { return false; }
  };
  struct has_value : boost::static_visitor<bool>
  {
    result_type operator()(ambiguous::note const&) const { return true; }
    result_type operator()(ambiguous::rest const&) const { return true; }
    result_type operator()(ambiguous::chord const&) const { return true; }
    template<typename Sign>
    result_type operator()(Sign const&) const { return false; }
  };
  struct get_value : boost::static_visitor<ambiguous::value>
  {
    result_type operator()(ambiguous::note const& note) const
    { return note.ambiguous_value; }
    result_type operator()(ambiguous::rest const& rest) const
    { return rest.ambiguous_value; }
    result_type operator()(ambiguous::chord const& chord) const
    { return (*this)(chord.base); }
    template<typename T>
    result_type operator()(T const&) const
    { return ambiguous::unknown; }
  };
  static
  ambiguous::partial_voice::iterator
  same_category_end( ambiguous::partial_voice::iterator& begin
                   , ambiguous::partial_voice::iterator const& end
                   , ambiguous::value_distinction const& distinction
                   )
  {
    if (boost::apply_visitor(is_value_distinction(distinction), *begin)) {
      begin = begin + 1;
      ambiguous::partial_voice::iterator iter(begin);
      if (iter != end &&
          boost::apply_visitor(has_value(), *iter)) {
        ambiguous::value initial(boost::apply_visitor(get_value(), *iter++));
        while (iter != end && apply_visitor(get_value(), *iter) == initial)
          ++iter;
        return iter;
      }
    }
    return begin;
  }
  static
  ambiguous::partial_voice::iterator
  notegroup_end( ambiguous::partial_voice::iterator const& begin
               , ambiguous::partial_voice::iterator const& end
               )
  {
    if (boost::apply_visitor(has_value(), *begin)) {
      if (boost::apply_visitor(get_value(), *begin) != ambiguous::eighth_or_128th) {
        ambiguous::partial_voice::iterator iter = begin + 1;
        while (iter != end &&
               boost::apply_visitor(has_value(), *iter) &&
               boost::apply_visitor(get_value(), *iter) == ambiguous::eighth_or_128th)
          ++iter;
        if (std::distance(begin, iter) > 2) return iter;
      }
    }
    return begin;
  }
  struct large_and_small : std::vector<value_proxy>, boost::static_visitor<void>
  {
    template<typename Sign>
    large_and_small(Sign& sign) : std::vector<value_proxy>()
    { boost::apply_visitor(*this, sign); }

    template<class Value>
    result_type operator()(Value& value)
    {
      if (not is_grace(value)) {
        push_back(value_type(value, large));
        push_back(value_type(value, small));
      }
    }
    result_type operator()(ambiguous::value_distinction&) {}
    result_type operator()(ambiguous::hand_sign&) {}
    result_type operator()(ambiguous::barline&) {}
    result_type operator()(ambiguous::simile&) {}
    bool is_grace(ambiguous::note const& note) const
    {
      return std::find(note.articulations.begin(),
                       note.articulations.end(),
                       ambiguous::appoggiatura)
             != note.articulations.end()
             ||
             std::find(note.articulations.begin(),
                       note.articulations.end(),
                      ambiguous::short_appoggiatura)
              != note.articulations.end();
    }
    bool is_grace(ambiguous::rest const& rest) const
    { return false; }
    bool is_grace(ambiguous::chord const& chord) const
    { return is_grace(chord.base); }
  };
  class notegroup
  : public proxied_partial_voice
  , public boost::static_visitor<void>
  {
    value_category const category;
    ambiguous::value type;
  public:
    notegroup( ambiguous::partial_voice::iterator const& begin
             , ambiguous::partial_voice::iterator const& end
             , value_category const& category
             )
    : proxied_partial_voice(), category(category), type(ambiguous::unknown)
    { std::for_each(begin, end, boost::apply_visitor(*this)); }

    result_type operator()(ambiguous::note& note)
    {
      if (type == ambiguous::unknown) type = note.ambiguous_value;
      value_proxy proxy(note, category, type);
      push_back(proxy);
    }
    result_type operator()(ambiguous::rest& rest)
    {
      if (type == ambiguous::unknown) type = rest.ambiguous_value;
      push_back(value_proxy(rest, category, type));
    }
    result_type operator()(ambiguous::chord& chord)
    {
      if (type == ambiguous::unknown) type = chord.base.ambiguous_value;
      push_back(value_proxy(chord, category, type));
    }
    result_type operator()(ambiguous::value_distinction&) {}
    result_type operator()(ambiguous::hand_sign&) {}
    result_type operator()(ambiguous::barline&) {}
    result_type operator()(ambiguous::simile&) {}
  };
  class same_category
  : public proxied_partial_voice
  , public boost::static_visitor<void>
  {
    value_category category;
  public:
    same_category( ambiguous::partial_voice::iterator const& begin
                 , ambiguous::partial_voice::iterator const& end
                 , value_category const& category
                 )
    : proxied_partial_voice(), category(category)
    { std::for_each(begin, end, boost::apply_visitor(*this)); }

    result_type operator()(ambiguous::note& note)
    { push_back(value_proxy(note, category)); }
    result_type operator()(ambiguous::rest& rest)
    { push_back(value_proxy(rest, category)); }
    result_type operator()(ambiguous::chord& chord)
    { push_back(value_proxy(chord, category)); }
    result_type operator()(ambiguous::value_distinction&) {}
    result_type operator()(ambiguous::hand_sign&) {}
    result_type operator()(ambiguous::barline&) {}
    result_type operator()(ambiguous::simile&) {}
  };

  static
  std::vector<value_type>
  recurse( ambiguous::partial_voice::iterator begin
         , ambiguous::partial_voice::iterator const& end
         , const_reference stack
         , rational const& max_duration
         , rational const& position
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      if (not stack.empty()) result.push_back(stack);
    } else {
      ambiguous::partial_voice::iterator tail;
      if (position % rational(1, 4) == zero &&
          (tail = notegroup_end(begin, end)) > begin) {
        { // Large group
          notegroup group(begin, tail, large);
          if (duration(group) <= max_duration) {
            value_type new_stack(stack);
            boost::range::insert(new_stack, new_stack.end(), group);
            boost::range::insert(result, result.end(),
                                 recurse(tail, end, new_stack,
                                         max_duration - duration(group),
                                         position + duration(group)));
          }
        }
        { // Small group
          notegroup group(begin, tail, small);
          if (duration(group) <= max_duration) {
            value_type new_stack(stack);
            boost::range::insert(new_stack, new_stack.end(), group);
            boost::range::insert(result, result.end(),
                                 recurse(tail, end, new_stack,
                                         max_duration - duration(group),
                                         position + duration(group)));
          }
        }
        large_and_small possibilities(*begin);
        tail = begin + 1;
        if (possibilities.empty()) {
          boost::range::insert(result, result.end(),
                               recurse(tail, end, stack,
                                       max_duration, position));
        } else {
          BOOST_FOREACH(large_and_small::const_reference value, possibilities) {
            if (value.as_rational() <= max_duration) {
              value_type new_stack(stack);
              new_stack.push_back(value);
              boost::range::insert(result, result.end(),
                                   recurse(tail, end, new_stack,
                                           max_duration - value.as_rational(),
                                           position + value.as_rational()));
            }
          }
        }
      } else if ((tail = same_category_end(begin, end,
                                           ambiguous::large_follows)) > begin) {
        same_category group(begin, tail, large);
        if (duration(group) <= max_duration) {
          value_type new_stack(stack);
          boost::range::insert(new_stack, new_stack.end(), group);
          boost::range::insert(result, result.end(),
                               recurse(tail, end, new_stack,
                                       max_duration - duration(group),
                                       position + duration(group)));
        }
      } else if ((tail = same_category_end(begin, end,
                                           ambiguous::small_follows)) > begin) {
        same_category group(begin, tail, small);
        if (duration(group) <= max_duration) {
          value_type new_stack(stack);
          boost::range::insert(new_stack, new_stack.end(), group);
          boost::range::insert(result, result.end(),
                               recurse(tail, end, new_stack,
                                       max_duration - duration(group),
                                       position + duration(group)));
        }
      } else {
        large_and_small possibilities(*begin);
        if (possibilities.empty()) {
          return recurse(begin + 1, end, stack, max_duration, position);
        } else {
          BOOST_FOREACH(large_and_small::const_reference value, possibilities) {
            if (value.as_rational() <= max_duration) {
              value_type new_stack(stack);
              new_stack.push_back(value);
              boost::range::insert(result, result.end(),
                                   recurse(begin + 1, end, new_stack,
                                           max_duration - value.as_rational(),
                                           position + value.as_rational()));
            }
          }
        }
      }
    }
    return result;
  }

public:
  partial_voice_interpretations( ambiguous::partial_voice& voice
                               , rational const& max_duration
                               , rational const& position
                               )
  : std::vector<value_type>(recurse(voice.begin(), voice.end(),
                                    value_type(), max_duration, position))
  {}
};

typedef std::vector<proxied_partial_voice> proxied_partial_measure;

class partial_measure_interpretations
: public std::vector<proxied_partial_measure>
{
  static
  std::vector<value_type>
  recurse( ambiguous::partial_measure::iterator const& begin
         , ambiguous::partial_measure::iterator const& end
         , const_reference stack
         , rational const& length, rational const& position
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      ambiguous::partial_measure::iterator const tail = begin + 1;
      BOOST_FOREACH(partial_voice_interpretations::const_reference possibility,
                    partial_voice_interpretations(*begin, length, position)) {
        if (duration(possibility) == length) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
          boost::range::insert(result, result.end(),
                               recurse(tail, end, new_stack, length, position));
        }
      }
    }
    return result;
  }
public:
  partial_measure_interpretations( ambiguous::partial_measure& partial_measure
                                 , rational const& max_length
                                 , rational const& position
                                 )
  : std::vector<proxied_partial_measure>()
  {
    if (not partial_measure.empty()) {
      BOOST_FOREACH(partial_voice_interpretations::const_reference possibility,
                    partial_voice_interpretations(partial_measure.front(),
                                                  max_length, position)) {
        boost::range::insert(*this, this->end(),
                             recurse(partial_measure.begin() + 1,
                                     partial_measure.end(),
                                     value_type(1, possibility),
                                     duration(possibility), position));
      }
    }
  }
};

rational
duration(proxied_partial_measure const& voices)
{
  rational value(0);
  if (not voices.empty()) {
    value = duration(voices.front());
    for (proxied_partial_measure::const_iterator
         voice = voices.begin() + 1; voice != voices.end(); ++voice) {
      BOOST_ASSERT(value == duration(*voice));
    }
  }
  return value;
}

typedef std::vector<proxied_partial_measure> proxied_voice;

rational
operator+(rational const& r, proxied_voice::const_reference part)
{ return r + duration(part); }

rational
duration(proxied_voice const& parts)
{ return boost::accumulate(parts, rational(0)); }

class voice_interpretations : public std::vector<proxied_voice>
{
  static
  std::vector<value_type>
  recurse( ambiguous::voice::iterator const& begin
         , ambiguous::voice::iterator const& end
         , const_reference stack
         , rational const& max_length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      if (not stack.empty()) result.push_back(stack);
    } else {
      ambiguous::voice::iterator const tail = begin + 1;
      BOOST_FOREACH(partial_measure_interpretations::const_reference possibility,
                    partial_measure_interpretations(*begin, max_length,
                                                    duration(stack))) {
        value_type new_stack(stack);
        new_stack.push_back(possibility);
        boost::range::insert(result, result.end(),
                             recurse(tail, end, new_stack,
                                     max_length - duration(possibility)));
      }
    }
    return result;
  }

public:
  voice_interpretations(ambiguous::voice& voice, rational max_length)
  : std::vector<proxied_voice>(recurse(voice.begin(), voice.end(),
                                       value_type(), max_length))
  {}
};

typedef std::vector<proxied_voice> proxied_measure;

rational
duration(proxied_measure const& voices)
{
  rational value;
  if (not voices.empty()) {
    value = duration(voices.front());
    for (proxied_measure::const_iterator
	 voice = voices.begin() + 1; voice != voices.end(); ++voice) {
      BOOST_ASSERT(value == duration(*voice));
    }
  }
  return value;
}

class measure_interpretations
: public std::vector<proxied_measure>
{
  rational const max_duration;

  std::vector<value_type>
  recurse( ambiguous::measure::iterator const& begin
         , ambiguous::measure::iterator const& end
         , const_reference stack, rational const& real_length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      ambiguous::measure::iterator const tail = begin + 1;
      BOOST_FOREACH(voice_interpretations::const_reference possibility,
                    voice_interpretations(*begin, max_duration)) {
        if (real_length == duration(possibility)) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
          boost::range::insert(result, result.end(),
                               recurse(tail, end, new_stack, real_length));
        }
      }
    }
    return result;
  }
public:
  measure_interpretations( ambiguous::measure& measure
                         , rational const& max_duration
                         )
  : max_duration(max_duration)
  , std::vector<value_type>()
  {
    BOOST_ASSERT(not measure.empty());
    BOOST_ASSERT(max_duration >= 0);
    BOOST_FOREACH(voice_interpretations::const_reference possibility,
                  voice_interpretations(measure.front(), max_duration)) {
      boost::range::insert(*this, end(),
                           recurse(measure.begin() + 1, measure.end(),
                                   value_type(1, possibility),
                                   duration(possibility)));
    }

    if (size() > 1 && contains_complete_measure())
      for (iterator measure = begin(); measure != end();
           measure = duration(*measure) != max_duration? erase(measure): measure + 1);
  }
  bool contains_complete_measure() const {
    for (const_iterator measure = begin(); measure != end(); ++measure) {
      if (max_duration == duration(*measure)) return true;
    }
    return false;
  }
};

void
accept(proxied_measure& measure)
{
  BOOST_FOREACH(proxied_measure::reference voice, measure)
    BOOST_FOREACH(proxied_voice::reference part, voice)
      BOOST_FOREACH(proxied_partial_measure::reference partial_voice, part)
        BOOST_FOREACH(proxied_partial_voice::reference value, partial_voice)
          value.set_final_type();
}

template<typename Char>
std::basic_ostream<Char>&
operator<<(std::basic_ostream<Char>& os, proxied_measure const& measure)
{
  BOOST_FOREACH(proxied_measure::const_reference voice, measure) {
    os << '[';
    BOOST_FOREACH(proxied_voice::const_reference part, voice) {
      os << '{';
      BOOST_FOREACH(proxied_partial_measure::const_reference partial_voice, part) {
        os << '(';
        BOOST_FOREACH(proxied_partial_voice::const_reference value, partial_voice) {
          os << '<' << value.as_rational().numerator() << '/' << value.as_rational().denominator() << '>';
        }
        os << ')';
      }
      os << '}';
    }
    os << ']';
  }
  return os;
}

compiler::result_type
compiler::operator()(ambiguous::measure& measure) const
{
  measure_interpretations interpretations(measure, global_time_signature);

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

