// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/braille/semantic_analysis/value_disambiguation.hpp"
#include <boost/variant/get.hpp>
#include <map>

namespace music { namespace braille { namespace value_disambiguation {

rational const undotted[8] = {
  rational{1, 1},  rational{1, 2},  rational{1, 4},  rational{1, 8},
  rational{1, 16}, rational{1, 32}, rational{1, 64}, rational{1, 128}
};

rational const &
value_proxy::undotted_duration() const
{
  BOOST_ASSERT(category==large || category==small);
  BOOST_ASSERT(value_type >= 0 and value_type < 4);
  return undotted[category | value_type];
}

rational
value_proxy::calculate_duration(unsigned dots) const
{
  rational result{undotted_duration()};
  if (dots) result *= augmentation_dots_factor(dots);
  if (tuplet_factor != rational{1}) result *= tuplet_factor;
  return result;
}

void
value_proxy::accept() const
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
  default:
  case ptr_type::uninitialized: BOOST_ASSERT(false);
  }
}

namespace {

struct maybe_whole_measure_rest : boost::static_visitor<bool>
{
  result_type operator()(ast::rest const &rest) const
  { return rest.ambiguous_value == ast::whole_or_16th and not rest.dots; }
  template<typename Sign>
  result_type operator()(Sign const &) const { return false; }
};

struct tuplet_info
{
  struct level {
    unsigned number = 1;
    rational factor = rational{1};
    bool first_tuplet = true;
    unsigned ttl = 0;
  };
  std::vector<level> levels;

  void operator()( rational &factor
                 , std::vector<rational> &tuplet_begin, unsigned &tuplet_end
                 , bool &dyadic_next_position
                 )
  {
    factor = rational{1};
    tuplet_begin.clear();
    tuplet_end = 0;
    dyadic_next_position = true;
    for (tuplet_info::level &level: levels) {
      if (level.ttl) {
        if (level.first_tuplet) {
          tuplet_begin.emplace_back(level.factor);
          level.first_tuplet = false;
        }
        if (level.ttl == 1) ++tuplet_end; else dyadic_next_position = false;
        factor *= level.factor;

        --level.ttl;
      }
    }
  }
};

bool
is_tuplet_begin( ast::partial_voice::iterator const &iterator
               , unsigned &number, bool &simple, bool &doubled
               )
{
  return apply_visitor(ast::is_tuplet_start(number, doubled, simple), *iterator);
}

ast::partial_voice::iterator
tuplet_end(ast::partial_voice::iterator begin, ast::partial_voice::iterator const &end, unsigned in_number, bool in_simple)
{
  unsigned number;
  bool doubled, simple;
  while (begin != end) {
    if (apply_visitor(ast::is_tuplet_start(number, doubled, simple), *begin)) {
      if (in_simple and simple) break;

      // This check prevents same-number tuplets to be nested.
      // This is a questionable limitation but helpful to cut on possible
      // interpretations.
      if (not in_simple and in_number == number) break;
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
  , tuplet(tuplet)
  { std::for_each(begin, end, apply_visitor(*this)); }

  result_type operator()(ast::note &note)
  {
    BOOST_ASSERT(note.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = note.ambiguous_value;
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    tuplet(factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(note, small, type, factor);
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
    tuplet(factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(rest, small, type, factor);
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
    tuplet(factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(chord, small, type, factor);
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
    tuplet(factor, tuplet_begin, tuplet_end, dyadic_next_position);
    new (stack_end) value_proxy(chord, small, type, factor);
    stack_end->set_tuplet_info(tuplet_begin, tuplet_end);
    stack_end++;
  }
  result_type operator()(ast::value_distinction const &) { BOOST_ASSERT(false); }
  // A note group must never contain a music hyphen.
  result_type operator()(ast::hyphen const &) { BOOST_ASSERT(false); }
  result_type operator()(braille::ast::tie &) {}
  result_type operator()(braille::ast::tuplet_start &) { BOOST_ASSERT(false); }
  result_type operator()(braille::hand_sign &) {}
  result_type operator()(ast::clef &) {}
  result_type operator()(ast::barline &) {}
  result_type operator()(ast::simile const &) { BOOST_ASSERT(false); }

  value_proxy *end() const
  {
    // Make value_proxies aware of the fact that they are part of a notegroup.
    auto iter = stack_begin;
    (iter++)->make_beam_begin();
    while (iter < stack_end - 1) (iter++)->make_beam_continue();
    iter->make_beam_end();
    BOOST_ASSERT(++iter == stack_end);

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
      auto iter = begin + 1;
      while (iter != end and
             apply_visitor(ast::get_ambiguous_value(), *iter) == ast::eighth_or_128th and
             not apply_visitor(ast::is_rest(), *iter) and
             not (apply_visitor(ast::get_augmentation_dots(), *iter) > 0) and
             not apply_visitor(ast::is_hyphen(), *iter))
        ++iter;
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
  result_type operator()(ast::value_distinction const &) {}
  template<typename Sign> result_type operator()(Sign const &)
  { BOOST_ASSERT(false); }
};

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
  return std::count_if(begin, end, apply_visitor(is_rhythmic));
}

/** A fast but potentially unsafe operator<= for rationals.
 *
 * Potentially overflows, which is not really relevant to callers.
 */
inline bool
fast_leq(rational const &lhs, rational const &rhs)
{
  return (lhs.numerator() * rhs.denominator()) <= (rhs.numerator() * lhs.denominator());
}

class partial_voice_interpreter
{
  bool const last_partial_measure;
  global_state const &state;
  rational const start_position;
  proxied_partial_voice::function const &yield;
  ast::partial_voice &voice;
  ast::partial_voice::iterator const voice_end;
  std::unique_ptr<value_proxy[]> stack_begin;

  /** \brief Try the common large and small variations.
   */
  inline
  void large_and_small( ast::partial_voice::iterator const &iterator
                      , value_proxy *stack_end
                      , rational const &max_duration
                      , rational const &position
                      , tuplet_info const &tuplet
                      ) const;

public:
  bool on_beat(rational const &position) const
  { return no_remainder(position, state.beat); }

  void recurse( ast::partial_voice::iterator const &iterator
              , value_proxy *stack_end
              , rational const &max_duration
              , rational const &position
              , tuplet_info const &tuplet
              ) const
  {
    if (iterator == voice_end) {
      if (not (last_partial_measure and state.exact_match_found
               and
               static_cast<bool>(max_duration)))
        yield(stack_begin.get(), stack_end, position - start_position);
    } else {
      unsigned tuplet_number = 0;
      bool simple_triplet, tuplet_doubled;
      ast::partial_voice::iterator tail;
      if (on_beat(position) and
          (tail = notegroup_end(iterator, voice_end)) > iterator) {
        {
          notegroup const group(iterator, tail, stack_end, tuplet);
          rational const group_duration(group.duration());
          if (fast_leq(group_duration, max_duration)) {
            rational const next_position(position + group_duration);
            if (on_beat(next_position)) {
              recurse(tail, group.end(), max_duration - group_duration, next_position, group.tuplet_state());
            }
          }
        }

        large_and_small(iterator, stack_end, max_duration, position, tuplet);
      } else if ((tail = same_category_end
                         ( iterator, voice_end
                         , ast::value_distinction::large_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, large);
        if (fast_leq(duration(group), max_duration)) {
          recurse(tail, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group), tuplet);
        }
      } else if ((tail = same_category_end
                         ( iterator, voice_end
                         , ast::value_distinction::small_follows
                         )
                 ) > iterator) {
        same_category const group(iterator, tail, small);
        if (fast_leq(duration(group), max_duration)) {
          recurse(tail, std::copy(group.begin(), group.end(), stack_end),
                  max_duration - duration(group),
                  position + duration(group), tuplet);
        }
      } else if (is_tuplet_begin(iterator, tuplet_number, simple_triplet, tuplet_doubled)) {
        tail = iterator + 1;
        tuplet_info t(tuplet);
        unsigned parent_ttl = t.levels.empty()? 0: t.levels.back().ttl;
        if (t.levels.empty() or t.levels.back().ttl > 0)
          t.levels.emplace_back();
        t.levels.back().number = tuplet_number;
        t.levels.back().first_tuplet = true;
        unsigned ttl = count_rhythmic(tail, tuplet_end(tail, voice_end, tuplet_number, simple_triplet));
        // A nested tuplet can not be longer then the tuplet it is contained in.
        if (parent_ttl and parent_ttl < ttl) ttl = parent_ttl;
        // Try all possible note counts and ratios.
        if (tuplet_doubled) {
          for (rational const &factor: tuplet_number_factors.at(tuplet_number)) {
            t.levels.back().factor = factor;
            t.levels.back().ttl = ttl;
            recurse(tail, stack_end, max_duration, position, t);
          }
        } else {
          for (t.levels.back().ttl = ttl; t.levels.back().ttl;
               --t.levels.back().ttl)
            for (rational const &factor: tuplet_number_factors.at(tuplet_number)) {
              t.levels.back().factor = factor;
              recurse(tail, stack_end, max_duration, position, t);
            }
        }
      } else {
        large_and_small(iterator, stack_end, max_duration, position, tuplet);

        if (stack_begin.get() == stack_end and position == 0 and
            state.time_signature != 1 and
            apply_visitor(maybe_whole_measure_rest(), *iterator)) {
          *stack_end = value_proxy(boost::get<ast::rest&>(*iterator), state.time_signature);
          recurse(std::next(iterator), std::next(stack_end),
                  zero, position + state.time_signature, tuplet);
        }
      }
    }
  }

  partial_voice_interpreter( ast::partial_voice &voice
                           , rational const &position
                           , bool last_partial_measure
                           , global_state const &state
                           , proxied_partial_voice::function const &yield
                           )
  : last_partial_measure(last_partial_measure)
  , state(state)
  , start_position(position)
  , yield(yield)
  , voice(voice)
  , voice_end(voice.end())
  , stack_begin(new value_proxy[voice.size()])
  {}

  void operator()(rational const &max_duration) const
  {
    tuplet_info tuplet;
    recurse(voice.begin(), stack_begin.get(), max_duration, start_position, tuplet);
  }
  rational const &last_measure_duration() const { return state.last_measure_duration; }
};

class large_and_small_visitor : public boost::static_visitor<bool>
{
  ast::partial_voice::iterator const &rest;
  value_proxy *const proxy;
  rational const &max_duration
               , &position;
  tuplet_info tuplet;               ;
  partial_voice_interpreter const &interpreter;
public:
  large_and_small_visitor( ast::partial_voice::iterator const &rest
                         , value_proxy *stack_end
                         , rational const &max_duration
                         , rational const &position
                         , tuplet_info const &tuplet
                         , partial_voice_interpreter const &interpreter
                         )
  : rest(rest)
  , proxy(stack_end)
  , max_duration(max_duration)
  , position(position)
  , tuplet(tuplet)
  , interpreter(interpreter)
  {}

  template <class Value>
  result_type operator()(Value &value)
  {
    rational factor;
    std::vector<rational> tuplet_begin;
    unsigned tuplet_end;
    bool dyadic_next_position;
    tuplet(factor, tuplet_begin, tuplet_end, dyadic_next_position);
    if (not is_grace(value)) {
      value_proxy *const next = proxy + 1;
      if (fast_leq(*new(proxy)value_proxy(value, large, factor), max_duration)) {
        rational const next_position(position + *proxy);
        // If this is a tuplet end, only accept it if its position makes sense.
        if (not dyadic_next_position or is_dyadic(next_position)) {
          proxy->set_tuplet_info(tuplet_begin, tuplet_end);
          interpreter.recurse( rest, next
                             , max_duration - *proxy, next_position, tuplet
                             );
        }
      }
      if (fast_leq(*new(proxy)value_proxy(value, small, factor), max_duration)) {
        rational const next_position(position + *proxy);
        if (not dyadic_next_position or is_dyadic(next_position)) {
          proxy->set_tuplet_info(tuplet_begin, tuplet_end);
          interpreter.recurse( rest, next
                             , max_duration - *proxy, next_position, tuplet
                             );
        }
      }
      return true;
    }
    return false;
  }
  result_type operator()(ast::value_distinction &) { return false; }
  result_type operator()(ast::hyphen &) { return false; }
  result_type operator()(braille::hand_sign &) { return false; }
  result_type operator()(ast::clef &) { return false; }
  result_type operator()(ast::tie &) { return false; }
  result_type operator()(ast::tuplet_start &) { return false; }
  result_type operator()(ast::barline &) { return false; }
  result_type operator()(ast::simile &simile)
  {
    if (not position) { // full measure simile
      BOOST_ASSERT(static_cast<bool>(interpreter.last_measure_duration()));
      if (*new(proxy)value_proxy
          (simile, interpreter.last_measure_duration()) > rational(0) and
          fast_leq(static_cast<rational>(*proxy) / simile.count, max_duration)) {
        rational const duration(static_cast<rational>(*proxy) / simile.count);
        interpreter.recurse( rest, proxy + 1
                           , max_duration - duration, position + duration, tuplet
                           );
      }
    } else { // partial measure simile
      if (interpreter.on_beat(position)) {
        if (fast_leq(*new(proxy)value_proxy(simile, position), max_duration))
          interpreter.recurse( rest, proxy + 1
                             , max_duration - *proxy, position + *proxy, tuplet
                             );
      }
    }
    return true;
  }

  bool is_grace(ast::note const &note)
  {
    return std::find_if(note.articulations.begin(), note.articulations.end(),
                        [](articulation a) {
                          return a == appoggiatura || a ==  short_appoggiatura;
                        }) != note.articulations.end();
  }
  bool is_grace(ast::rest const &) { return false; }
  bool is_grace(ast::chord const &chord) { return is_grace(chord.base); }
  bool is_grace(ast::moving_note &chord) { return is_grace(chord.base); }
};

inline
void
partial_voice_interpreter
::
large_and_small( ast::partial_voice::iterator const &iterator
               , value_proxy *stack_end
               , rational const &max_duration
               , rational const &position
               , tuplet_info const &tuplet
               ) const
{
  // Skip this sign if it does not result in at least one possible proxy
  auto rest = iterator; ++rest;
  large_and_small_visitor l_and_s(rest, stack_end, max_duration, position, tuplet, *this);
  if (not apply_visitor(l_and_s, *iterator))
    recurse(rest, stack_end, max_duration, position, tuplet);
}

template<typename Function>
inline
void
partial_measure_interpretations( ast::partial_measure::iterator const &begin
                               , ast::partial_measure::iterator const &end
                               , proxied_partial_voice::shared_ptr *stack_begin
                               , proxied_partial_voice::shared_ptr *stack_end
                               , rational const &length
                               , rational const &position
                               , bool last_partial_measure
                               , global_state const &state
                               , Function yield
                               )
{
  if (begin == end) {
    if (stack_begin != stack_end) yield(stack_begin, stack_end);
  } else {
    auto const tail = begin + 1;
    if (stack_begin == stack_end) {
      partial_voice_interpreter
      ( *begin, position, last_partial_measure, state
      , [ stack_begin, stack_end
        , &tail, &end
        , &position, last_partial_measure, &state
        , &yield
        ]
        ( value_proxy const *f
        , value_proxy const *l
        , rational const &duration
        ) {
          stack_end->reset(new proxied_partial_voice(f, l, duration));
          partial_measure_interpretations( tail, end
                                         , stack_begin, stack_end + 1
                                         , duration, position
                                         , last_partial_measure
                                         , state
                                         , yield
                                         );
        }
      )(length);
    } else {
      partial_voice_interpreter
      ( *begin, position, last_partial_measure, state
      , [ stack_begin, stack_end, &tail, &end
        , &length, &position, last_partial_measure, &state
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
                                           , last_partial_measure
                                           , state
                                           , yield
                                           );
          }
        }
      )(length);
    }
  }
}

template<typename Function>
inline
void
voice_interpretations( ast::voice::iterator const &begin
                     , ast::voice::iterator const &end
                     , proxied_partial_measure::shared_ptr *stack_begin
                     , proxied_partial_measure::shared_ptr *stack_end
                     , rational const &max_length
                     , rational const &position
                     , global_state const &state
                     , Function yield
                     )
{
  if (begin == end) {
    if (stack_begin != stack_end) yield(stack_begin, stack_end, position);
  } else {
    std::unique_ptr<proxied_partial_voice::shared_ptr[]>
    stack(new proxied_partial_voice::shared_ptr[begin->size()]);

    partial_measure_interpretations
    ( begin->begin(), begin->end()
    , stack.get(), stack.get()
    , max_length, position, std::next(begin) == end, state
    , [ stack_begin, stack_end, &begin, &end
      , &max_length, &position, &state
      , &yield
      ]
      ( proxied_partial_voice::shared_ptr const *f
      , proxied_partial_voice::shared_ptr const *l
      ) {
        stack_end->reset(new proxied_partial_measure(f, l));
        rational const partial_measure_duration(duration(*stack_end));
        voice_interpretations( std::next(begin), end
                             , stack_begin, std::next(stack_end)
                             , max_length - partial_measure_duration
                             , position + partial_measure_duration
                             , state
                             , yield
                             );
      }
    );
  }
}

}

rational const &
proxied_measure::harmonic_mean()
{
  if (not mean) {
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

void
measure_interpretations::recurse
( std::vector<ast::voice>::iterator const &begin
, std::vector<ast::voice>::iterator const &end
, value_type::pointer stack_begin
, value_type::pointer stack_end
, rational const &length
)
{
  if (begin == end) {
    if (stack_begin != stack_end) {
      if (not exact_match_found or length == time_signature) {
        if (not exact_match_found and length == time_signature) {
          // We found the first intepretation matching the time signature.
          // So this is not an anacrusis.  Drop accumulated (incomplete)
          // interpretations and continue more efficiently.
          clear();
          exact_match_found = true;
        }
        emplace_back(stack_begin, stack_end);
      }
    }
  } else {
    std::unique_ptr<proxied_partial_measure::shared_ptr[]>
    stack(new proxied_partial_measure::shared_ptr[begin->size()]);

    voice_interpretations
    ( begin->begin(), begin->end()
    , stack.get(), stack.get()
    , length, zero
    , *this
    , [ stack_begin, stack_end, &begin, &end, &length, this ]
      ( proxied_partial_measure::shared_ptr const *f
      , proxied_partial_measure::shared_ptr const *l
      , rational const &duration
      ) {
        if ((stack_begin == stack_end and not this->exact_match_found) or
            (duration == length)) {
          stack_end->reset(new proxied_voice(f, l, duration));
          this->recurse( std::next(begin), end
                       , stack_begin, std::next(stack_end)
                       , duration);
        }
      }
    );
  }
}

void
measure_interpretations::cleanup()
{
  // Drop interpretations with a significant lower harmonic mean
  if (exact_match_found and size() > 1) {
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
      rational const margin{best_score * rational{3, 4}};
      base_type good;
      for (reference measure: *this)
        if (measure.harmonic_mean() > margin) good.emplace_back(measure);
      assign(good.begin(), good.end());
    }
  }
}

}}}

