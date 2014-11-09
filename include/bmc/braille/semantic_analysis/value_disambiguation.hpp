// Copyright (C) 2011, 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_VALUE_DISAMBIGUATION_HPP
#define BMC_VALUE_DISAMBIGUATION_HPP

#include "bmc/braille/ast.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <sstream>
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
enum value_category { large = 0, small = 4 };

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
  } type;
  ast::value value_type:4;
  value_category category:4;
  ast::notegroup_member_type beam = ast::notegroup_member_type::none;
  std::vector<rational> tuplet_begin;
  unsigned tuplet_end = 0;
  union {
    ast::note *note_ptr;
    ast::rest *rest_ptr;
    ast::chord *chord_ptr;
    ast::moving_note *moving_note_ptr;
    ast::simile *simile_ptr;
  };
  rational tuplet_factor = rational{1};
  rational duration;

  rational const &undotted_duration() const;
  rational calculate_duration(unsigned dots) const;

public:
  value_proxy() : type(ptr_type::uninitialized) {}

  value_proxy(ast::note &note, value_category category, rational const &factor)
  : type(ptr_type::note), note_ptr(&note)
  , value_type(note.ambiguous_value), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(note.dots))
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy(ast::note &note, value_category category, ast::value value_type, rational const &factor)
  : type(ptr_type::note), note_ptr(&note)
  , value_type(value_type), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(note.dots))
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category category, rational const &factor)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(rest.ambiguous_value), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category category, ast::value value_type, rational const &factor)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(value_type), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, rational const &duration)
  : type(ptr_type::whole_measure_rest), rest_ptr(&rest)
  , duration(duration)
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::chord &chord, value_category category, rational const &factor)
  : type(ptr_type::chord), chord_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(chord_ptr->base.type == zero); }

  value_proxy( ast::chord &chord, value_category category
             , ast::value value_type, rational const &factor
             )
  : type(ptr_type::chord), chord_ptr(&chord)
  , value_type(value_type), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(chord_ptr->base.type == zero); }

  value_proxy(ast::moving_note &chord, value_category category, rational const &factor)
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy( ast::moving_note &chord, value_category category
             , ast::value value_type, rational const &factor
             )
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type(value_type), category(category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy(ast::simile &simile, rational const& duration)
  : type(ptr_type::simile), simile_ptr(&simile)
  , duration(duration * simile.count)
  { BOOST_ASSERT(simile_ptr->duration == zero); }

  void make_beam_begin()
  {
    BOOST_ASSERT(beam == ast::notegroup_member_type::none);
    beam = ast::notegroup_member_type::begin;
  }
  void make_beam_continue()
  {
    BOOST_ASSERT(beam == ast::notegroup_member_type::none);
    beam = ast::notegroup_member_type::middle;
  }
  void make_beam_end()
  {
    BOOST_ASSERT(beam == ast::notegroup_member_type::none);
    beam = ast::notegroup_member_type::end;
  }
  void set_tuplet_info(std::vector<rational> const &begin, unsigned end) {
    tuplet_begin = begin;
    tuplet_end = end;
  }

  operator rational const &() const { return duration; }

  /** \brief Fill the infomation gathered about this partiuclar interpretation
    *        into the AST
   */
  void accept() const;
};

inline rational
duration(std::vector<value_proxy> const &values)
{
  // This avoids rational{0} + x, which is always x.
  return values.empty()
         ? zero
         : std::accumulate(std::next(std::begin(values)), std::end(values),
                           static_cast<rational>(values.front()));
}

struct global_state
{
  music::time_signature time_signature;
  rational last_measure_duration;
  rational beat;
  std::atomic<bool> exact_match_found { false };
  std::atomic<short> threads { 1 };

  global_state() = default;
  global_state( global_state const &other )
  : time_signature { other.time_signature }
  , last_measure_duration { other.last_measure_duration }
  , beat { other.beat }
  , exact_match_found { other.exact_match_found.load() }
  , threads { other.threads.load() }
  {}
  global_state &operator=(global_state const &other)
  {
    time_signature = other.time_signature;
    last_measure_duration = other.last_measure_duration;
    beat = other.beat;
    exact_match_found.store(other.exact_match_found.load());
    threads.store(other.threads.load());
    return *this;
  }
  global_state( music::time_signature const &time_signature
              , rational const &last_measure_duration
              , rational const &beat
              )
  : time_signature{time_signature}
  , last_measure_duration{last_measure_duration}
  , beat{beat}
  , exact_match_found{false}
  {}
};

class proxied_partial_voice : public std::vector<value_proxy>
{
  rational const duration;
public:
  using base_type = std::vector<value_proxy>;
  proxied_partial_voice( const_pointer begin, const_pointer end
                       , rational const &duration
                       )
  : base_type{begin, end} // copy the given range of value_proxy objects
  , duration{duration}    // remember the accumulative duration of all elements
  {}

  operator rational const &() const { return duration; }

  using shared_ptr = std::shared_ptr<proxied_partial_voice>;
};

inline
rational const &
duration(proxied_partial_voice::shared_ptr const &partial_voice)
{ return *partial_voice; }

class proxied_partial_measure : public std::vector<proxied_partial_voice::shared_ptr>
{
public:
  using base_type = std::vector<proxied_partial_voice::shared_ptr>;
  proxied_partial_measure() = default;
  proxied_partial_measure(const_pointer begin, const_pointer end)
  : base_type{begin, end}
  {}
  proxied_partial_measure(proxied_partial_measure const &) = default;
  proxied_partial_measure(proxied_partial_measure &&) = default;

  using shared_ptr = std::shared_ptr<proxied_partial_measure>;
};

inline rational
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

inline rational
duration(proxied_partial_measure::shared_ptr const &voices)
{ return duration(*voices); }

class proxied_voice : public std::vector<proxied_partial_measure::shared_ptr>
{
  rational duration;
public:
  using base_type = std::vector<proxied_partial_measure::shared_ptr>;
  proxied_voice() = default;
  proxied_voice( const_pointer begin, const_pointer end
               , rational const &duration
               )
  : base_type{begin, end}
  , duration{duration}
  {}
  proxied_voice(proxied_voice const &) = default;
  proxied_voice(proxied_voice &&) = default;

  operator rational const &() const { return duration; }

  void set_duration(rational const &d) { duration = d; }

  using shared_ptr = std::shared_ptr<proxied_voice>;
};

inline rational const &
duration(proxied_voice::shared_ptr const &voice)
{ return *voice; }

/** @brief Represents a possible interpretation of an ast::measure.
 */
class proxied_measure : public std::vector<proxied_voice::shared_ptr>
{
  rational mean;
public:
  using base_type = std::vector<proxied_voice::shared_ptr>;

  proxied_measure() = default;
  proxied_measure(const_pointer begin, const_pointer end)
  : base_type{begin, end}
  , mean{} // Do not precalculate the harmonic mean as it is potentially unused
  {}
  proxied_measure(proxied_measure const &) = default;
  proxied_measure(proxied_measure &&) = default;
  proxied_measure &operator=(proxied_measure &&) = default;
  proxied_measure &operator=(proxied_measure const &) = default;

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
  ssize_t id;

  void recurse( std::vector<ast::voice>::iterator const& begin
              , std::vector<ast::voice>::iterator const& end
              , rational const &length, std::mutex &
              ) ;
  void recurse( std::vector<ast::voice>::iterator const& begin
              , std::vector<ast::voice>::iterator const& end
              , value_type &&outer_stack
              , rational const &length, std::mutex &
              ) ;

  void cleanup();

public:
  typedef std::vector<proxied_measure> base_type;

  measure_interpretations() : id{-1} {}

  measure_interpretations(measure_interpretations const& other)
  : base_type{other.begin(), other.end()}
  , global_state{other}
  , id{other.id}
  {}

  measure_interpretations( ast::measure& measure
                         , music::time_signature const &time_signature
                         , rational const &last_measure_duration = rational(0)
                         )
  : base_type{}
  , global_state{ time_signature, last_measure_duration
                , rational{1, time_signature.denominator()}
                }
  , id{measure.id}
  {
    BOOST_ASSERT(time_signature >= 0);
    std::mutex mutex;
    recurse( measure.voices.begin(), measure.voices.end()
           , time_signature, mutex
           ) ;

    cleanup();
  }

  bool contains_complete_measure() const
  { return exact_match_found; }

  bool completes_uniquely(measure_interpretations const &other) const
  {
    BOOST_ASSERT(not this->exact_match_found);
    BOOST_ASSERT(not other.exact_match_found);
    BOOST_ASSERT(this->time_signature == other.time_signature);
    std::size_t matches = 0;
    for (const_reference lhs: *this)
      for (const_reference rhs: other)
        if (duration(lhs) + duration(rhs) == time_signature) ++matches;
    return matches == 1;
  }

  ssize_t get_measure_id() const { return id; }

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

