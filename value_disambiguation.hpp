// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_VALUE_DISAMBIGUATION_HPP
#define BMC_VALUE_DISAMBIGUATION_HPP

#include "ambiguous.hpp"
#include <cmath>
#include <boost/foreach.hpp>
#include <boost/range/numeric.hpp>
#include <memory>
#include <sstream>

namespace music { namespace braille {

/**
 * \brief Utilities for disambiguating note and rest values.
 *
 * \see value_disambiguator
 */
namespace value_disambiguation {

/** \brief Braille music has two fundamental categories of note and rest values.
 *
 * <code>large</code> spans from whole to eighth note values.
 * <code>small</code> from 16th to 128th note values.
 */
enum value_category
{
  large, small
};

rational const undotted[8] = {
  rational(1, 1), rational(1, 2), rational(1, 4), rational(1, 8),
  rational(1, 16), rational(1, 32), rational(1, 64), rational(1, 128)
};

/**
 * \brief A possible interpretation of a value (note, rest or chord).
 *
 * This class saves a pointer to the original sign it was used to create such
 * that it can later fill in the actual value type it stands for.
 */
class value_proxy
{
  ambiguous::value value_type:4;
  value_category category:4;

  rational const& undotted_duration() const
  {
    BOOST_ASSERT(category==large || category==small);
    BOOST_ASSERT(value_type >= 0 and value_type < 4);
    return undotted[category * 4 + value_type];
  }

  unsigned dots;

  rational duration;
  rational calculate_duration()
  {
    rational const base(undotted_duration());
    return dots? base * 2 - base / pow(2, dots): base;
  }

  rational *final_type;
  bool *whole_measure_rest;

public:
  value_proxy() : final_type(nullptr), whole_measure_rest(nullptr) {}
  value_proxy( ambiguous::note& note
             , value_category const& category
             )
  : value_type(note.ambiguous_value), category(category), dots(note.dots)
  , duration(calculate_duration())
  , final_type(&note.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::note& note
             , value_category const& category
             , ambiguous::value value_type
             )
  : value_type(value_type), category(category), dots(note.dots)
  , duration(calculate_duration())
  , final_type(&note.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::rest& rest
             , value_category const& category
             )
  : value_type(rest.ambiguous_value), category(category), dots(rest.dots)
  , duration(calculate_duration())
  , final_type(&rest.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::rest& rest
             , value_category const& category
             , ambiguous::value value_type
             )
  : value_type(value_type), category(category), dots(rest.dots)
  , duration(calculate_duration())
  , final_type(&rest.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::rest& rest
             , value_category const& category
             , rational const& duration
             )
  : value_type(rest.ambiguous_value), category(category), dots(rest.dots)
  , duration(duration)
  , final_type(&rest.type)
  , whole_measure_rest(&rest.whole_measure)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::chord& chord
             , value_category const& category
             )
  : value_type(chord.base.ambiguous_value), category(category)
  , dots(chord.base.dots)
  , duration(calculate_duration())
  , final_type(&chord.base.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ambiguous::chord& chord
             , value_category const& category
             , ambiguous::value value_type
             )
  : value_type(value_type), category(category), dots(chord.base.dots)
  , duration(calculate_duration())
  , final_type(&chord.base.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  operator rational const&() const { return duration; }

  bool operator==(value_proxy const& rhs) const
  { return final_type == rhs.final_type && duration == rhs.duration; }

  void accept() const
  {
    if (whole_measure_rest) {
      *final_type = duration;
      *whole_measure_rest = true;
    } else {
      *final_type = undotted_duration();
    }
  }
};

class proxied_partial_voice : public std::vector<value_proxy>
{
  rational const duration_;
public:
  proxied_partial_voice( value_proxy const *begin, value_proxy const *end
                       , rational const& duration
                       )
  : std::vector<value_proxy>(begin, end)
  , duration_(duration)
  {}
  rational const& duration() const
  { return duration_; }
};

inline rational
duration(std::vector<value_proxy> const& values)
{ return boost::accumulate(values, zero); }

typedef std::shared_ptr<proxied_partial_voice const> proxied_partial_voice_ptr;

class partial_voice_interpretations
: public std::vector<proxied_partial_voice_ptr>
{
  struct maybe_whole_measure_rest : boost::static_visitor<bool>
  {
    result_type operator()(ambiguous::rest const& rest) const
    {
      return rest.ambiguous_value == ambiguous::whole_or_16th and
             rest.dots == 0;
    }
    template<typename Sign>
    result_type operator()(Sign const&) const { return false; }
  };
  class make_whole_measure_rest : public boost::static_visitor<value_proxy>
  {
    rational const duration;
  public:
    make_whole_measure_rest(rational const& duration) : duration(duration) {}
    result_type operator()(ambiguous::rest& rest) const
    { return result_type(rest, large, duration); }
    template<typename Sign>
    result_type operator()(Sign const&) const
    { BOOST_ASSERT(false); return value_proxy(); }
  };

  class is_value_distinction : public boost::static_visitor<bool>
  {
    ambiguous::value_distinction expected;
  public:
    is_value_distinction(ambiguous::value_distinction const& distinction)
    : expected(distinction) {}

    result_type operator()(ambiguous::value_distinction const& distinction) const
    { return distinction == expected; }

    template <class Sign>
    result_type operator()(Sign const&) const
    { return false; }
  };
  static
  ambiguous::partial_voice::iterator
  same_category_end( ambiguous::partial_voice::iterator& begin
                   , ambiguous::partial_voice::iterator const& end
                   , ambiguous::value_distinction const& distinction
                   )
  {
    if (boost::apply_visitor(is_value_distinction(distinction), *begin)) {
      begin = begin + 1; // Advance the outer iterator to avoid a loop
      ambiguous::partial_voice::iterator iter(begin);
      if (iter != end and
          boost::apply_visitor(ambiguous::is_rhythmic(), *iter)) {
        for (ambiguous::value
             initial = boost::apply_visitor(ambiguous::get_ambiguous_value(), *iter++);
             iter != end and
             apply_visitor(ambiguous::get_ambiguous_value(), *iter) == initial;
             ++iter);
        return iter;
      }
    }
    return begin;
  }
  /**
   * \brief Find end of note group.
   *
   * \param begin The beginning of the range of signs to examine
   * \param end   The end of the range of signs to examine
   * \return If note group was found the returned iterator points one element
   *         beyond its last element.  Otherwise, begin is returned.
   */
  static
  ambiguous::partial_voice::iterator
  notegroup_end( ambiguous::partial_voice::iterator const& begin
               , ambiguous::partial_voice::iterator const& end
               )
  {
    if (boost::apply_visitor(ambiguous::is_rhythmic(), *begin)) {
      if (boost::apply_visitor(ambiguous::get_ambiguous_value(), *begin) != ambiguous::eighth_or_128th) {
        auto iter = begin + 1;
        while (iter != end and
               boost::apply_visitor(ambiguous::get_ambiguous_value(), *iter) == ambiguous::eighth_or_128th)
          ++iter;
	// A note group is only valid if it consists of at least 3 rhythmic signs
        if (std::distance(begin, iter) > 2) return iter;
      }
    }
    return begin;
  }

  struct large_and_small : std::vector<value_proxy>, boost::static_visitor<void>
  {
    large_and_small(ambiguous::sign& sign)
    { boost::apply_visitor(*this, sign); }

    template <class Value>
    result_type operator()(Value& value)
    {
      if (not is_grace(value)) {
        emplace_back(value, large);
        emplace_back(value, small);
      }
    }
    result_type operator()(ambiguous::value_distinction&) {}
    result_type operator()(braille::hand_sign&) {}
    result_type operator()(ambiguous::barline&) {}
    result_type operator()(ambiguous::simile&) {}
    bool is_grace(ambiguous::note const& note) const
    {
      return std::find(note.articulations.begin(), note.articulations.end(),
                       appoggiatura)
             != note.articulations.end()
             ||
             std::find(note.articulations.begin(), note.articulations.end(),
                      short_appoggiatura)
             != note.articulations.end();
    }
    bool is_grace(ambiguous::rest const&) const
    { return false; }
    bool is_grace(ambiguous::chord const& chord) const
    { return is_grace(chord.base); }
  };
  class notegroup
  : public std::vector<value_proxy>
  , public boost::static_visitor<void>
  {
    value_category const category;
    ambiguous::value type;
  public:
    notegroup( ambiguous::partial_voice::iterator const& begin
             , ambiguous::partial_voice::iterator const& end
             , value_category const& category
             )
    : category(category), type(ambiguous::unknown)
    { std::for_each(begin, end, boost::apply_visitor(*this)); }

    result_type operator()(ambiguous::note& note)
    {
      if (type == ambiguous::unknown) type = note.ambiguous_value;
      emplace_back(note, category, type);
    }
    result_type operator()(ambiguous::rest& rest)
    {
      if (type == ambiguous::unknown) type = rest.ambiguous_value;
      emplace_back(rest, category, type);
    }
    result_type operator()(ambiguous::chord& chord)
    {
      if (type == ambiguous::unknown) type = chord.base.ambiguous_value;
      emplace_back(chord, category, type);
    }
    result_type operator()(ambiguous::value_distinction&) {}
    result_type operator()(braille::hand_sign&) {}
    result_type operator()(ambiguous::barline&) {}
    result_type operator()(ambiguous::simile&) {}
  };
  class same_category
  : public std::vector<value_proxy>
  , public boost::static_visitor<void>
  {
    value_category const category;
  public:
    same_category( ambiguous::partial_voice::iterator const& begin
                 , ambiguous::partial_voice::iterator const& end
                 , value_category const& category
                 )
    : category(category)
    { std::for_each(begin, end, boost::apply_visitor(*this)); }

    result_type operator()(ambiguous::note& note)
    { emplace_back(note, category); }
    result_type operator()(ambiguous::rest& rest)
    { emplace_back(rest, category); }
    result_type operator()(ambiguous::chord& chord)
    { emplace_back(chord, category); }

    template<typename Sign> result_type operator()(Sign const&)
    { BOOST_ASSERT(false); }
  };

  music::time_signature const& time_signature;
  rational const start_position;
  rational const beat;
  bool on_beat(rational const& position) const {
    return position % beat == zero;
  }

  void recurse( ambiguous::partial_voice::iterator begin
              , ambiguous::partial_voice::iterator const& end
              , value_proxy stack_begin[]
              , value_proxy *stack_end
              , rational const& max_duration
              , rational const& position
              )
  {
    if (begin == end) {
      emplace_back(std::make_shared<proxied_partial_voice>
                   (stack_begin, stack_end, position - start_position)
                  );
    } else {
      ambiguous::partial_voice::iterator tail;
      if (on_beat(position) and (tail = notegroup_end(begin, end)) > begin) {
        while (std::distance(begin, tail) >= 3) {
          notegroup const group(begin, tail, small);
          rational const group_duration(duration(group));
          if (group_duration <= max_duration and
              on_beat(position + group_duration)) {
            recurse(tail, end,
                    stack_begin, std::copy(group.begin(), group.end(),
                                           stack_end),
                    max_duration - group_duration,
                    position + group_duration);
          }
          --tail;
        }
        large_and_small const possibilities(*begin);
        tail = begin; ++tail;
        if (possibilities.empty()) {
          recurse(tail, end, stack_begin, stack_end, max_duration, position);
        } else {
          for(large_and_small::const_reference value : possibilities) {
            if (value <= max_duration) {
              *stack_end = value;
              recurse(tail, end, stack_begin, stack_end + 1,
                      max_duration - value, position + value);
            }
          }
        }
      } else if ((tail = same_category_end(begin, end,
                                           ambiguous::large_follows)) > begin) {
        same_category const group(begin, tail, large);
        if (duration(group) <= max_duration) {
          recurse(tail, end,
                  stack_begin, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group));
        }
      } else if ((tail = same_category_end(begin, end,
                                           ambiguous::small_follows)) > begin) {
        same_category const group(begin, tail, small);
        if (duration(group) <= max_duration) {
          recurse(tail, end,
                  stack_begin, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group));
        }
      } else {
        large_and_small const possibilities(*begin);
        tail = begin; ++tail;
        if (possibilities.empty()) {
          recurse(tail, end, stack_begin, stack_end, max_duration, position);
        } else {
          for(large_and_small::const_reference value: possibilities) {
            if (value <= max_duration) {
              *stack_end = value;
              recurse(tail, end, stack_begin, stack_end + 1,
                      max_duration - value, position + value);
            }
          }
        }

        if (stack_begin == stack_end and position == 0 and time_signature != 1 and
            boost::apply_visitor(maybe_whole_measure_rest(), *begin)) {
          *stack_end = boost::apply_visitor(make_whole_measure_rest(time_signature), *begin);
          recurse(tail, end, stack_begin, stack_end + 1,
                  zero, position + time_signature);
        }
      }
    }
  }

public:
  partial_voice_interpretations( ambiguous::partial_voice& voice
                               , rational const& max_duration
                               , rational const& position
                               , music::time_signature const& time_sig
                               )
  : time_signature(time_sig)
  , start_position(position)
  , beat(1, time_signature.denominator())
  {
    value_proxy *stack = new value_proxy[voice.size()];
    recurse(voice.begin(), voice.end(),
            stack, stack,
            max_duration, position);
    delete [] stack;
  }
};

inline rational const&
duration(proxied_partial_voice_ptr const& partial_voice)
{ return (*partial_voice).duration(); }

typedef std::vector<proxied_partial_voice_ptr> proxied_partial_measure;

typedef std::shared_ptr<proxied_partial_measure> proxied_partial_measure_ptr;

class partial_measure_interpretations
: public std::vector<proxied_partial_measure_ptr>
{
  void recurse( ambiguous::partial_measure::iterator const& begin
              , ambiguous::partial_measure::iterator const& end
              , proxied_partial_voice_ptr stack_begin[]
              , proxied_partial_voice_ptr *stack_end
              , rational const& length
              , rational const& position
              , music::time_signature const& time_sig
              )
  {
    if (begin == end) {
      if (stack_begin != stack_end)
        emplace_back(std::make_shared<proxied_partial_measure>(stack_begin, stack_end));
    } else {
      ambiguous::partial_measure::iterator const tail = begin + 1;
      for(partial_voice_interpretations::const_reference possibility:
          partial_voice_interpretations(*begin, length, position, time_sig)) {
        rational const partial_voice_duration(duration(possibility));
        if (stack_begin == stack_end or partial_voice_duration == length) {
          *stack_end = possibility;
          recurse(tail, end, stack_begin, stack_end + 1,
                  partial_voice_duration, position, time_sig);
        }
      }
    }
  }
public:
  partial_measure_interpretations( ambiguous::partial_measure& partial_measure
                                 , rational const& max_length
                                 , rational const& position
                                 , music::time_signature const& time_sig
                                 )
  {
    proxied_partial_voice_ptr *stack = new proxied_partial_voice_ptr[partial_measure.size()];
    recurse(partial_measure.begin(), partial_measure.end(),
            stack, stack,
            max_length, position, time_sig);
    delete [] stack;
  }
};

inline rational
duration(proxied_partial_measure const& voices)
{
  rational value;
  if (not voices.empty()) {
    value = duration(voices.front());
#if !defined(NDEBUG)
    for (proxied_partial_measure::const_iterator
         voice = voices.begin() + 1; voice != voices.end(); ++voice) {
      BOOST_ASSERT(value == duration(*voice));
    }
#endif
  }
  return value;
}

inline rational
duration(proxied_partial_measure_ptr const& voices)
{ return duration(*voices); }

class proxied_voice : public std::vector<proxied_partial_measure_ptr>
{
  rational const duration_;
public:
  proxied_voice( proxied_partial_measure_ptr const *begin
               , proxied_partial_measure_ptr const *end
               )
  : std::vector<proxied_partial_measure_ptr>(begin, end)
  , duration_(std::accumulate(begin, end, zero))
  {}
  rational const& duration() const
  { return duration_; }
};

inline rational const&
duration(proxied_voice const& parts)
{ return parts.duration(); }

inline rational
operator+(rational const& r, proxied_partial_measure_ptr const& part)
{ return r + duration(part); }

inline rational
duration( proxied_partial_measure_ptr *const begin
        , proxied_partial_measure_ptr *const end
        )
{ return std::accumulate(begin, end, zero); }

typedef std::shared_ptr<proxied_voice const> proxied_voice_ptr;

class voice_interpretations : public std::vector<proxied_voice_ptr>
{
  void recurse( ambiguous::voice::iterator const& begin
              , ambiguous::voice::iterator const& end
              , proxied_partial_measure_ptr stack_begin[]
              , proxied_partial_measure_ptr *stack_end
              , rational const& max_length
              , music::time_signature const& time_sig
              , bool complete
              )
  {
    if (begin == end) {
      if (stack_begin != stack_end) {
        if (not complete or duration(stack_begin, stack_end) == time_sig)
          emplace_back(std::make_shared<proxied_voice>(stack_begin, stack_end));
      }
    } else {
      ambiguous::voice::iterator const tail = begin + 1;
      for(partial_measure_interpretations::const_reference possibility:
          partial_measure_interpretations(*begin, max_length,
                                          duration(stack_begin, stack_end),
                                          time_sig)) {
        *stack_end = possibility;
        recurse(tail, end, stack_begin, stack_end + 1,
                max_length - duration(possibility), time_sig, complete);
      }
    }
  }

public:
  voice_interpretations( ambiguous::voice& voice
                       , rational const& max_length
                       , music::time_signature const& time_sig
                       , bool complete
                       )
  {
    proxied_partial_measure_ptr *stack = new proxied_partial_measure_ptr[voice.size()];
    recurse(voice.begin(), voice.end(),
            stack, stack,
            max_length, time_sig, complete);
    delete [] stack;
  }
};

inline rational
duration(proxied_voice_ptr const& parts)
{ return duration(*parts); }

class proxied_measure : public std::vector<proxied_voice_ptr>
{
  rational mean;
public:
  proxied_measure( proxied_voice_ptr const *begin
                 , proxied_voice_ptr const *end
                 )
  : std::vector<proxied_voice_ptr>(begin, end)
  , mean()
  {
  }

  rational const& harmonic_mean()
  {
    if (mean == zero) {
      rational sum;
      for (const_reference voice: *this)
        for (proxied_voice::const_reference part: *voice)
          for (proxied_partial_measure::const_reference partial_voice: *part)
            for (proxied_partial_voice::const_reference value: *partial_voice) {
              sum += reciprocal(value), ++mean;
            }
      mean /= sum;
    }
    return mean;
  }

  void
  accept()
  {
    for (const_reference voice: *this)
      for (proxied_voice::const_reference partial_measure: *voice)
        for (proxied_partial_measure::const_reference partial_voice: *partial_measure)
          std::for_each(partial_voice->begin(), partial_voice->end(),
                        std::mem_fun_ref(&value_proxy::accept));
  }
};

inline rational
duration(proxied_measure const& voices)
{
  rational value;
  if (not voices.empty()) {
    value = duration(voices.front());
#if !defined(NDEBUG)
    for (proxied_measure::const_iterator
         voice = voices.begin() + 1; voice != voices.end(); ++voice) {
      BOOST_ASSERT(value == duration(*voice));
    }
#endif
  }
  return value;
}

template <typename Char>
std::basic_ostream<Char>&
operator<<(std::basic_ostream<Char>& os, proxied_measure const& measure)
{
  BOOST_FOREACH(proxied_measure::const_reference voice, measure) {
    os << '[';
    BOOST_FOREACH(proxied_voice::const_reference part, *voice) {
      os << '{';
      BOOST_FOREACH(proxied_partial_measure::const_reference partial_voice, *part) {
        os << '(';
        BOOST_FOREACH(proxied_partial_voice::const_reference value, *partial_voice) {
          os << '<' << rational(value).numerator() << '/' << rational(value).denominator() << '>';
        }
        os << ')';
      }
      os << '}';
    }
    os << ']';
  }
  return os;
}

class measure_interpretations : public std::list<proxied_measure>
{
  music::time_signature time_signature;
  bool complete;

  void recurse( std::vector<ambiguous::voice>::iterator const& begin
              , std::vector<ambiguous::voice>::iterator const& end
              , proxied_voice_ptr stack_begin[]
              , proxied_voice_ptr *stack_end
              , rational const& length
              )
  {
    if (begin == end) {
      if (stack_begin != stack_end) {
        if (length == time_signature or not complete) {
          if (not complete and length == time_signature) {
            erase_incomplete_interpretations();
            complete = true;
          }
          emplace_back(stack_begin, stack_end);
        }
      }
    } else {
      std::vector<ambiguous::voice>::iterator const tail = begin + 1;
      for (voice_interpretations::const_reference possibility:
          voice_interpretations(*begin, length, time_signature, complete)) {
        rational const voice_duration(duration(possibility));
        if ((stack_begin == stack_end and not complete) or voice_duration == length) {
          *stack_end = possibility;
          recurse(tail, end, stack_begin, stack_end + 1, voice_duration);
        }
      }
    }
  }

  void
  erase_incomplete_interpretations()
  {
    for (iterator interpretation = begin(); interpretation != end();
         interpretation = duration(*interpretation) != time_signature?
                          erase(interpretation): ++interpretation);
  }

public:
  measure_interpretations()
  : complete(false)
  {}

  measure_interpretations(measure_interpretations const& other)
  : std::list<proxied_measure>(other.begin(), other.end())
  , time_signature(other.time_signature)
  , complete(other.complete)
  {}

  measure_interpretations( ambiguous::measure& measure
                         , music::time_signature const& time_signature
                         )
  : time_signature(time_signature)
  , complete(false)
  {
    BOOST_ASSERT(time_signature >= 0);
    proxied_voice_ptr *stack = new proxied_voice_ptr[measure.voices.size()];
    recurse(measure.voices.begin(), measure.voices.end(),
            stack, stack,
            time_signature);
    delete [] stack;

    if (complete and size() > 1) {
      rational best_score;
      bool single_best_score = false;
      for (reference possibility: *this) {
        rational const score(possibility.harmonic_mean());
        if (score > best_score) {
          best_score = score, single_best_score = true;
        } else if (score == best_score) {
          single_best_score = false;
        }
      }
      // Do not consider possibilities below a certain margin as valid
      if (single_best_score) {
        rational const margin(best_score * rational(2, 3));
        for (iterator measure = begin(); measure != end();
             measure = measure->harmonic_mean() < margin?
                       erase(measure): ++measure);
      }
    }
  }

  bool contains_complete_measure() const
  { return complete; }

  bool completes_uniquely(measure_interpretations const& other) {
    BOOST_ASSERT(not this->complete);
    BOOST_ASSERT(not other.complete);
    std::size_t matches = 0;
    for (const_reference lhs: *this) {
      for (const_reference rhs: other) {
        if (duration(lhs) + duration(rhs) == time_signature) ++matches;
      }
    }
    return matches == 1;
  }
};

}

}}

#endif

