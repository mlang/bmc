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

/**
 * \brief A possible interpretation of a value (note, rest or chord).
 *
 * This class saves a pointer to the original sign it was used to create such
 * that it can later fill in the actual value type it stands for.
 */
class value_proxy
{
  enum class ptr_type: uint8_t
  {
    uninitialized, note, rest, whole_measure_rest, chord, moving_note, simile
  };
  ptr_type type;  
  ast::value value_type:4;
  value_category category:4;
  union {
    ast::note *note_ptr;
    ast::rest *rest_ptr;
    ast::chord *chord_ptr;
    ast::moving_note *moving_note_ptr;
    ast::simile *simile_ptr;
  };
  rational duration;

  rational const &undotted_duration() const;
  rational calculate_duration(unsigned dots) const;

public:
  value_proxy()
  : type(ptr_type::uninitialized)
  {}

  value_proxy(ast::note &note, value_category category)
  : type(ptr_type::note), note_ptr(&note)
  , value_type(note.ambiguous_value), category(category)
  , duration(calculate_duration(note.dots))
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy(ast::note &note, value_category category, ast::value value_type)
  : type(ptr_type::note), note_ptr(&note)
  , value_type(value_type), category(category)
  , duration(calculate_duration(note.dots))
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category category)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(rest.ambiguous_value), category(category)
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category category, ast::value value_type)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(value_type), category(category)
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, rational const &duration)
  : type(ptr_type::whole_measure_rest), rest_ptr(&rest)
  , duration(duration)
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::chord &chord, value_category category)
  : type(ptr_type::chord), chord_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(category)
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(chord_ptr->type == zero); }

  value_proxy( ast::chord &chord, value_category category
             , ast::value value_type
             )
  : type(ptr_type::chord), chord_ptr(&chord)
  , value_type(value_type), category(category)
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(chord_ptr->type == zero); }

  value_proxy(ast::moving_note &chord, value_category category)
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(category)
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy( ast::moving_note &chord, value_category category
             , ast::value value_type
             )
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type(value_type), category(category)
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy(ast::simile &simile, rational const& duration)
  : type(ptr_type::simile), simile_ptr(&simile)
  , duration(duration * simile.count)
  { BOOST_ASSERT(simile_ptr->duration == zero); }

  operator rational const &() const { return duration; }

  bool operator==(value_proxy const &rhs) const
  { return type == rhs.type && note_ptr == rhs.note_ptr && duration == rhs.duration; }

  /** \brief Fill the infomation gathered about this partiuclar interpretation
    *        into the AST
   */
  void accept() const;
};

inline rational
duration(std::vector<value_proxy> const &values)
{ return boost::accumulate(values, zero); }

struct global_state
{
  music::time_signature time_signature;
  rational last_measure_duration;
  rational beat;
  bool exact_match_found;

  global_state()
  : time_signature()
  , beat(1, time_signature.denominator())
  , exact_match_found(false)
  {}

  global_state( music::time_signature const &time_signature
              , rational const &last_measure_duration
              , rational const &beat
              )
  : time_signature(time_signature)
  , last_measure_duration(last_measure_duration)
  , beat(beat)
  , exact_match_found(false)
  {}
};

class proxied_partial_voice : public std::vector<value_proxy>
{
  rational const duration;
  std::size_t use_count;
public:
  typedef std::vector<value_proxy> base_type;
  proxied_partial_voice( const_pointer begin, const_pointer end
                       , rational const &duration
                       )
  : base_type(begin, end) // copy the given range of value_proxy objects
  , duration(duration)    // remember the accumulative duration of all elements
  , use_count(0)          // initialize use count to 0
  {}

  operator rational const &() const { return duration; }

  friend inline void intrusive_ptr_add_ref(proxied_partial_voice *p)
  { ++p->use_count; }

  friend inline void intrusive_ptr_release(proxied_partial_voice *p)
  { if (--p->use_count == 0) delete p; }

  typedef boost::intrusive_ptr<proxied_partial_voice> shared_ptr;

  typedef std::function<void( value_proxy const *
                            , value_proxy const *
                            , rational const &
                            )
                       >
          function;

  static void
  foreach( ast::partial_voice &
         , rational const &max_duration
         , rational const &position
         , bool last_partial_measure
         , global_state const &
         , function const &
         ) ;
};

inline
rational const &
duration(proxied_partial_voice::shared_ptr const &partial_voice)
{ return *partial_voice; }

struct proxied_partial_measure : std::vector<proxied_partial_voice::shared_ptr>
{
  typedef std::shared_ptr<proxied_partial_measure const> shared_ptr;
  proxied_partial_measure(const_pointer begin, const_pointer end)
  : std::vector<proxied_partial_voice::shared_ptr>(begin, end)
  {}

  typedef std::function<void( proxied_partial_voice::shared_ptr const *
                            , proxied_partial_voice::shared_ptr const *
                            )
                       >
          function;

  static void
  foreach( ast::partial_measure &partial_measure
         , rational const &max_length
         , rational const &position
         , bool last_partial_measure
         , global_state const &
         , function const &
         ) ;
};

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
duration(proxied_partial_measure::shared_ptr const &voices)
{ return duration(*voices); }

class proxied_voice : public std::vector<proxied_partial_measure::shared_ptr>
{
  rational const duration;
public:
  typedef std::vector<proxied_partial_measure::shared_ptr> base_type;
  proxied_voice( const_pointer begin, const_pointer end
               , rational const &duration
               )
  : base_type(begin, end)
  , duration(duration)
  {}
  operator rational const &() const { return duration; }

  typedef std::shared_ptr<proxied_voice const> shared_ptr;
  typedef std::function<void( proxied_partial_measure::shared_ptr const *
                            , proxied_partial_measure::shared_ptr const *
                            , rational const &
                            )
                       >
          function;

  static void
  foreach( ast::voice &voice
         , rational const &max_length
         , global_state const &
         , function const &
         ) ;
};

inline
rational const &
duration(proxied_voice::shared_ptr const &voice)
{ return *voice; }

/** @brief Represents a possible interpretation of an ast::measure.
 */
class proxied_measure : public std::vector<proxied_voice::shared_ptr>
{
  rational mean;
public:
  proxied_measure(const_pointer begin, const_pointer end)
  : std::vector<proxied_voice::shared_ptr>(begin, end)
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
  rational const &harmonic_mean();

  /** @brief Transfer the herein learnt information to the refered-to objects.
   *
   * @note This member function should only be called on one found result.
   */
  void accept() const;
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

class measure_interpretations: std::vector<proxied_measure>, public global_state
{
  std::size_t id;

  void recurse( std::vector<ast::voice>::iterator const& begin
              , std::vector<ast::voice>::iterator const& end
              , value_type::pointer stack_begin
              , value_type::pointer stack_end
              , rational const &length
	      ) ;

  void cleanup();

public:
  typedef std::vector<proxied_measure> base_type;

  measure_interpretations()
  : id(0)
  {}

  measure_interpretations(measure_interpretations const& other)
  : base_type(other.begin(), other.end())
  , global_state(other)
  , id(other.id)
  {}

  measure_interpretations( ast::measure& measure
                         , music::time_signature const &time_signature
                         , rational const &last_measure_duration = rational(0)
                         )
  : base_type()
  , global_state( time_signature
                , last_measure_duration
                , rational(1, time_signature.denominator())
                )
  , id(measure.id)
  {
    BOOST_ASSERT(time_signature >= 0);
    std::unique_ptr<value_type::value_type[]>
    stack(new value_type::value_type[measure.voices.size()]);
    recurse( measure.voices.begin(), measure.voices.end()
           , stack.get(), stack.get()
           , time_signature
           ) ;

    cleanup();
  }

  bool contains_complete_measure() const
  { return exact_match_found; }

  bool completes_uniquely(measure_interpretations const &other) const
  {
    BOOST_ASSERT(not this->complete);
    BOOST_ASSERT(not other.complete);
    BOOST_ASSERT(this->time_signature == other.time_signature);
    std::size_t matches = 0;
    for (const_reference lhs: *this)
      for (const_reference rhs: other)
        if (duration(lhs) + duration(rhs) == time_signature) ++matches;
    return matches == 1;
  }

  std::size_t get_measure_id() const { return id; }

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

