#include <boost/foreach.hpp>
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

class proxied_partial_measure_voice
: public std::vector< std::vector<value_proxy> >
{
  static
  std::vector<value_type>
  recurse( value_proxy_list::iterator const& first
         , value_proxy_list::iterator const& last
         , const_reference stack
         , rational const& max_length
         )
  {
    std::vector<value_type> result;
    if (first == last) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(value_proxy_list::value_type::const_reference possibility,
		    *first) {
	if (possibility.as_rational() <= max_length) {
	  value_type new_stack(stack);
	  new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(first + 1, last, new_stack,
				       max_length - possibility.as_rational()));
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
	if (possibility.as_rational() <= max_length) {
	  value_type stack;
	  stack.push_back(possibility);
	  boost::range::insert(*this, end(),
			       recurse(vpl.begin() + 1, vpl.end(), stack,
				       max_length - possibility.as_rational()));
	}
      }
    }
  }
};

rational
duration( std::vector<value_proxy> const& proxies )
{
  rational value;
  for (std::vector<value_proxy>::const_iterator
       iter = proxies.begin(); iter != proxies.end(); ++iter)
  {
    value += iter->as_rational();
  }
  return value;
}

class proxied_partial_measure
: public std::vector< std::vector< std::vector<value_proxy> > >
{
  static
  std::vector< value_type >
  recurse( ambiguous::partial_measure::iterator const& first
         , ambiguous::partial_measure::iterator const& last
         , const_reference stack
         , rational const& length
         )
  {
    std::vector< value_type > result;
    if (first == last) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(proxied_partial_measure_voice::const_reference possibility,
		    proxied_partial_measure_voice(*first, length))
      {
        if (duration(possibility) == length) {
          value_type new_stack(stack);
          new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(first + 1, last, new_stack, length));
        }
      }
    }
    return result;
  }
public:
  proxied_partial_measure( ambiguous::partial_measure& partial_measure
                         , rational const& max_length
                         )
  : std::vector< std::vector< std::vector<value_proxy> > >()
  {
    if (not partial_measure.empty()) {
      BOOST_FOREACH(proxied_partial_measure_voice::const_reference possibility,
		    proxied_partial_measure_voice(partial_measure.front(),
						  max_length))
      {
        value_type stack;
        stack.push_back(possibility);
	boost::range::insert(*this, end(),
			     recurse(partial_measure.begin() + 1,
				     partial_measure.end(), stack,
				     duration(possibility)));
      }
    }
  }
};

rational
duration( std::vector< std::vector<value_proxy> > const& proxies )
{
  rational value;
  for (std::vector<value_proxy>::const_iterator
       iter = proxies.begin()->begin(); iter != proxies.begin()->end(); ++iter)
  {
    value += iter->as_rational();
  }
  return value;
}

class proxied_voice
: public std::vector< std::vector< std::vector< std::vector<value_proxy> > > >
{
  static
  std::vector< value_type >
  recurse( ambiguous::voice::iterator const& first
         , ambiguous::voice::iterator const& last
         , const_reference stack
         , rational const& max_length
         )
  {
    BOOST_ASSERT(max_length >= 0);
    std::vector< value_type > result;
    if (first == last) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(proxied_partial_measure::const_reference possibility,
		    proxied_partial_measure(*first, max_length))
      {
        value_type new_stack(stack);
        new_stack.push_back(possibility);
	boost::range::insert(result, result.end(),
			     recurse(first + 1, last, new_stack,
				     max_length - duration(possibility)));
      }
    }
    return result;
  }
public:
  proxied_voice(ambiguous::voice& voice, rational max_length)
  : std::vector< std::vector< std::vector< std::vector<value_proxy> > > >()
  {
    if (not voice.empty()) {
      BOOST_FOREACH(proxied_partial_measure::const_reference possibility,
		    proxied_partial_measure(voice.front(), max_length))
      {
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
duration( std::vector< std::vector< std::vector<value_proxy> > > const& proxies )
{
  rational value;
  for (std::vector< std::vector< std::vector<value_proxy> > >::const_iterator
       iter = proxies.begin(); iter != proxies.end(); ++iter)
  {
    value += duration(*iter);
  }
  return value;
}

rational
duration( std::vector< std::vector< std::vector< std::vector<value_proxy> > > > const& proxies )
{
  rational value;
  if (not proxies.empty()) value = duration(proxies.front());
  return value;
}

class proxied_measure
: public std::vector< std::vector< std::vector< std::vector< std::vector<value_proxy> > > > >
{
  static std::vector<value_type>
  recurse( ambiguous::measure::iterator const& first
         , ambiguous::measure::iterator const& last
         , const_reference stack
         , rational const& max_length
         , rational const& real_length
         )
  {
    std::vector< value_type > result;
    if (first == last) {
      result.push_back(stack);
    } else {
      BOOST_FOREACH(proxied_voice::const_reference possibility,
		    proxied_voice(*first, max_length))
      {
	if (real_length == duration(possibility)) {
	  value_type new_stack(stack);
	  new_stack.push_back(possibility);
	  boost::range::insert(result, result.end(),
			       recurse(first + 1, last, new_stack,
				       max_length, real_length));
	}
      }
    }
    return result;
  }
public:
  proxied_measure(ambiguous::measure& measure, rational max_duration)
  : std::vector< std::vector< std::vector< std::vector< std::vector<value_proxy> > > > >()
  {
    if (not measure.empty()) {
      BOOST_ASSERT(max_duration >= 0);
      BOOST_FOREACH(proxied_voice::const_reference possibility,
		    proxied_voice(measure.front(), max_duration))
      {
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
