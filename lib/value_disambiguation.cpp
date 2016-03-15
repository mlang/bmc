// Copyright (C) 2012-2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/braille/semantic_analysis/value_disambiguation.hpp"
#include <boost/variant/get.hpp>
#include <future>
#include <map>

namespace bmc { namespace braille { namespace value_disambiguation {

rational const undotted[8] = {
  rational{1, 1},  rational{1, 2},  rational{1, 4},  rational{1, 8},
  rational{1, 16}, rational{1, 32}, rational{1, 64}, rational{1, 128}
};

rational const &value_proxy::undotted_duration() const
{
  BOOST_ASSERT(category==large_value || category==small_value);
  BOOST_ASSERT(value_type >= 0 && value_type < 4);
  return undotted[category | value_type];
}

rational value_proxy::calculate_duration(unsigned dots) const
{
  rational result{undotted_duration()};
  if (dots) result *= augmentation_dots_factor(dots);
  if (tuplet_factor != rational{1}) result *= tuplet_factor;
  return result;
}

void value_proxy::accept() const
{
  switch (type) {
  case ptr_type::note:
    note_ptr->type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      note_ptr->notegroup_member = beam;
    note_ptr->factor = tuplet_factor;
    note_ptr->tuplet_begin = tuplet_begin;
    note_ptr->tuplet_end = tuplet_end;
    break;
  case ptr_type::rest:
    rest_ptr->type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      rest_ptr->notegroup_member = beam;
    rest_ptr->factor = tuplet_factor;
    rest_ptr->tuplet_begin = tuplet_begin;
    rest_ptr->tuplet_end = tuplet_end;
    break;
  case ptr_type::whole_measure_rest:
    rest_ptr->type = duration;
    rest_ptr->whole_measure = true;
    break;
  case ptr_type::chord:
    chord_ptr->base.type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      chord_ptr->base.notegroup_member = beam;
    chord_ptr->base.factor = tuplet_factor;
    chord_ptr->base.tuplet_begin = tuplet_begin;
    chord_ptr->base.tuplet_end = tuplet_end;
    break;
  case ptr_type::moving_note:
    moving_note_ptr->base.type = undotted_duration();
    moving_note_ptr->base.factor = tuplet_factor;
    moving_note_ptr->base.tuplet_begin = tuplet_begin;
    moving_note_ptr->base.tuplet_end = tuplet_end;
    break;
  case ptr_type::simile:
    simile_ptr->duration = duration;
    break;
  case ptr_type::uninitialized: BOOST_ASSERT(false);
  }
}

namespace {

bool maybe_whole_measure_rest(ast::sign const &sign) {
  auto rest = boost::get<ast::rest>(&sign);
  return rest && rest->ambiguous_value == ast::whole_or_16th && !rest->dots;
}

struct tuplet_level {
  unsigned number = 1;
  rational factor = rational{1};
  bool first_tuplet = true;
  unsigned ttl = 0;
  bool doubled = false;
};

using tuplet_info = std::vector<tuplet_level>;

void process_tuplet_info( tuplet_info &tuplet
                        , rational &factor
                        , std::vector<rational> &tuplet_begin, unsigned &tuplet_end
                        , bool &dyadic_next_position
                        )
{
  factor = rational{1};
  tuplet_begin.clear();
  tuplet_end = 0;
  dyadic_next_position = true;

  for (auto &&level: tuplet) {
    if (level.ttl) {
      if (level.first_tuplet) {
        tuplet_begin.push_back(level.factor);
        level.first_tuplet = false;
      }
      if (level.ttl == 1) ++tuplet_end; else dyadic_next_position = false;
      factor *= level.factor;

      --level.ttl;
    }
  }

  if (!tuplet.empty() && !tuplet.back().ttl && !tuplet.back().doubled) {
    tuplet.pop_back();
  }
}

partial_voice_doubled_tuplet_info extract_doubled( tuplet_info const &tuplets )
{
  partial_voice_doubled_tuplet_info result;
  for (auto &&level: tuplets) {
    if (level.doubled) {
      result.emplace_back();
      result.back().number = level.number;
      result.back().factor = level.factor;
    }
  }
  return result;
}

bool
is_tuplet_begin( ast::partial_voice::iterator const &iterator
               , unsigned &number, bool &simple, bool &doubled
               )
{
  if (auto tuplet_start = boost::get<ast::tuplet_start>(&*iterator)) {
    number = tuplet_start->number();
    simple = tuplet_start->simple_triplet();
    doubled = tuplet_start->doubled();
    return true;
  }
  return false;
}

ast::partial_voice::iterator
tuplet_end( ast::partial_voice::iterator begin
	  , ast::partial_voice::iterator const &end
	  , unsigned in_number, bool in_simple
	  )
{
  unsigned number;
  bool doubled, simple;
  while (begin != end) {
    if (ast::is_simile(*begin)) break;

    if (is_tuplet_begin(begin, number, simple, doubled)) {
      if (in_simple && simple) break;

      // This check prevents same-number tuplets to be nested.
      // This is a questionable limitation but helpful to cut on possible
      // interpretations.
      if (!in_simple && in_number == number) break;
    }
    ++begin;
  }
  return begin;
}

class notegroup: public boost::static_visitor<void>
{
  ast::value type;
  value_proxy *const stack_begin, *stack_end;
  tuplet_info tuplet;
public:
  notegroup( ast::partial_voice::iterator const &begin
           , ast::partial_voice::iterator const &end
           , value_proxy *stack_end
           , tuplet_info const &tuplet
           )
  : type{ast::unknown}
  , stack_begin{stack_end}, stack_end{stack_end}
  , tuplet{tuplet}
  { std::for_each(begin, end, apply_visitor(*this)); }

  result_type operator()(ast::note &note)
  {
    BOOST_ASSERT(note.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = note.ambiguous_value;
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    process_tuplet_info(tuplet, factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(note, small_value, type, factor);
    stack_end->set_tuplet_info(tuplet_begin, tuplet_end);
    stack_end++;
  }
  result_type operator()(ast::rest &rest)
  {
    BOOST_ASSERT(rest.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = rest.ambiguous_value;
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    process_tuplet_info(tuplet, factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(rest, small_value, type, factor);
    stack_end->set_tuplet_info(tuplet_begin, tuplet_end);
    stack_end++;
  }
  result_type operator()(ast::chord &chord)
  {
    BOOST_ASSERT(chord.base.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = chord.base.ambiguous_value;
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    process_tuplet_info(tuplet, factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(chord, small_value, type, factor);
    stack_end->set_tuplet_info(tuplet_begin, tuplet_end);
    stack_end++;
  }
  result_type operator()(ast::moving_note &chord)
  {
    BOOST_ASSERT(chord.base.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = chord.base.ambiguous_value;
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    process_tuplet_info(tuplet, factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(chord, small_value, type, factor);
    stack_end->set_tuplet_info(tuplet_begin, tuplet_end);
    stack_end++;
  }
  result_type operator()(ast::value_prefix const &) { BOOST_ASSERT(false); }
  // A note group must never contain a music hyphen.
  result_type operator()(ast::hyphen const &) { BOOST_ASSERT(false); }
  result_type operator()(braille::ast::tie &) {}
  result_type operator()(braille::ast::tuplet_start &) { BOOST_ASSERT(false); }
  result_type operator()(braille::ast::hand_sign &) {}
  result_type operator()(ast::clef &) {}
  result_type operator()(ast::barline &) {}
  result_type operator()(ast::simile const &) { BOOST_ASSERT(false); }

  value_proxy *end() const
  {
    // Make value_proxies aware of the fact that they are part of a notegroup.
    auto iter = stack_begin, last = std::prev(stack_end);
    (iter++)->make_beam_begin();
    while (iter < last) (iter++)->make_beam_continue();
    (iter++)->make_beam_end();
    BOOST_ASSERT(iter == stack_end);

    return stack_end;
  }

  rational duration() const
  {
    BOOST_ASSERT(stack_begin != stack_end);
    return std::accumulate(std::next(stack_begin), stack_end,
                           static_cast<rational>(*stack_begin));
  }

  tuplet_info const &tuplet_state() const { return tuplet; }
};

/**
 * \brief Find end of note group.
 *
 * \param begin The beginning of the range of signs to examine
 * \param end   The end of the range of signs to examine
 * \return If note group was found the returned iterator points one element
 *         beyond its last element.  Otherwise, begin is returned.
 */
ast::partial_voice::iterator
notegroup_end( ast::partial_voice::iterator const &begin
             , ast::partial_voice::iterator const &end
             )
{
  if (apply_visitor(ast::is_rhythmic(), *begin)) {
    if (apply_visitor(ast::get_ambiguous_value(), *begin) != ast::eighth_or_128th) {
      auto iter = std::next(begin);
      while (iter != end &&
             apply_visitor(ast::get_ambiguous_value(), *iter) == ast::eighth_or_128th &&
             !ast::is_rest(*iter) &&
             !apply_visitor(ast::get_augmentation_dots(), *iter) &&
             !ast::is_hyphen(*iter))
	std::advance(iter, 1);
      // A note group is only valid if it consists of at least 3 rhythmic signs
      if (std::distance(begin, iter) > 2) return iter;
    }
  }
  return begin;
}

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
  { emplace_back(note, category, rational(1)); }
  result_type operator()(ast::rest &rest)
  { emplace_back(rest, category); }
  result_type operator()(ast::chord &chord)
  { emplace_back(chord, category, rational(1)); }
  result_type operator()(ast::moving_note &chord)
  { emplace_back(chord, category, rational(1)); }
  result_type operator()(ast::value_prefix const &) {}
  template<typename Sign> result_type operator()(Sign const &)
  { BOOST_ASSERT(false); }
};

ast::partial_voice::iterator
same_category_end( ast::partial_voice::iterator const &begin
                 , ast::partial_voice::iterator const &end
                 , ast::value_prefix::type prefix
                 )
{
  if (apply_visitor(ast::is_value_prefix(prefix), *begin)) {
    ast::partial_voice::iterator iter(begin + 1);
    if (iter != end && apply_visitor(ast::is_rhythmic(), *iter)) {
      for (ast::value
           initial = apply_visitor(ast::get_ambiguous_value(), *iter++);
           iter != end &&
           apply_visitor(ast::is_rhythmic(), *iter) &&
           apply_visitor(ast::get_ambiguous_value(), *iter) == initial;
           ++iter);
      return iter;
    }
  }
  return begin;
}

std::map<unsigned, std::vector<rational>> tuplet_number_factors =
{{2, {{3, 2}}}
,{3, {{2, 3}}}
,{4, {{3, 4}}}
,{5, {{2, 5}
     ,{4, 5}}}
,{6, {{4, 6}}}
,{7, {{4, 7}
     ,{8, 7}}}
};

unsigned
count_rhythmic( ast::partial_voice::iterator const &begin
              , ast::partial_voice::iterator const &end
              )
{
  ast::is_rhythmic is_rhythmic;
  return unsigned(std::count_if(begin, end, apply_visitor(is_rhythmic)));
}

/** A fast but potentially unsafe operator<= for rationals.
 *
 * Potentially overflows, which is not really relevant to callers.
 */
inline bool
fast_leq(rational const &lhs, rational const &rhs)
{
  return (lhs.numerator() * rhs.denominator())
      <= (rhs.numerator() * lhs.denominator());
}

template<typename Function>
class partial_voice_interpreter
{
  bool const last_partial_measure;
  global_state &state;
  rational const start_position;
  Function yield;
  unsigned int const max_threads = 4;

  /** \brief Try the common large and small variations.
   */
  inline
  void large_and_small( ast::partial_voice::iterator const &iterator
                      , ast::partial_voice::iterator const &end
                      , value_proxy *stack_begin, value_proxy *stack_end
                      , rational const &max_duration, rational const &position
                      , tuplet_info const &tuplet
                      ) const;

public:
  bool on_beat(rational const &position) const
  { return no_remainder(position, state.beat); }

  void recurse( ast::partial_voice::iterator const &iterator
              , ast::partial_voice::iterator const &end
              , value_proxy *const stack_begin, value_proxy *stack_end
              , rational const &max_duration, rational const &position
              , tuplet_info const &tuplet
              ) const
  {
    if (iterator == end) {
      if (!(last_partial_measure && state.exact_match_found &&
            static_cast<bool>(max_duration))) {
        yield(stack_begin, stack_end, position - start_position, tuplet);
      }
    } else {
      unsigned tuplet_number = 0;
      bool simple_triplet, tuplet_doubled;
      ast::partial_voice::iterator tail;
      if (on_beat(position) &&
          (tail = notegroup_end(iterator, end)) > iterator) {
        {
          notegroup const group(iterator, tail, stack_end, tuplet);
          rational const group_duration(group.duration());
          if (fast_leq(group_duration, max_duration)) {
            rational const next_position(position + group_duration);
            if (on_beat(next_position)) {
              recurse( tail, end, stack_begin, group.end()
                     , max_duration - group_duration, next_position
                     , group.tuplet_state());
            }
          }
        }

        large_and_small(iterator, end, stack_begin, stack_end, max_duration, position, tuplet);
      } else if ((tail = same_category_end
                         ( iterator, end
                         , ast::value_prefix::large_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, large_value);
        if (fast_leq(duration(group), max_duration)) {
          recurse(tail, end, stack_begin, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group)
                  , tuplet);
        }
      } else if ((tail = same_category_end
                         ( iterator, end
                         , ast::value_prefix::small_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, small_value);
        if (fast_leq(duration(group), max_duration)) {
          recurse(tail, end, stack_begin, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group)
                  , tuplet);
        }
      } else if (is_tuplet_begin(iterator, tuplet_number, simple_triplet, tuplet_doubled)) {
        tail = iterator + 1;
        tuplet_info t(tuplet);
        unsigned parent_ttl = t.empty()? 0: t.back().ttl;

        if (!t.empty() && t.back().doubled &&
            !tuplet_doubled && t.back().number == tuplet_number) {
          // explicitly terminated doubled tuplet
          t.back().doubled = false;
          t.back().first_tuplet = true;
          t.back().ttl = count_rhythmic(tail, tuplet_end(tail, end, tuplet_number, simple_triplet));
          for (; t.back().ttl; --t.back().ttl)
            recurse(tail, end, stack_begin, stack_end, max_duration, position, t);
        } else {
          if (t.empty() || t.back().ttl > 0) t.emplace_back();

          t.back().number = tuplet_number;
          t.back().first_tuplet = true;
          unsigned ttl = count_rhythmic(tail, tuplet_end(tail, end, tuplet_number, simple_triplet));
          // A nested tuplet can not be longer then the tuplet it is contained in.
          if (parent_ttl && parent_ttl < ttl) ttl = parent_ttl;
          // Try all possible note counts and ratios.
          if (tuplet_doubled) {
            t.back().doubled = true;
  	    t.back().ttl = ttl;
            for (rational const &factor: tuplet_number_factors.at(tuplet_number)) {
              t.back().factor = factor;
              recurse( tail, end, stack_begin, stack_end, max_duration, position, t);
            }
          } else {
            for (t.back().ttl = ttl; t.back().ttl; --t.back().ttl) {
              for (rational const &factor: tuplet_number_factors.at(tuplet_number)) {
                t.back().factor = factor;
                recurse( tail, end, stack_begin, stack_end, max_duration, position
                       , t);
              }
            }
          }
        }
      } else {
        large_and_small( iterator, end, stack_begin, stack_end
                       , max_duration, position, tuplet);

        if (stack_begin == stack_end && position == 0 &&
            state.time_signature != 1 && maybe_whole_measure_rest(*iterator)) {
          *stack_end = value_proxy(boost::get<ast::rest>(*iterator), state.time_signature);
          recurse( std::next(iterator), end, stack_begin, std::next(stack_end)
                 , zero, position + state.time_signature, tuplet);
        }
      }
    }
  }

  partial_voice_interpreter( rational const &position
                           , bool last_partial_measure
                           , global_state &state
                           , Function yield
                           )
  : last_partial_measure{last_partial_measure}
  , state{state}
  , start_position{position}
  , yield{yield}
  {}

  void operator()( ast::partial_voice::iterator const &begin
                 , ast::partial_voice::iterator const &end
                 , partial_voice_doubled_tuplet_info const &doubled_tuplets
                 , proxied_partial_voice::pointer stack_begin
                 , proxied_partial_voice::pointer stack_end
                 , rational const &max_duration)
  {
    tuplet_info tuplet;
    for (auto &&factor: doubled_tuplets) {
      tuplet.emplace_back();
      tuplet.back().doubled = true;
      tuplet.back().factor = factor.factor;
      tuplet.back().number = factor.number;
      tuplet.back().first_tuplet = true;
      tuplet.back().ttl = count_rhythmic(begin, tuplet_end(begin, end, factor.number, true));
    }

    recurse(begin, end, stack_begin, stack_end, max_duration, start_position
           , tuplet);
  }
  rational const &last_measure_duration() const { return state.last_measure_duration; }
};

template<typename Interpreter>
class large_and_small_visitor : public boost::static_visitor<bool>
{
  ast::partial_voice::iterator const &rest;
  ast::partial_voice::iterator const &end;
  value_proxy *const stack_begin;
  value_proxy *const proxy;
  rational const &max_duration, &position;
  global_state &state;
  tuplet_info const &tuplet_ref;
  unsigned int max_threads;
  Interpreter const &interpreter;
public:
  large_and_small_visitor( ast::partial_voice::iterator const &rest
                         , ast::partial_voice::iterator const &end
                         , value_proxy *stack_begin, value_proxy *stack_end
                         , rational const &max_duration
                         , rational const &position
                         , global_state &state
                         , tuplet_info const &tuplet
                         , unsigned int max_threads
                         , Interpreter const &interpreter
                         )
  : rest{rest}, end{end}, stack_begin{stack_begin}, proxy{stack_end}
  , max_duration{max_duration}, position{position}
  , state { state }
  , tuplet_ref{tuplet}, max_threads{max_threads}
  , interpreter{interpreter}
  {}

  template <class Value>
  result_type operator()(Value &value) const
  {
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    tuplet_info tuplet{tuplet_ref};
    process_tuplet_info(tuplet, factor, tuplet_begin, tuplet_end, dyadic_next_position);
    std::unique_ptr<value_proxy[]> new_stack {};
    std::future<void> future;
    if (!is_grace(value)) {
      value_proxy *const next = proxy + 1;
      if (fast_leq(*new(proxy)value_proxy(value, large_value, factor), max_duration)) {
        rational const next_position(position + *proxy);
        // If this is a tuplet end, only accept it if its position makes sense.
        if (!dyadic_next_position || is_dyadic(next_position)) {
          if (std::distance(rest, end) >= 10 && state.threads < max_threads) {
            state.threads++;
            new_stack.reset(
              new value_proxy[std::distance(stack_begin, proxy + 1) +
                              std::distance(rest, end)]
            );
            value_proxy *new_proxy {
              std::prev(std::copy(stack_begin, proxy + 1, new_stack.get()))
            };
            new_proxy->set_tuplet_info(tuplet_begin, tuplet_end);
            future = std::async(std::launch::async, [&]() {
              interpreter.recurse( rest, end, new_stack.get(), new_proxy + 1
                                 , max_duration - *new_proxy, next_position
                                 , tuplet
                                 );
              state.threads--;
            });
          } else {
            proxy->set_tuplet_info(tuplet_begin, tuplet_end);
            interpreter.recurse( rest, end, stack_begin, next
                               , max_duration - *proxy, next_position, tuplet
                               );
          }
        }
      }
      if (fast_leq(*new(proxy)value_proxy(value, small_value, factor), max_duration)) {
        rational const next_position(position + *proxy);
        if (!dyadic_next_position || is_dyadic(next_position)) {
          proxy->set_tuplet_info(tuplet_begin, tuplet_end);
          interpreter.recurse( rest, end, stack_begin, next
                             , max_duration - *proxy, next_position, tuplet
                             );
        }
      }
      if (future.valid()) future.get();
      return true;
    }
    return false;
  }
  result_type operator()(ast::simile &simile) const
  {
    if (!position) { // full measure simile
      BOOST_ASSERT(static_cast<bool>(interpreter.last_measure_duration()));
      if (*new(proxy)value_proxy
          (simile, interpreter.last_measure_duration()) > rational(0) &&
          fast_leq(static_cast<rational>(*proxy) / rational::int_type(simile.count), max_duration)) {
        rational const duration(static_cast<rational>(*proxy) / rational::int_type(simile.count));
        interpreter.recurse( rest, end, stack_begin, proxy + 1
                           , max_duration - duration, position + duration
                           , tuplet_ref
                           );
      }
    } else { // partial measure simile
      if (is_dyadic(position)) {
        rational const repeated_duration {
          std::accumulate
          ( stack_begin, proxy, zero
          , [](rational const &lhs, value_proxy const &rhs)
            {
              // Reset to zero if we found a (partial measure) simile.
              if (refers_to<ast::simile>(rhs)) return rational{};

              return lhs + static_cast<rational>(rhs);
            }
          )
        };
        if (fast_leq(*new(proxy)value_proxy(simile, repeated_duration), max_duration)) {
          tuplet_info tuplet{tuplet_ref};
          if (!tuplet.empty() && tuplet.back().doubled) {
            BOOST_ASSERT(tuplet.back().ttl == 0);
            tuplet.back().first_tuplet = true;
            tuplet.back().ttl = count_rhythmic(rest, tuplet_end(rest, end, tuplet.back().number, true));
          }
          interpreter.recurse( rest, end, stack_begin, proxy + 1
                             , max_duration - *proxy, position + *proxy
                             , tuplet
                             );
        }
      }
    }
    return true;
  }
  result_type operator()(ast::value_prefix &) const { return false; }
  result_type operator()(ast::hyphen &) const { return false; }
  result_type operator()(braille::ast::hand_sign &) const { return false; }
  result_type operator()(ast::clef &) const { return false; }
  result_type operator()(ast::tie &) const { return false; }
  result_type operator()(ast::tuplet_start &) const { return false; }
  result_type operator()(ast::barline &) const { return false; }
};

template<typename Function>
inline void
partial_voice_interpreter<Function>::large_and_small
( ast::partial_voice::iterator const &iterator
, ast::partial_voice::iterator const &end
, value_proxy *const stack_begin, value_proxy *stack_end
, rational const &max_duration, rational const &position
, tuplet_info const &tuplet
) const
{
  // Skip this sign if it does not result in at least one possible proxy
  auto rest = iterator; ++rest;
  if (!boost::apply_visitor(large_and_small_visitor<partial_voice_interpreter<Function>>
                        { rest, end, stack_begin, stack_end
                        , max_duration, position, state, tuplet, max_threads
                        , *this
                        }, *iterator))
    recurse(rest, end, stack_begin, stack_end, max_duration, position, tuplet);
}

template<typename Function>
void
interpretations( ast::partial_voice::iterator const &begin
               , ast::partial_voice::iterator const &end
               , partial_voice_doubled_tuplet_info const &doubled_tuplets
               , rational const &max_duration, rational const &position
               , bool last_partial_measure
               , global_state &state, Function&& yield
               )
{
  std::unique_ptr<proxied_partial_voice::value_type[]> stack {
    new proxied_partial_voice::value_type[std::distance(begin, end)]
  };

  partial_voice_interpreter<Function>
  (position, last_partial_measure, state, std::forward<Function>(yield))
  (begin, end, doubled_tuplets, stack.get(), stack.get(), max_duration);
}

template<typename Function>
void
interpretations( ast::partial_measure::iterator const &begin
               , ast::partial_measure::iterator const &end
               , partial_measure_doubled_tuplet_info::const_iterator dt_begin
               , partial_measure_doubled_tuplet_info::const_iterator dt_end
               , proxied_partial_measure &&candidate
               , rational const &length
               , rational const &position
               , bool last_partial_measure
               , global_state &state, Function&& yield
               )
{
  if (begin == end) {
    yield(std::move(candidate));
  } else {
    auto const next = std::next(begin);
    partial_voice_doubled_tuplet_info tuplet_data;
    if (dt_begin != dt_end) {
      tuplet_data.assign(dt_begin->begin(), dt_begin->end());
      dt_begin++;
    }

    interpretations
    ( begin->begin(), begin->end(), tuplet_data
    , length, position, last_partial_measure, state
    , [&]( value_proxy const *f, value_proxy const *l, rational const &duration
         , tuplet_info const &tuplet
         )
      {
        if (duration == length) {
          proxied_partial_measure copy { candidate };
          copy.push_back (
            std::make_shared<proxied_partial_voice> (
              f, l, duration, extract_doubled(tuplet)
            )
          );
          interpretations( next, end, dt_begin, dt_end
                         , std::move(copy), duration, position
                         , last_partial_measure, state, yield
                         );
        }
      }
    );
  }
}

template<typename Function>
void
interpretations( ast::partial_measure::iterator const &begin
               , ast::partial_measure::iterator const &end
               , partial_measure_doubled_tuplet_info::const_iterator dt_begin
               , partial_measure_doubled_tuplet_info::const_iterator dt_end
               , rational const &length, rational const &position
               , bool last_partial_measure
               , global_state &state, Function&& yield
               )
{
  if (begin != end) {
    auto const next = std::next(begin);
    partial_voice_doubled_tuplet_info tuplet_data;
    if (dt_begin != dt_end) {
      tuplet_data.assign(dt_begin->begin(), dt_begin->end());
      dt_begin++;
    }

    interpretations
    ( begin->begin(), begin->end(), tuplet_data
    , length, position, last_partial_measure, state
    , [&]( value_proxy const *f, value_proxy const *l, rational const &duration
	 , tuplet_info const &tuplet
	 )
      {
        proxied_partial_measure candidate { };
        candidate.push_back (
          std::make_shared<proxied_partial_voice> (
            f, l, duration, extract_doubled(tuplet)
          )
        );
        interpretations( next, end, dt_begin, dt_end
                       , std::move(candidate), duration, position
                       , last_partial_measure, state, yield
                       );
      }
    );
  }
}

template<typename Function>
void
interpretations( ast::voice::iterator const &begin
               , ast::voice::iterator const &end
               , proxied_voice &&candidate
               , rational const &max_length, rational const &position
               , global_state &state, Function&& yield
               )
{
  if (begin == end) {
    yield(std::move(candidate), position);
  } else {
    auto const next = std::next(begin);
    partial_measure_doubled_tuplet_info tuplet_data;

    interpretations
    ( begin->begin(), begin->end(), tuplet_data.begin(), tuplet_data.end(), max_length, position, next == end, state
    , [&](proxied_partial_measure &&p)
      {
        rational const partial_measure_duration { duration(p) };
        proxied_voice copy { candidate };
        copy.push_back(std::make_shared<proxied_partial_measure>(std::move(p)));
        interpretations( next, end, std::move(copy)
                       , max_length - partial_measure_duration
                       , position + partial_measure_duration
                       , state, yield
                       );
      }
    );
  }
}

template<typename Function>
void
interpretations( ast::voice::iterator const &begin
               , ast::voice::iterator const &end
               , partial_measure_doubled_tuplet_info::const_iterator dt_begin
               , partial_measure_doubled_tuplet_info::const_iterator dt_end
               , rational const &max_length, rational const &position
               , global_state &state, Function&& yield
               )
{
  if (begin != end) {
    auto next = std::next(begin);

    interpretations
    ( begin->begin(), begin->end(), dt_begin, dt_end
    , max_length, position, next == end, state
    , [&](proxied_partial_measure &&p)
      {
        rational const partial_measure_duration { duration(p) };
        proxied_voice candidate { };
        candidate.push_back(std::make_shared<proxied_partial_measure>(std::move(p)));
        interpretations( next, end, std::move(candidate)
                       , max_length - partial_measure_duration
                       , position + partial_measure_duration
                       , state, yield
                       );
      }
    );
  }
}

template<typename Function>
void
interpretations( std::vector<ast::voice>::iterator const &begin
               , std::vector<ast::voice>::iterator const &end
               , measure_doubled_tuplet_info::const_iterator dt_begin
               , measure_doubled_tuplet_info::const_iterator dt_end
               , proxied_measure &&candidate
               , rational const &length, global_state &state, Function&& yield
               )
{
  if (begin == end) {
    yield(std::move(candidate), length);
  } else {
    auto const next = std::next(begin);
    partial_measure_doubled_tuplet_info tuplet_data;
    if (dt_begin != dt_end) {
      tuplet_data.assign(dt_begin->begin(), dt_begin->end());
      dt_begin++;
    }

    interpretations
    ( begin->begin(), begin->end(), tuplet_data.begin(), tuplet_data.end()
    , length, zero, state
    , [&](proxied_voice &&p, rational const &position)
      {
        if (position == length) {
          proxied_measure copy { candidate };
          copy.push_back(std::make_shared<proxied_voice>(std::move(p)));
          interpretations(next, end, dt_begin, dt_end, std::move(copy), position, state, yield);
        }
      }
    );
  }
}

template<typename Function>
void
interpretations( std::vector<ast::voice>::iterator const &begin
               , std::vector<ast::voice>::iterator const &end
               , measure_doubled_tuplet_info::const_iterator dt_begin
               , measure_doubled_tuplet_info::const_iterator dt_end
               , rational const &length, global_state &state, Function&& yield
               )
{
  if (begin != end) {
    auto const next = std::next(begin);
    partial_measure_doubled_tuplet_info tuplet_data;
    if (dt_begin != dt_end) {
      tuplet_data.assign(dt_begin->begin(), dt_begin->end());
      dt_begin++;
    }

    interpretations
    ( begin->begin(), begin->end(), tuplet_data.begin(), tuplet_data.end()
    , length, zero, state
    , [&](proxied_voice &&p, rational const &position)
      {
        if (!state.exact_match_found || position == length) {
          proxied_measure candidate { };
          candidate.push_back(std::make_shared<proxied_voice>(std::move(p)));
          interpretations(next, end, dt_begin, dt_end, std::move(candidate), position, state, yield);
        }
      }
    );
  }
}

}

rational const &
proxied_measure::harmonic_mean()
{
  if (!mean) {
    // Avoid expensive (and unneeded) gcd in rational::operator+=
    rational::int_type n=0, d=1, count=0;
    for (const_reference voice: *this)
      for (proxied_voice::const_reference part: *voice)
        for (proxied_partial_measure::const_reference partial_voice: *part)
          for (rational const &value: *partial_voice)
            n = n*value.numerator() + d*value.denominator(),
            d *= value.numerator(),
            ++count;
    mean.assign(count*d, n);
  }
  return mean;
}

void
proxied_measure::accept() const
{
  for (const_reference voice: *this)
    for (auto const &partial_measure: *voice)
      for (auto const &partial_voice: *partial_measure)
        for_each(partial_voice->begin(), partial_voice->end(),
                 std::mem_fun_ref(&value_proxy::accept));
}

namespace {

template< std::size_t MinItemsPerThread, typename Iterator
        , typename Tuple = std::tuple<Iterator, bool>
        >
Tuple best_harmonic_mean(Iterator first, Iterator last, unsigned int threads)
{
  std::vector<std::future<Tuple>> results;

  if (threads > 1) {
    auto const size = std::distance(first, last);
    while (threads > 1 && std::size_t(size / threads) < MinItemsPerThread) threads--;

    auto const chunk_size = size / threads--;
    while (threads--) {
      Iterator const last_ { std::next(first, chunk_size) };
      results.push_back
      ( std::async( std::launch::async
                  , &best_harmonic_mean<MinItemsPerThread, Iterator, Tuple>
                  , first, last_, 1));
      first = last_;
    }
  }

  if (first == last) {
    BOOST_ASSERT(results.empty());
    return Tuple { last, false };
  }

  Tuple result { first++, true };

  while (first < last) { 
    rational const &best_mean { std::get<0>(result)->harmonic_mean() }
                 , &curr_mean { first->harmonic_mean() };
    if (best_mean < curr_mean) {
      std::get<0>(result) = first, std::get<1>(result) = true;
    } else if (best_mean == curr_mean) {
      std::get<1>(result) = false;
    }

    std::advance(first, 1);
  }

  for (auto &&future: std::move(results)) {
    Tuple other { future.get() };

    if (std::get<0>(result)->harmonic_mean() < std::get<0>(other)->harmonic_mean()) {
      result = std::move(other);
    } else if (std::get<0>(result)->harmonic_mean() == std::get<0>(other)->harmonic_mean()) {
      std::get<1>(result) = false;
    }
  }

  return result;
}

}

measure_interpretations::measure_interpretations
( ast::measure& measure
, ::bmc::time_signature const &time_signature
, rational const &last_measure_duration
, measure_doubled_tuplet_info const &last_measure_doubled_tuplets
)
: base_type{}
, global_state
  {
    time_signature, last_measure_duration,
    rational{1, time_signature.denominator()}
  }
, id { measure.id }
{
  BOOST_ASSERT(time_signature >= 0);

  std::mutex mutex{};

  // Find all possible measure interpretations.
  interpretations
  ( measure.voices.begin(), measure.voices.end()
  , last_measure_doubled_tuplets.begin(), last_measure_doubled_tuplets.end()
  , time_signature, *this
  , [&](proxied_measure &&p, rational const &length)
    {
      if (!exact_match_found || length == time_signature) {
        std::lock_guard<std::mutex> lock { mutex };

        if (!exact_match_found && length == time_signature) {
          // We found the first intepretation matching the time signature.
          // So this is not an anacrusis.  Drop accumulated (incomplete)
          // interpretations and continue more efficiently.
          clear();
          exact_match_found = true;
        }
        push_back(std::move(p));
      }
    }
  );

  // Drop interpretations with a significant lower harmonic mean.
  if (exact_match_found && size() > 1) {
    auto best = best_harmonic_mean<5000>(begin(), end(), 4);
    if (std::get<1>(best)) {
      auto const margin = std::get<0>(best)->harmonic_mean() * rational{3, 4};
      erase(partition(begin(), end(), [&margin](reference measure) {
        return !fast_leq(measure.harmonic_mean(), margin);
      }), end());
    }
  }
}

}}}
