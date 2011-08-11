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
};

inline
rational
duration(value_proxy const& proxy)
{ return proxy.as_rational(); }

class value_proxy_list
: public std::vector< std::vector< std::vector<value_proxy> > >
{
public:
  class distinction_sign
  : public boost::static_visitor<bool>
  {
    ambiguous::value_distinction expected_distinction;
  public:
    distinction_sign(ambiguous::value_distinction const& distinction)
    : expected_distinction(distinction)
    {}

    result_type
    operator()(ambiguous::value_distinction const& distinction) const
    { return distinction == expected_distinction; }

    template<class T>
    result_type operator()(T const&) const
    { return false; }
  };

  class has_value
  : public boost::static_visitor<bool>
  {
  public:
    result_type operator()(ambiguous::note const&) const
    { return true; }
    result_type operator()(ambiguous::rest const&) const
    { return true; }
    result_type operator()(ambiguous::chord const&) const
    { return true; }
    template<typename T>
    result_type operator()(T const&) const
    { return false; }
  };
  class get_value
  : public boost::static_visitor<ambiguous::value>
  {
  public:
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

  class combinations
  : public value_type
  , public boost::static_visitor<void>
  {
    std::vector<value_type> values;

    static
    std::vector<value_type>
    recurse( std::vector<value_type>::iterator const& begin
	   , std::vector<value_type>::iterator const& end
           , const_reference stack, rational const& max_duration
	   )
    {
      std::vector<value_type> result;
      if (begin == end) {
	result.push_back(stack);
      } else {
	std::vector<value_type>::iterator const rest = begin + 1;
	BOOST_FOREACH(value_proxy const& v, *begin) {
	  rational const value_duration(duration(v));
	  if (value_duration <= max_duration) {
	    value_type new_stack(stack);
	    new_stack.push_back(v);
	    boost::range::insert(result, result.end(),
				 recurse(rest, end, new_stack,
					 max_duration - value_duration));
	  }
	}
      }
      return result;
    }
  public:
    combinations( ambiguous::partial_voice::iterator const& begin
                , ambiguous::partial_voice::iterator const& end
		, rational const& max_duration
                )
    : std::vector<value_type>()
    {
      std::for_each(begin, end, boost::apply_visitor(*this));
      if (not values.empty()) {
	BOOST_FOREACH(value_proxy const& v, values.front()) {
	  boost::range::insert(*this, this->end(),
			       recurse(values.begin() + 1, values.end(),
				       value_type(1, v),
				       max_duration - duration(v)));
	}
      }
      values.clear();
    }

    void operator()(ambiguous::note& note) {
      if (not (std::find(note.articulations.begin(),
                         note.articulations.end(),
                         ambiguous::appoggiatura)
               != note.articulations.end()
               ||
               std::find(note.articulations.begin(),
                         note.articulations.end(),
                         ambiguous::short_appoggiatura)
               != note.articulations.end())) {
        value_type possibilities;
        possibilities.push_back(value_proxy(note, large));
        possibilities.push_back(value_proxy(note, small));
        values.push_back(possibilities);
      }
    }
    void operator()(ambiguous::rest& value) {
      value_type possibilities;
      possibilities.push_back(value_proxy(value, large));
      possibilities.push_back(value_proxy(value, small));
      values.push_back(possibilities);
    }
    void operator()(ambiguous::chord& value) {
      value_type possibilities;
      possibilities.push_back(value_proxy(value, large));
      possibilities.push_back(value_proxy(value, small));
      values.push_back(possibilities);
    }
    void operator()(ambiguous::simile&)
    {}
    void operator()(ambiguous::value_distinction&)
    {}
    void operator()(ambiguous::hand_sign&)
    {}
    void operator()(ambiguous::barline&)
    {}
  };

  class same_category
  : public value_type
  , public boost::static_visitor<void>
  {
    value_category category;
  public:
    same_category( ambiguous::partial_voice::iterator const& begin
                 , ambiguous::partial_voice::iterator const& end
                 , value_category const& category
	         )
    : std::vector< std::vector<value_proxy> >()
    , category(category)
    { std::for_each(begin, end, boost::apply_visitor(*this)); }

    template<class Value>
    void operator()(Value& value)
    { push_back(value_type(1, value_proxy(value, category))); }
    void operator()(ambiguous::simile&)
    { BOOST_ASSERT(false); }
    void operator()(ambiguous::value_distinction&)
    { BOOST_ASSERT(false); }
    void operator()(ambiguous::hand_sign&)
    {}
    void operator()(ambiguous::barline&)
    {}
  };

  class notegroup
  : public combinations
  {
    value_type group;
    value_category category;
    ambiguous::value faked_type;
  public:
    notegroup( ambiguous::partial_voice::iterator const& begin
             , ambiguous::partial_voice::iterator const& end
	     , rational const& max_duration
             )
    : combinations(begin, end, max_duration)
    {
      BOOST_ASSERT(begin != end);
      group.clear();
      faked_type = boost::apply_visitor(get_value(), *begin);

      category = large;
      std::for_each(begin, end, boost::apply_visitor(*this));
      push_back(group);
      group.clear();

      category = small;
      std::for_each(begin, end, boost::apply_visitor(*this));
      push_back(group);

      BOOST_ASSERT(size() > 2);
      BOOST_FOREACH(const_reference element, *this)
        BOOST_ASSERT(front().size() == element.size());
    }

    template<class Value>
    void operator()(Value& value)
    {
      group.push_back(value_proxy(value, category, faked_type));
    }
    void operator()(ambiguous::value_distinction&)
    { BOOST_ASSERT(false); }
    void operator()(ambiguous::hand_sign&)
    { BOOST_ASSERT(false); }
    void operator()(ambiguous::barline&)
    { BOOST_ASSERT(false); }
    void operator()(ambiguous::simile&)
    { BOOST_ASSERT(false); }
  };

private:
  ambiguous::partial_voice::iterator
  same_category_end( ambiguous::partial_voice::iterator& begin
	           , ambiguous::partial_voice::iterator const& end
		   , ambiguous::value_distinction const& distinction
                   ) const
  {
    if (boost::apply_visitor(distinction_sign(distinction), *begin)) {
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
  ambiguous::partial_voice::iterator
  notegroup_end( ambiguous::partial_voice::iterator const& begin
               , ambiguous::partial_voice::iterator const& end
               ) const
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

public:
  value_proxy_list( ambiguous::partial_voice& voice
                  , rational const& max_duration
                  )
  : std::vector<value_type>()
  {
    ambiguous::partial_voice::iterator begin = voice.begin();
    while (begin != voice.end()) {
      ambiguous::partial_voice::iterator end;
      if ((end = notegroup_end(begin, voice.end())) != begin) {
	push_back(notegroup(begin, end, max_duration));
      } else if ((end = same_category_end(begin, voice.end(),
					  ambiguous::large_follows)) != begin) {
	push_back(same_category(begin, end, large));
      } else if ((end = same_category_end(begin, voice.end(),
					  ambiguous::small_follows)) != begin) {
	push_back(same_category(begin, end, small));
      } else {
	if (begin != voice.end()) {
	  end = begin + 1;
	  push_back(combinations(begin, end, max_duration));
	}
      }
      std::advance(begin, std::distance(begin, end));
    }
  }
};

typedef std::vector<value_proxy> proxied_partial_voice;

inline
rational
operator+(rational const& r, proxied_partial_voice::const_reference v)
{ return r + duration(v); }

inline
rational
duration(proxied_partial_voice const& values)
{ return boost::accumulate(values, rational(0)); }

class partial_voice_interpretations
: public std::vector<proxied_partial_voice>
{
  static
  std::vector<value_type>
  recurse( value_proxy_list::iterator const& begin
         , value_proxy_list::iterator const& end
         , const_reference stack, rational const& max_length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      value_proxy_list::iterator const tail = begin + 1;
      if (begin->empty()) {
        return recurse(tail, end, stack, max_length);
      } else {
        BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
                      *begin) {
          rational const value_duration(duration(possibility));
          if (value_duration <= max_length) {
            value_type new_stack(stack);
            boost::range::insert(new_stack, new_stack.end(), possibility);
            boost::range::insert(result, result.end(),
                                 recurse(tail, end, new_stack,
                                         max_length - value_duration));
          }
        }
      }
    }
    return result;
  }  
public:
  partial_voice_interpretations( ambiguous::partial_voice& voice
			       , rational const& max_length
			       )
  : std::vector<proxied_partial_voice>()
  {
    value_proxy_list vpl(voice, max_length);
    if (not vpl.empty()) {
      value_proxy_list::iterator const tail = vpl.begin() + 1;
      if (vpl.front().empty()) {
        value_type stack;
        boost::range::insert(*this, end(),
                             recurse(tail, vpl.end(), stack, max_length));
      } else {
        BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
  		      vpl.front()) {
	  rational const value_duration(duration(possibility));
	  if (value_duration <= max_length) {
	    boost::range::insert(*this, end(),
	  	 	         recurse(tail, vpl.end(),
			                 possibility,
				         max_length - value_duration));
          }
        }
      }
    }
  }
};

typedef std::vector<proxied_partial_voice> proxied_partial_measure;

class partial_measure_interpretations
: public std::vector< proxied_partial_measure >
{
  static
  std::vector<value_type>
  recurse( ambiguous::partial_measure::iterator const& begin
         , ambiguous::partial_measure::iterator const& end
         , const_reference stack, rational const& length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      ambiguous::partial_measure::iterator const tail = begin + 1;
      BOOST_FOREACH(partial_voice_interpretations::const_reference possibility,
		    partial_voice_interpretations(*begin, length))
	if (duration(possibility) == length) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(tail, end, new_stack, length));
        }
    }
    return result;
  }
public:
  partial_measure_interpretations( ambiguous::partial_measure& partial_measure
                                 , rational const& max_length
                                 )
  : std::vector<proxied_partial_measure>()
  {
    if (not partial_measure.empty()) {
      BOOST_FOREACH(partial_voice_interpretations::const_reference possibility,
		    partial_voice_interpretations(partial_measure.front(),
						  max_length))
	boost::range::insert(*this, end(),
			     recurse(partial_measure.begin() + 1,
				     partial_measure.end(),
				     value_type(1, possibility),
				     duration(possibility)));
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

class voice_interpretations
: public std::vector<proxied_voice>
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
      result.push_back(stack);
    } else {
      ambiguous::voice::iterator const tail = begin + 1;
      BOOST_FOREACH(partial_measure_interpretations::const_reference possibility,
                    partial_measure_interpretations(*begin, max_length)) {
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
  : std::vector<proxied_voice>()
  {
    if (not voice.empty()) {
      BOOST_FOREACH(partial_measure_interpretations::const_reference possibility,
		    partial_measure_interpretations(voice.front(), max_length))
	boost::range::insert(*this, end(),
			     recurse(voice.begin() + 1, voice.end(),
				     value_type(1, possibility),
				     max_length - duration(possibility)));
    }
  }
};

rational
operator+(rational const& r, proxied_partial_measure const& p)
{ return r + duration(p); }

rational
duration(proxied_voice const& parts)
{ return boost::accumulate(parts, rational(0)); }

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
  : std::vector<proxied_measure>()
  , max_duration(max_duration)
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

