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

namespace bmc { namespace braille {

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
enum value_category { large_value = 0, small_value = 4 };

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
  union {
    ast::note *note_ptr;
    ast::rest *rest_ptr;
    ast::chord *chord_ptr;
    ast::moving_note *moving_note_ptr;
    ast::simile *simile_ptr;
  };
  ast::value value_type:4;
  value_category category:4;
  ast::notegroup_member_type beam = ast::notegroup_member_type::none;
  std::vector<rational> tuplet_begin;
  unsigned tuplet_end = 0;
  rational tuplet_factor = rational{1};
  rational duration;

  rational const &undotted_duration() const;
  rational calculate_duration(unsigned dots) const;

public:
  value_proxy() : type(ptr_type::uninitialized) {}

  value_proxy(ast::note &note, value_category note_value_category, rational const &factor)
  : type(ptr_type::note), note_ptr(&note)
  , value_type(note.ambiguous_value), category(note_value_category)
  , tuplet_factor{factor}
  , duration{calculate_duration(note.dots)}
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy( ast::note &note, value_category note_value_category
             , ast::value note_value_type, rational const &factor)
  : type{ptr_type::note}, note_ptr{&note}
  , value_type{note_value_type}, category{note_value_category}
  , tuplet_factor{factor}
  , duration(calculate_duration(note.dots))
  { BOOST_ASSERT(note_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category rest_value_category, rational const &factor)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(rest.ambiguous_value), category(rest_value_category)
  , tuplet_factor{factor}
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, value_category rest_value_category, ast::value rest_value_type, rational const &factor)
  : type(ptr_type::rest), rest_ptr(&rest)
  , value_type(rest_value_type), category(rest_value_category)
  , tuplet_factor{factor}
  , duration(calculate_duration(rest.dots))
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::rest &rest, rational const &whole_measure_rest_duration)
  : type{ptr_type::whole_measure_rest}, rest_ptr{&rest}
  , duration(whole_measure_rest_duration)
  { BOOST_ASSERT(rest_ptr->type == zero); }

  value_proxy(ast::chord &chord, value_category chord_value_category, rational const &factor)
  : type(ptr_type::chord), chord_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(chord_value_category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(chord_ptr->base.type == zero); }

  value_proxy( ast::chord &chord, value_category chord_value_category
             , ast::value chord_value_type, rational const &factor
             )
  : type{ptr_type::chord}, chord_ptr{&chord}
  , value_type{chord_value_type}, category{chord_value_category}
  , tuplet_factor{factor}
  , duration{calculate_duration(chord.base.dots)}
  { BOOST_ASSERT(chord_ptr->base.type == zero); }

  value_proxy(ast::moving_note &chord, value_category moving_note_value_category, rational const &factor)
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type(chord.base.ambiguous_value), category(moving_note_value_category)
  , tuplet_factor{factor}
  , duration(calculate_duration(chord.base.dots))
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy( ast::moving_note &chord, value_category moving_note_value_category
             , ast::value moving_note_value_type, rational const &factor
             )
  : type(ptr_type::moving_note), moving_note_ptr(&chord)
  , value_type{moving_note_value_type}, category{moving_note_value_category}
  , tuplet_factor{factor}
  , duration{calculate_duration(chord.base.dots)}
  { BOOST_ASSERT(moving_note_ptr->base.type == zero); }

  value_proxy(ast::simile &simile, rational const& simile_duration)
  : type{ptr_type::simile}, simile_ptr{&simile}
  , duration{simile_duration * int(simile.count)}
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

  template<class> friend bool refers_to(value_proxy const &);

  /** \brief Fill the infomation gathered about this partiuclar interpretation
    *        into the AST
   */
  void accept() const;
};

template<class> bool refers_to(value_proxy const &) { return false; }
template<> inline bool refers_to<ast::simile>(value_proxy const &proxy)
{ return proxy.type == value_proxy::ptr_type::simile; }

inline rational
duration(std::vector<value_proxy> const &values)
{
  // This avoids rational{0} + x, which is always x.
  return values.empty()
         ? zero
         : std::accumulate(std::next(std::begin(values)), std::end(values),
                           static_cast<rational>(values.front()),
			   [](rational const &a, value_proxy const &b) {
			     return a + rational(b);
			   });
}

struct global_state
{
  ::bmc::time_signature time_signature;
  rational last_measure_duration;
  rational beat;
  std::atomic<bool> exact_match_found { false };
  std::atomic<unsigned int> threads { 1 };

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
  global_state( ::bmc::time_signature const &time_signature
              , rational const &last_measure_duration
              , rational const &beat
              )
  : time_signature{time_signature}
  , last_measure_duration{last_measure_duration}
  , beat{beat}
  , exact_match_found{false}
  {}
};

struct doubled_tuplet_level {
  unsigned number = 1;
  rational factor;
};
using partial_voice_doubled_tuplet_info = std::vector<doubled_tuplet_level>;
using partial_measure_doubled_tuplet_info = std::vector<partial_voice_doubled_tuplet_info>;
using measure_doubled_tuplet_info = std::vector<partial_measure_doubled_tuplet_info>;

class proxied_partial_voice : public std::vector<value_proxy>
{
  rational const duration;
  partial_voice_doubled_tuplet_info const doubled_tuplets;
public:
  using base_type = std::vector<value_proxy>;
  proxied_partial_voice( const_pointer begin, const_pointer end
                       , rational const &duration
                       , partial_voice_doubled_tuplet_info const &doubled_tuplets
                       )
  : base_type{begin, end} // copy the given range of value_proxy objects
  , duration{duration}    // remember the accumulative duration of all elements
  , doubled_tuplets{doubled_tuplets}
  {}

  operator rational const &() const { return duration; }

  partial_voice_doubled_tuplet_info const &
  get_doubled_tuplets() const
  { return doubled_tuplets; }

  using shared_ptr = std::shared_ptr<proxied_partial_voice>;
};

inline
rational const &
duration(proxied_partial_voice::shared_ptr const &partial_voice)
{ return *partial_voice; }

using proxied_partial_measure = std::vector<proxied_partial_voice::shared_ptr>;

inline rational duration(proxied_partial_measure const &voices)
{
  rational value;
  if (!voices.empty()) {
    value = duration(voices.front());
#if !defined(NDEBUG)
    for (auto voice = std::next(voices.begin()); voice != voices.end(); ++voice)
      BOOST_ASSERT(value == duration(*voice));
#endif
  }
  return value;
}

inline rational duration(std::shared_ptr<proxied_partial_measure> const &partial_measure)
{ return duration(*partial_measure); }

using proxied_voice = std::vector<std::shared_ptr<proxied_partial_measure>>;

inline rational duration(std::shared_ptr<proxied_voice> const &voice)
{
  BOOST_ASSERT(voice);
  return std::accumulate(voice->begin(), voice->end(), zero, []
    (rational const &lhs, std::shared_ptr<proxied_partial_measure> const &rhs) {
      return lhs + duration(rhs);
    }
  );
}

/** @brief Represents a possible interpretation of an ast::measure.
 */
class proxied_measure : public std::vector<std::shared_ptr<proxied_voice>>
{
  rational mean;
public:
  using base_type = std::vector<std::shared_ptr<proxied_voice>>;

  proxied_measure() = default;
  proxied_measure(proxied_measure const &) = default;

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

  measure_doubled_tuplet_info
  get_doubled_tuplets() const
  {
    measure_doubled_tuplet_info result;
    for (const_reference voice: *this) {
      result.emplace_back();
      for (auto &&partial_voice: *voice->back()) {
        result.back().push_back(partial_voice->get_doubled_tuplets());
      }
    }
    return result;
  }
};

/** @brief Duration of a proxied_measure.
 */
inline rational duration(proxied_measure const &voices)
{
  rational value;
  if (!voices.empty()) {
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
  int id;

public:
  typedef std::vector<proxied_measure> base_type;

  measure_interpretations() : id{-1} {}

  measure_interpretations(measure_interpretations const& other)
  : base_type{other.begin(), other.end()}
  , global_state{other}
  , id{other.id}
  {}

  measure_interpretations( ast::measure& measure
                         , ::bmc::time_signature const &time_signature
                         , rational const &last_measure_duration
                         , measure_doubled_tuplet_info const &last_doubled_tuplets
                         );
  bool contains_complete_measure() const
  { return exact_match_found; }

  bool completes_uniquely(measure_interpretations const &other) const
  {
    BOOST_ASSERT(!this->exact_match_found);
    BOOST_ASSERT(!other.exact_match_found);
    BOOST_ASSERT(this->time_signature == other.time_signature);
    std::size_t matches = 0;
    for (const_reference lhs: *this)
      for (const_reference rhs: other)
        if (duration(lhs) + duration(rhs) == time_signature) ++matches;
    return matches == 1;
  }

  int get_measure_id() const { return id; }

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

