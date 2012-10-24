// Copyright (C) 2011, 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_VALUE_DISAMBIGUATION_HPP
#define BMC_VALUE_DISAMBIGUATION_HPP

#include "bmc/braille/ast.hpp"
#include <cmath>
#include <memory>
#include <sstream>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/range/numeric.hpp>

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
  ast::value value_type:4;
  value_category category:4;

  rational const &undotted_duration() const
  {
    BOOST_ASSERT(category==large || category==small);
    BOOST_ASSERT(value_type >= 0 and value_type < 4);
    return undotted[category * 4 + value_type];
  }

  rational duration;
  rational calculate_duration(unsigned dots)
  {
    rational const base(undotted_duration());
    return dots? base * 2 - base / pow(2, dots): base;
  }

  rational *final_type;
  bool *whole_measure_rest;

public:
  value_proxy() : final_type(nullptr), whole_measure_rest(nullptr) {}
  value_proxy(ast::note &note, value_category const &category)
  : value_type(note.ambiguous_value), category(category)
  , duration(calculate_duration(note.dots))
  , final_type(&note.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ast::note &note, value_category const &category
             , ast::value value_type
             )
  : value_type(value_type), category(category)
  , duration(calculate_duration(note.dots))
  , final_type(&note.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy(ast::rest &rest, value_category const &category)
  : value_type(rest.ambiguous_value), category(category)
  , duration(calculate_duration(rest.dots))
  , final_type(&rest.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy(ast::rest &rest, value_category const &category
             , ast::value value_type
             )
  : value_type(value_type), category(category)
  , duration(calculate_duration(rest.dots))
  , final_type(&rest.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ast::rest &rest, value_category const &category
             , rational const& duration
             )
  : value_type(rest.ambiguous_value), category(category)
  , duration(duration)
  , final_type(&rest.type)
  , whole_measure_rest(&rest.whole_measure)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy(ast::chord &chord, value_category const &category)
  : value_type(chord.base.ambiguous_value), category(category)
  , duration(calculate_duration(chord.base.dots))
  , final_type(&chord.base.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  value_proxy( ast::chord &chord, value_category const &category
             , ast::value value_type
             )
  : value_type(value_type), category(category)
  , duration(calculate_duration(chord.base.dots))
  , final_type(&chord.base.type)
  , whole_measure_rest(nullptr)
  { BOOST_ASSERT(*final_type == zero); }

  operator rational const &() const { return duration; }

  bool operator==(value_proxy const &rhs) const
  { return final_type == rhs.final_type && duration == rhs.duration; }

  /** \brief Fill the infomation gathered about this partiuclar interpretation
    *        into the AST
   */
  void accept() const
  {
    BOOST_ASSERT(final_type);
    BOOST_ASSERT(*final_type == zero);
    if (whole_measure_rest) {
      *final_type = duration;
      *whole_measure_rest = true;
    } else {
      *final_type = undotted_duration();
    }
  }
};

inline rational
duration(std::vector<value_proxy> const &values)
{ return boost::accumulate(values, zero); }

class proxied_partial_voice : public std::vector<value_proxy>
{
  rational const duration;
  std::size_t use_count;
public:
  typedef std::vector<value_proxy> base_type;
  proxied_partial_voice( const_pointer begin, const_pointer end
                       , rational const &duration
                       )
  : base_type(begin, end)
  , duration(duration)
  , use_count(0)
  {}

  operator rational const &() const { return duration; }

  friend inline void intrusive_ptr_add_ref(proxied_partial_voice *p)
  { ++p->use_count; }

  friend inline void intrusive_ptr_release(proxied_partial_voice *p)
  { if (--p->use_count == 0) delete p; }
};

typedef boost::intrusive_ptr<proxied_partial_voice> proxied_partial_voice_ptr;

struct maybe_whole_measure_rest : boost::static_visitor<bool>
{
  result_type operator()(ast::rest const &rest) const
  {
    return rest.ambiguous_value == ast::whole_or_16th and
           rest.dots == 0;
  }
  template<typename Sign>
  result_type operator()(Sign const &) const { return false; }
};

class make_whole_measure_rest : public boost::static_visitor<value_proxy>
{
  rational const duration;
public:
  make_whole_measure_rest(rational const &duration) : duration(duration) {}
  result_type operator()(ast::rest &rest) const
  { return result_type(rest, large, duration); }
  template<typename Sign>
  result_type operator()(Sign const &) const
  { BOOST_ASSERT(false); return value_proxy(); }
};

class notegroup: public boost::static_visitor<void>
{
  value_category const category;
  ast::value type;
  value_proxy *const stack_begin, *stack_end;
public:
  notegroup( ast::partial_voice::iterator const &begin
           , ast::partial_voice::iterator const &end
           , value_category const &category
           , value_proxy *stack_end
           )
  : category(category), type(ast::unknown)
  , stack_begin(stack_end), stack_end(stack_end)
  { std::for_each(begin, end, apply_visitor(*this)); }

  result_type operator()(ast::note &note)
  {
    BOOST_ASSERT(note.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = note.ambiguous_value;
    new (stack_end++) value_proxy(note, category, type);
  }
  result_type operator()(ast::rest &rest)
  {
    BOOST_ASSERT(rest.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = rest.ambiguous_value;
    new (stack_end++) value_proxy(rest, category, type);
  }
  result_type operator()(ast::chord &chord)
  {
    BOOST_ASSERT(chord.base.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = chord.base.ambiguous_value;
    new (stack_end++) value_proxy(chord, category, type);
  }
  result_type operator()(ast::value_distinction const &) { BOOST_ASSERT(false); }
  result_type operator()(braille::hand_sign &) {}
  result_type operator()(ast::barline &) {}
  result_type operator()(ast::simile const &) { BOOST_ASSERT(false); }

  value_proxy *end() const { return stack_end; }
  rational duration() const
  { return std::accumulate(stack_begin, stack_end, zero); }
};

class same_category
: public std::vector<value_proxy>
, public boost::static_visitor<void>
{
  value_category const category;
public:
  same_category( ast::partial_voice::iterator const &begin
               , ast::partial_voice::iterator const &end
               , value_category const &category
               )
  : category(category)
  { std::for_each(begin, end, apply_visitor(*this)); }

  result_type operator()(ast::note &note)
  { emplace_back(note, category); }
  result_type operator()(ast::rest &rest)
  { emplace_back(rest, category); }
  result_type operator()(ast::chord &chord)
  { emplace_back(chord, category); }
  result_type operator()(ast::value_distinction const &) {}
  template<typename Sign> result_type operator()(Sign const &)
  { BOOST_ASSERT(false); }
};

template<typename Callback>
class partial_voice_interpreter
{
  static
  ast::partial_voice::iterator
  same_category_end( ast::partial_voice::iterator const &begin
                   , ast::partial_voice::iterator const &end
                   , ast::value_distinction::type distinction
                   )
  {
    if (apply_visitor(ast::is_value_distinction(distinction), *begin)) {
      ast::partial_voice::iterator iter(begin + 1);
      if (iter != end and apply_visitor(ast::is_rhythmic(), *iter)) {
        for (ast::value
             initial = apply_visitor(ast::get_ambiguous_value(), *iter++);
             iter != end and
             apply_visitor(ast::is_rhythmic(), *iter) and
             apply_visitor(ast::get_ambiguous_value(), *iter) == initial;
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
  ast::partial_voice::iterator
  notegroup_end( ast::partial_voice::iterator const &begin
               , ast::partial_voice::iterator const &end
               )
  {
    if (apply_visitor(ast::is_rhythmic(), *begin)) {
      if (apply_visitor(ast::get_ambiguous_value(), *begin) != ast::eighth_or_128th) {
        auto iter = begin + 1;
        while (iter != end and
               apply_visitor(ast::get_ambiguous_value(), *iter) == ast::eighth_or_128th)
          ++iter;
        // A note group is only valid if it consists of at least 3 rhythmic signs
        if (std::distance(begin, iter) > 2) return iter;
      }
    }
    return begin;
  }

  music::time_signature const &time_signature;
  rational const &start_position;
  rational const beat;
  Callback yield;
  ast::partial_voice &voice;
  ast::partial_voice::iterator const voice_end;
  value_proxy *const stack_begin;

  bool on_beat(rational const &position) const
  { return no_remainder(position, beat); }

  /** \brief Try the common large and small variations.
   */
  inline
  void large_and_small( ast::partial_voice::iterator const &iterator
                      , value_proxy *stack_end
                      , rational const &max_duration
                      , rational const &position
                      ) const;

public:
  void recurse( ast::partial_voice::iterator const &iterator
              , value_proxy *stack_end
              , rational const &max_duration
              , rational const &position
              ) const
  {
    if (iterator == voice_end) {
      yield(stack_begin, stack_end, position - start_position);
    } else {
      ast::partial_voice::iterator tail;
      if (on_beat(position) and
          (tail = notegroup_end(iterator, voice_end)) > iterator) {
        // Try all possible notegroups, starting with the longest
        while (std::distance(iterator, tail) >= 3) {
          notegroup const group(iterator, tail, small, stack_end);
          rational const group_duration(group.duration());
          if (group_duration <= max_duration) {
            rational const next_position(position + group_duration);
            if (on_beat(next_position)) {
              recurse(tail, group.end(), max_duration - group_duration, next_position);
            }
          }
          --tail;
        }

        large_and_small(iterator, stack_end, max_duration, position);
      } else if ((tail = same_category_end
                         ( iterator, voice_end
                         , ast::value_distinction::large_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, large);
        if (duration(group) <= max_duration) {
          recurse(tail, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group));
        }
      } else if ((tail = same_category_end
                         ( iterator, voice_end
                         , ast::value_distinction::small_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, small);
        if (duration(group) <= max_duration) {
          recurse(tail, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group));
        }
      } else {
        large_and_small(iterator, stack_end, max_duration, position);

        if (stack_begin == stack_end and position == 0 and
            time_signature != 1 and
            apply_visitor(maybe_whole_measure_rest(), *iterator)) {
          *stack_end = apply_visitor(make_whole_measure_rest(time_signature), *iterator);
          recurse(iterator + 1, stack_end + 1,
                  zero, position + time_signature);
        }
      }
    }
  }

  partial_voice_interpreter( ast::partial_voice &voice
                           , rational const &position
                           , music::time_signature const &time_sig
                           , Callback yield
                           )
  : time_signature(time_sig)
  , start_position(position)
  , beat(1, time_signature.denominator())
  , yield(yield)
  , voice(voice)
  , voice_end(voice.end())
  , stack_begin(new value_proxy[voice.size()])
  {}
  ~partial_voice_interpreter() { delete [] stack_begin; }

  void operator()(rational const &max_duration) const
  { recurse(voice.begin(), stack_begin, max_duration, start_position); }
};

template<typename Callback>
class large_and_small_visitor : public boost::static_visitor<bool>
{
  ast::partial_voice::iterator const &rest;
  value_proxy *const proxy;
  rational const &max_duration, &position;
  partial_voice_interpreter<Callback> const &interpreter;
public:
  large_and_small_visitor( ast::partial_voice::iterator const &rest
                         , value_proxy *stack_end
                         , rational const &max_duration
                         , rational const &position
                         , partial_voice_interpreter<Callback> const &interpreter
                         )
  : rest(rest)
  , proxy(stack_end)
  , max_duration(max_duration)
  , position(position)
  , interpreter(interpreter)
  {}

  template <class Value>
  result_type operator()(Value &value) const
  {
    if (not is_grace(value)) {
      value_proxy *const next = proxy + 1;
      if (*new(proxy)value_proxy(value, large) <= max_duration)
        interpreter.recurse( rest, next
                           , max_duration - *proxy, position + *proxy
                           );
      if (*new(proxy)value_proxy(value, small) <= max_duration)
        interpreter.recurse( rest, next
                           , max_duration - *proxy, position + *proxy
                           );
      return true;
    }
    return false;
  }
  result_type operator()(ast::value_distinction &) const { return false; }
  result_type operator()(braille::hand_sign &) const { return false; }
  result_type operator()(ast::barline &) const { return false; }
  result_type operator()(ast::simile &) const { return false; }
  bool is_grace(ast::note const &note) const
  {
    return std::find(note.articulations.begin(), note.articulations.end(),
                     appoggiatura)
           != note.articulations.end()
           ||
           std::find(note.articulations.begin(), note.articulations.end(),
                    short_appoggiatura)
           != note.articulations.end();
  }
  bool is_grace(ast::rest const &) const { return false; }
  bool is_grace(ast::chord const &chord) const { return is_grace(chord.base); }
};

template<typename Callback>
inline
void
partial_voice_interpreter<Callback>
::
large_and_small( ast::partial_voice::iterator const &iterator
               , value_proxy *stack_end
               , rational const &max_duration
               , rational const &position
               ) const
{
  // Skip this sign if it does not result in at least one possible proxy
  auto rest = iterator; ++rest;
  if (not apply_visitor( large_and_small_visitor<Callback>
                         (rest, stack_end, max_duration, position, *this)
                       , *iterator
                       )
     )
    recurse(rest, stack_end, max_duration, position);
}

template<typename Callback>
inline
void
partial_voice_interpretations( ast::partial_voice &voice
                             , rational const &max_duration
                             , rational const &position
                             , music::time_signature const &time_sig
                             , Callback yield
                             )
{
  partial_voice_interpreter<Callback>
  (voice, position, time_sig, yield)(max_duration);
}

inline
rational const &
duration(proxied_partial_voice_ptr const &partial_voice)
{ return *partial_voice; }

typedef std::vector<proxied_partial_voice_ptr> proxied_partial_measure;

typedef std::shared_ptr<proxied_partial_measure const> proxied_partial_measure_ptr;

template<typename Callback>
inline
void
partial_measure_interpretations( ast::partial_measure::iterator const &begin
                               , ast::partial_measure::iterator const &end
                               , proxied_partial_voice_ptr *stack_begin
                               , proxied_partial_voice_ptr *stack_end
                               , rational const &length
                               , rational const &position
                               , music::time_signature const &time_sig
                               , Callback yield
                               )
{
  if (begin == end) {
    if (stack_begin != stack_end) yield(stack_begin, stack_end);
  } else {
    auto const tail = begin + 1;
    if (stack_begin == stack_end) {
      partial_voice_interpretations
      ( *begin, length, position, time_sig
      , [stack_begin, stack_end, &tail, &end, &position, &time_sig, &yield]
        ( value_proxy const *f
        , value_proxy const *l
        , rational const &duration
        ) {
          stack_end->reset(new proxied_partial_voice(f, l, duration));
          partial_measure_interpretations( tail, end
                                         , stack_begin, stack_end + 1
                                         , duration, position
                                         , time_sig
                                         , yield
                                         );
        }
      );
    } else {
      partial_voice_interpretations
      ( *begin, length, position, time_sig
      , [ stack_begin, stack_end, &tail, &end
        , &length, &position, &time_sig
        , &yield
        ]
        ( value_proxy const *f
        , value_proxy const *l
        , rational const &duration
        ) {
          if (duration == length) {
            stack_end->reset(new proxied_partial_voice(f, l, duration));
            partial_measure_interpretations( tail, end
                                           , stack_begin, stack_end + 1
                                           , duration, position
                                           , time_sig
                                           , yield
                                           );
          }
        }
      );
    }
  }
}

template<typename Callback>
inline
void
partial_measure_interpretations( ast::partial_measure &partial_measure
                               , rational const &max_length
                               , rational const &position
                               , music::time_signature const &time_sig
                               , Callback callback
                               )
{
  proxied_partial_voice_ptr *
  stack = new proxied_partial_voice_ptr[partial_measure.size()];
  partial_measure_interpretations( partial_measure.begin()
                                 , partial_measure.end()
                                 , stack, stack
                                 , max_length, position, time_sig
                                 , callback
                                 );
  delete [] stack;
}

inline
rational
duration(proxied_partial_measure const &voices)
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

inline
rational
duration(proxied_partial_measure_ptr const &voices)
{ return duration(*voices); }

class proxied_voice : public std::vector<proxied_partial_measure_ptr>
{
  rational const duration;
public:
  typedef std::vector<proxied_partial_measure_ptr> base_type;
  proxied_voice( const_pointer begin, const_pointer end
               , rational const &duration
               )
  : base_type(begin, end)
  , duration(duration)
  {}
  operator rational const &() const { return duration; }
};

typedef std::shared_ptr<proxied_voice const> proxied_voice_ptr;

template<typename Callback>
inline
void
voice_interpretations( ast::voice::iterator const &begin
                     , ast::voice::iterator const &end
                     , proxied_partial_measure_ptr *stack_begin
                     , proxied_partial_measure_ptr *stack_end
                     , rational const &max_length
                     , rational const &position
                     , music::time_signature const &time_signature
                     , Callback yield
                     )
{
  if (begin == end) {
    if (stack_begin != stack_end) {
      yield(stack_begin, stack_end, position);
    }
  } else {
    auto const tail = begin + 1;
    partial_measure_interpretations
    ( *begin, max_length, position, time_signature
    , [ stack_begin, stack_end, &tail, &end
      , &max_length, &position, &time_signature
      , &yield
      ]
      ( proxied_partial_voice_ptr *f
      , proxied_partial_voice_ptr *l
      ) {
        stack_end->reset(new proxied_partial_measure(f, l));
        rational const partial_measure_duration(duration(*stack_end));
        voice_interpretations( tail, end, stack_begin, stack_end + 1
                             , max_length - partial_measure_duration
                             , position + partial_measure_duration
                             , time_signature
                             , yield
                             );
      }
    );
  }
}

template<typename Callback>
inline
void
voice_interpretations( ast::voice &voice
                     , rational const &max_length
                     , music::time_signature const &time_sig
                     , Callback callback
                     )
{
  proxied_partial_measure_ptr *
  stack = new proxied_partial_measure_ptr[voice.size()];
  voice_interpretations( voice.begin(), voice.end()
                       , stack, stack
                       , max_length, zero, time_sig
                       , callback
                       );
  delete [] stack;
}

inline
rational const &
duration(proxied_voice_ptr const &voice)
{ return *voice; }

/** @brief Represents a possible interpretation of an ast::measure.
 */
class proxied_measure : public std::vector<proxied_voice_ptr>
{
  rational mean;
public:
  proxied_measure(const_pointer begin, const_pointer end)
  : std::vector<proxied_voice_ptr>(begin, end)
  , mean() // Do not precalculate the harmonic mean as it is potentially unused
  {
  }

  /** @brief Harmonic mean of all contained rhythmic values.
   *
   * As the harmonic mean tends strongly toward the least elements of the list
   * it mitigates (compared to the arithmetic mean) the influence of large
   * outliers and increases the influence of small values.
   *
   * @see http://en.wikipedia.org/wiki/Harmonic_mean
   */
  rational const &harmonic_mean()
  {
    if (mean == zero) {
      // Avoid expensive (and unneeded) gcd in rational::operator+=
      rational::int_type n=0, d=1;
      for (const_reference voice: *this)
        for (proxied_voice::const_reference part: *voice)
          for (proxied_partial_measure::const_reference partial_voice: *part)
            for (rational const &value: *partial_voice)
              n = n*value.numerator() + d*value.denominator(),
              d *= value.numerator(),
              ++mean;
      mean /= rational(n, d);
    }
    return mean;
  }

  /** @breif Transfer the herein learnt information to the refered-to objects.
   *
   * @note This member function should only be called on one found result.
   */
  void accept() const
  {
    for (const_reference voice: *this)
      for (auto const &partial_measure: *voice)
        for (auto const &partial_voice: *partial_measure)
          for_each(partial_voice->begin(), partial_voice->end(),
                   std::mem_fun_ref(&value_proxy::accept));
  }
};

/** @brief Duration of a proxied_measure.
 */
inline
rational
duration(proxied_measure const &voices)
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
std::basic_ostream<Char> &
operator<<(std::basic_ostream<Char> &os, proxied_measure const &measure)
{
  for(proxied_measure::const_reference voice: measure) {
    os << '[';
    for(proxied_voice::const_reference part: *voice) {
      os << '{';
      for(proxied_partial_measure::const_reference partial_voice: *part) {
        os << '(';
        for(proxied_partial_voice::const_reference value: *partial_voice) {
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

class measure_interpretations: std::vector<proxied_measure>
{
  music::time_signature time_signature;
  bool complete;

  void recurse( std::vector<ast::voice>::iterator const& begin
              , std::vector<ast::voice>::iterator const& end
              , value_type::pointer stack_begin
              , value_type::pointer stack_end
              , rational const& length
              )
  {
    if (begin == end) {
      if (stack_begin != stack_end) {
        if (not complete or length == time_signature) {
          if (not complete and length == time_signature) {
            // We found the first intepretation matching the time signature.
            // So this is not an anacrusis.  Drop accumulated (incomplete)
            // interpretations and continue more efficiently.
            clear();
            complete = true;
          }
          emplace_back(stack_begin, stack_end);
        }
      }
    } else {
      auto const tail = begin + 1;
      voice_interpretations
      ( *begin, length, time_signature
      , [ stack_begin, stack_end, &tail, &end, &length, this ]
        ( proxied_partial_measure_ptr *f
        , proxied_partial_measure_ptr *l
        , rational const& duration
        ) {
          if ((stack_begin == stack_end and not this->complete) or
              (duration == length)) {
            stack_end->reset(new proxied_voice(f, l, duration));
            this->recurse(tail, end, stack_begin, stack_end + 1, duration);
          }
        }
      );
    }
  }

  void cleanup()
  {
    // Drop interpretations with a significant lower harmonic mean
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
        base_type good;
        for (reference measure: *this)
          if (measure.harmonic_mean() > margin) good.push_back(measure);
        assign(good.begin(), good.end());
      }
    }
  }

public:
  typedef std::vector<proxied_measure> base_type;

  measure_interpretations()
  : complete(false)
  {}

  measure_interpretations(measure_interpretations const& other)
  : base_type(other.begin(), other.end())
  , time_signature(other.time_signature)
  , complete(other.complete)
  {}

  measure_interpretations( ast::measure& measure
                         , music::time_signature const& time_signature
                         )
  : base_type()
  , time_signature(time_signature)
  , complete(false)
  {
    BOOST_ASSERT(time_signature >= 0);
    value_type::pointer
    stack = new value_type::value_type[measure.voices.size()];
    recurse(measure.voices.begin(), measure.voices.end(),
            stack, stack,
            time_signature);
    delete [] stack;

    cleanup();
  }

  bool contains_complete_measure() const
  { return complete; }

  bool completes_uniquely(measure_interpretations const& other) {
    BOOST_ASSERT(not this->complete);
    BOOST_ASSERT(not other.complete);
    BOOST_ASSERT(this->time_signature == other.time_signature);
    std::size_t matches = 0;
    for (const_reference lhs: *this)
      for (const_reference rhs: other)
        if (duration(lhs) + duration(rhs) == time_signature) ++matches;
    return matches == 1;
  }

  using base_type::begin;
  using base_type::clear;
  using base_type::end;
  using base_type::empty;
  using base_type::front;
  using base_type::size;
};

}

}}

#endif

