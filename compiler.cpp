#include "compiler.hpp"
#include <cmath>
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
  rational* final_type;
public:
  value_proxy(ambiguous::note& note, value_category const& category)
  : value_type(note.ambiguous_value), category(category), dots(note.dots)
  , final_type(&note.type)
  {
    BOOST_ASSERT(*final_type == music::rational());
  }

  value_proxy(ambiguous::rest& rest, value_category const& category)
  : value_type(rest.ambiguous_value), category(category), dots(rest.dots)
  , final_type(&rest.type)
  {
    BOOST_ASSERT(*final_type == music::rational());
  }

  value_proxy(ambiguous::chord& chord, value_category const& category)
  : value_type(chord.base.ambiguous_value), category(category), dots(chord.base.dots)
  , final_type(&chord.base.type)
  {
    BOOST_ASSERT(*final_type == music::rational());
  }

  void set_type(ambiguous::value type) { value_type = type; }

  rational undotted_duration() const {
    rational base;
    switch (value_type) {
    case ambiguous::whole_or_16th:
      base = rational(1, category==large? 1: 16);
      break;
    case ambiguous::half_or_32th:
      base = rational(1, category==large? 2: 32);
      break;
    case ambiguous::quarter_or_64th:
      base = rational(1, category==large? 4: 64);
      break;
    case ambiguous::eighth_or_128th:
      base = rational(1, category==large? 8: 128);
      break;
    default:
      BOOST_ASSERT(false);
    }
    return base;
  }

  void set_final_type()
  {
    *final_type = undotted_duration();
  }

  rational
  as_rational() const {
    return undotted_duration() * 2 - undotted_duration() / pow(2, dots);
  }
};

class value_proxy_list
: public std::vector< std::vector< std::vector<value_proxy> > >
{
public:
  class distinction_sign : public boost::static_visitor<bool>
  {
    ambiguous::value_distinction expected_distinction;
  public:
    distinction_sign(ambiguous::value_distinction const& distinction)
    : expected_distinction(distinction) {}
    bool operator()(ambiguous::value_distinction const& distinction) const {
      if (distinction == expected_distinction) return true;
      return false;
    }
    template<class T> bool operator()(T const&) const { return false; }
  };
  class has_value : public boost::static_visitor<bool>
  {
  public:
    bool operator()(ambiguous::note const&) const { return true; }
    bool operator()(ambiguous::rest const&) const { return true; }
    bool operator()(ambiguous::chord const&) const { return true; }
    bool operator()(ambiguous::value_distinction const&) const { return false; }
    bool operator()(ambiguous::simile const&) const { return false; }
  };
  class get_value : public boost::static_visitor<ambiguous::value>
  {
  public:
    result_type operator()(ambiguous::note const& note) const {
      return note.ambiguous_value;
    }
    result_type operator()(ambiguous::rest const& rest) const {
      return rest.ambiguous_value;
    }
    result_type operator()(ambiguous::chord const& chord) const {
      return chord.base.ambiguous_value;
    }
    result_type operator()(ambiguous::value_distinction const&) const {
      return ambiguous::unknown;
    }
    result_type operator()(ambiguous::simile const&) const {
      return ambiguous::unknown;
    }
  };
  class combinations
  : public value_type
  , public boost::static_visitor<void>
  {
    std::vector<value_type> values;
    std::vector<value_type>
    recurse( std::vector<value_type>::iterator const& begin
	   , std::vector<value_type>::iterator const& end
	   , const_reference stack
	   )
    {
      std::vector<value_type> result;
      if (begin == end) {
	result.push_back(stack);
      } else {
	BOOST_FOREACH(value_proxy const& v, *begin) {
	  value_type new_stack(stack);
	  new_stack.push_back(v);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack));
	}
      }
      return result;
    }
  public:
    combinations( ambiguous::partial_measure_voice::iterator const& begin
	        , ambiguous::partial_measure_voice::iterator const& end
	        )
    : std::vector< std::vector<value_proxy> >()
    {
      std::for_each(begin, end, boost::apply_visitor(*this));
      if (not values.empty()) {
	BOOST_FOREACH(value_proxy const& v, values.front()) {
	  boost::range::insert(*this, this->end(),
			       recurse(values.begin() + 1, values.end(),
				       value_type(1, v)));
	}
      }
      values.clear();
    }
    template<class Value>
    void operator()(Value& note) {
      value_type possibilities;
      possibilities.push_back(value_proxy(note, large));
      possibilities.push_back(value_proxy(note, small));
      values.push_back(possibilities);
    }
    void operator()(ambiguous::simile&) {}
    void operator()(ambiguous::value_distinction&) {}
  };
  class same_category
  : public value_type
  , public boost::static_visitor<void>
  {
    value_category category;
  public:
    same_category( ambiguous::partial_measure_voice::iterator const& begin
	         , ambiguous::partial_measure_voice::iterator const& end
	         , value_category const& category
	         )
    : std::vector< std::vector<value_proxy> >()
    , category(category)
    {
      std::for_each(begin, end, boost::apply_visitor(*this));
    }
    template<class Value>
    void operator()(Value& note) {
      value_type possibilities;
      possibilities.push_back(value_proxy(note, category));
      push_back(possibilities);
    }
    void operator()(ambiguous::simile&) {
      BOOST_ASSERT(false);
    }
    void operator()(ambiguous::value_distinction&) {
      BOOST_ASSERT(false);
    }
  };

  class notegroup : public combinations
  {
    value_type group;
    value_category category;
    ambiguous::value faked_type;
  public:
    notegroup( ambiguous::partial_measure_voice::iterator const& begin
	     , ambiguous::partial_measure_voice::iterator const& end
	     )
    : combinations(begin, end)
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
    void operator()(Value& note) {
      value_proxy proxy(note, category);
      proxy.set_type(faked_type);
      group.push_back(proxy);
    }
    void operator()(ambiguous::value_distinction&) {
      BOOST_ASSERT(false);
    }
    void operator()(ambiguous::simile&) {
      BOOST_ASSERT(false);
    }
  };

private:
  ambiguous::partial_measure_voice::iterator
  same_category_end( ambiguous::partial_measure_voice::iterator& begin
	           , ambiguous::partial_measure_voice::iterator const& end
		   , ambiguous::value_distinction const& distinction
                   ) const
  {
    if (boost::apply_visitor(distinction_sign(distinction), *begin)) {
      begin = begin + 1;
      ambiguous::partial_measure_voice::iterator iter(begin);
      if (iter != end &&
	  boost::apply_visitor(has_value(), *iter)) {
	ambiguous::value initial(boost::apply_visitor(get_value(), *iter++));
	while (iter != end && apply_visitor(get_value(), *iter) == initial) {
	  ++iter;
	}
	return iter;
      }
    }
    return begin;
  }
  ambiguous::partial_measure_voice::iterator
  notegroup_end( ambiguous::partial_measure_voice::iterator const& begin
	       , ambiguous::partial_measure_voice::iterator const& end
               ) const
  {
    if (boost::apply_visitor(has_value(), *begin)) {
      if (boost::apply_visitor(get_value(), *begin) != ambiguous::eighth_or_128th) {
	ambiguous::partial_measure_voice::iterator iter = begin + 1;
	while (iter != end &&
	       boost::apply_visitor(has_value(), *iter) &&
	       boost::apply_visitor(get_value(), *iter) == ambiguous::eighth_or_128th) {
	  ++iter;
	}
	if (std::distance(begin, iter) > 2) return iter;
      }
    }
    return begin;
  }

public:
  value_proxy_list(ambiguous::partial_measure_voice& voice)
  : std::vector<value_type>()
  {
    ambiguous::partial_measure_voice::iterator begin = voice.begin();
    while (begin != voice.end()) {
      ambiguous::partial_measure_voice::iterator end;
      if ((end = notegroup_end(begin, voice.end())) != begin) {
	push_back(notegroup(begin, end));
      } else if ((end = same_category_end(begin, voice.end(),
					  ambiguous::large_follows)) != begin) {
	push_back(same_category(begin, end, large));
      } else if ((end = same_category_end(begin, voice.end(),
					  ambiguous::small_follows)) != begin) {
	push_back(same_category(begin, end, small));
      } else {
	if (begin != voice.end()) {
	  end = begin + 1;
	  push_back(combinations(begin, end));
	}
      }
      std::advance(begin, std::distance(begin, end));
    }
  }
};

rational
duration(value_proxy const& proxy)
{
  return proxy.as_rational();
}

typedef std::vector<value_proxy> proxied_partial_voice;

rational
operator+(rational const& r, proxied_partial_voice::const_reference v)
{
  return r + duration(v);
}

rational
duration(proxied_partial_voice const& values)
{
  return boost::accumulate(values, rational(0));
}

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
      BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
		    *begin) {
	if (duration(possibility) <= max_length) {
	  value_type new_stack(stack);
	  boost::range::insert(new_stack, new_stack.end(), possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack,
				       max_length - duration(possibility)));
	}
      }
    }
    return result;
  }  
public:
  partial_voice_interpretations( ambiguous::partial_measure_voice& voice
			       , rational const& max_length
			       )
  : std::vector<proxied_partial_voice>()
  {
    value_proxy_list vpl(voice);
    if (not vpl.empty()) {
      BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
		    vpl.front()) {
	if (duration(possibility) <= max_length) {
	  boost::range::insert(*this, end(),
			       recurse(vpl.begin() + 1, vpl.end(),
				       possibility,
				       max_length - duration(possibility)));
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
      BOOST_FOREACH(partial_voice_interpretations::const_reference possibility,
		    partial_voice_interpretations(*begin, length))
        if (duration(possibility) == length) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack, length));
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
duration( proxied_partial_measure const& voices )
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
    BOOST_ASSERT(max_length >= 0);
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(partial_measure_interpretations::const_reference possibility,
		    partial_measure_interpretations(*begin, max_length)) {
        value_type new_stack(stack);
        new_stack.push_back(possibility);
	boost::range::insert(result, result.end(),
			     recurse(begin + 1, end, new_stack,
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
{
  return r + duration(p);
}

rational
duration( proxied_voice const& parts )
{
  return boost::accumulate(parts, rational(0));
}

typedef std::vector<proxied_voice> proxied_measure;

rational
duration( proxied_measure const& voices )
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
  rational max_duration;
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
      BOOST_FOREACH(voice_interpretations::const_reference possibility,
		    voice_interpretations(*begin, max_duration)) {
	if (real_length == duration(possibility)) {
	  value_type new_stack(stack);
	  new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack, real_length));
	}
      }
    }
    return result;
  }
public:
  measure_interpretations(ambiguous::measure& measure, rational max_duration)
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

    if (size() > 1)
      for (iterator measure = begin(); measure != end();
           measure = duration(*measure) != max_duration? erase(measure): measure + 1);
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

compiler::result_type
compiler::operator()(ambiguous::measure& measure) const
{
  measure_interpretations interpretations(measure, global_time_signature);

  if (interpretations.size() != 1) {
    if (interpretations.empty()) {
      report_error(measure.id, "No possible interpretations");
    } else {
      report_error(measure.id, "Several possible interpretations");
    }
    return false;
  }

  accept(interpretations.front());
  return true;
}

}}

