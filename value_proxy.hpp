#include <numeric>
#include <boost/foreach.hpp>
#include <boost/range/algorithm_ext/insert.hpp>
#include "ambiguous.hpp"

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

  rational as_rational() const {
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
};

class value_proxy_list
: public std::vector< std::vector<value_proxy> >
, public boost::static_visitor<void>
{
  enum choices { just_large, just_small, both };
  choices choice;
  ambiguous::value last_value;
public:
  value_proxy_list(ambiguous::partial_measure_voice& voice)
  : std::vector< std::vector<value_proxy> >()
  , choice(both), last_value(ambiguous::unknown)
  {
    std::for_each(voice.begin(), voice.end(), boost::apply_visitor(*this));
  }
  template<class Note>
  void operator()(Note& note) {
    value_type possibilities;
    if (choice == just_large) {
      if (last_value == ambiguous::unknown) last_value = value(note);
      if (last_value == value(note)) {
        possibilities.push_back(value_proxy(note, large));
      } else {
        last_value = ambiguous::unknown;
        choice = both;
      }
    } else if (choice == just_small) {
      if (last_value == ambiguous::unknown) last_value = value(note);
      if (last_value == value(note)) {
        possibilities.push_back(value_proxy(note, small));
      } else {
        last_value = ambiguous::unknown;
        choice = both;
      }
    }
    if (choice == both) {
      possibilities.push_back(value_proxy(note, large));
      possibilities.push_back(value_proxy(note, small));
    }
    BOOST_ASSERT(possibilities.size() <= 2);
    push_back(possibilities);
  }
  void operator()(ambiguous::value_distinction value_distinction) {
    switch (value_distinction) {
    case ambiguous::large_follows:
      choice = just_large;
      break;
    case ambiguous::small_follows:
      choice = just_small;
      break;
    default: BOOST_ASSERT(false);
    }
  }
  void operator()(ambiguous::simile&) {
  }
private:
  template<class NoteOrRest>
  static ambiguous::value value(NoteOrRest& note) {
    return note.ambiguous_value;
  }
  static ambiguous::value value(ambiguous::chord& chord) {
    return value(chord.base);
  }
};

rational
duration( value_proxy const& proxy )
{
  return proxy.as_rational();
}

class proxied_partial_measure_voice
: public std::vector< std::vector<value_proxy> >
{
  static
  std::vector<value_type>
  recurse( value_proxy_list::iterator const& begin
         , value_proxy_list::iterator const& end
         , const_reference stack
         , rational const& max_length
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
	  new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack,
				       max_length - duration(possibility)));
	}
      }
    }
    return result;
  }  
public:
  proxied_partial_measure_voice( ambiguous::partial_measure_voice& voice
			       , rational const& max_length
			       )
  : std::vector< std::vector<value_proxy> >()
  {
    value_proxy_list vpl(voice);
    if (not vpl.empty()) {
      BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
		    vpl.front()) {
	if (duration(possibility) <= max_length) {
	  value_type stack;
	  stack.push_back(possibility);
	  boost::range::insert(*this, end(),
			       recurse(vpl.begin() + 1, vpl.end(), stack,
				       max_length - duration(possibility)));
	}
      }
    }
  }
};

rational
operator+(rational const& r, value_proxy const& v)
{
  return r + duration(v);
}

rational
duration( std::vector<value_proxy> const& proxies )
{
  return std::accumulate(proxies.begin(), proxies.end(), rational(0));
}

typedef std::vector< std::vector<value_proxy> > proxied_partial_measure;

class partial_measure_interpretations
: public std::vector< proxied_partial_measure >
{
  static
  std::vector<value_type>
  recurse( ambiguous::partial_measure::iterator const& begin
         , ambiguous::partial_measure::iterator const& end
         , const_reference stack
         , rational const& length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(proxied_partial_measure_voice::const_reference possibility,
		    proxied_partial_measure_voice(*begin, length)) {
        if (duration(possibility) == length) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack, length));
        }
      }
    }
    return result;
  }
public:
  partial_measure_interpretations( ambiguous::partial_measure& partial_measure
                                 , rational const& max_length
                                 )
  : std::vector< proxied_partial_measure >()
  {
    if (not partial_measure.empty()) {
      BOOST_FOREACH(proxied_partial_measure_voice::const_reference possibility,
		    proxied_partial_measure_voice(partial_measure.front(),
						  max_length)) {
        value_type stack;
        stack.push_back(possibility);
	boost::range::insert(*this, end(),
			     recurse(partial_measure.begin() + 1,
				     partial_measure.end(),
				     value_type(1, possibility),
				     duration(possibility)));
      }
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

typedef std::vector< proxied_partial_measure > proxied_voice;

class voice_interpretations
: public std::vector< proxied_voice >
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
  : std::vector< proxied_voice >()
  {
    if (not voice.empty()) {
      BOOST_FOREACH(partial_measure_interpretations::const_reference possibility,
		    partial_measure_interpretations(voice.front(), max_length)) {
        value_type stack;
        stack.push_back(possibility);
	boost::range::insert(*this, end(),
			     recurse(voice.begin() + 1, voice.end(), stack,
				     max_length - duration(possibility)));
      }
    }
  }
};

rational
operator + (rational const& r, proxied_partial_measure const& p)
{
  return r + duration(p);
}

rational
duration( proxied_voice const& parts )
{
  return std::accumulate(parts.begin(), parts.end(), rational(0));
}

typedef std::vector< proxied_voice > proxied_measure;

rational
duration( proxied_measure const& voices )
{
  rational value;
  if (not voices.empty()) value = duration(voices.front());
  return value;
}

class measure_interpretations
: public std::vector< proxied_measure >
{
  static std::vector<value_type>
  recurse( ambiguous::measure::iterator const& begin
         , ambiguous::measure::iterator const& end
         , const_reference stack
         , rational const& max_length, rational const& real_length
         )
  {
    std::vector<value_type> result;
    if (begin == end) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(voice_interpretations::const_reference possibility,
		    voice_interpretations(*begin, max_length)) {
	if (real_length == duration(possibility)) {
	  value_type new_stack(stack);
	  new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(begin + 1, end, new_stack,
				       max_length, real_length));
	}
      }
    }
    return result;
  }
public:
  measure_interpretations(ambiguous::measure& measure, rational max_duration)
  : std::vector<proxied_measure>()
  {
    if (not measure.empty()) {
      BOOST_ASSERT(max_duration >= 0);
      BOOST_FOREACH(voice_interpretations::const_reference possibility,
		    voice_interpretations(measure.front(), max_duration)) {
	value_type stack;
        stack.push_back(possibility);
	boost::range::insert(*this, end(),
			     recurse(measure.begin() + 1, measure.end(),
                                     stack,
                                     max_duration, duration(possibility)));
      }
    }

    if (size() > 1) {
      iterator interpretation = begin();
      while (interpretation != end())
	interpretation = duration(*interpretation) == max_duration?
                         interpretation + 1: erase(interpretation);
    }
  }
};

}}
