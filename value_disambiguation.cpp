// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/braille/semantic_analysis/value_disambiguation.hpp"
#include <boost/variant/get.hpp>

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
  return dots ? undotted_duration() * augmentation_dots_factor(dots)
              : undotted_duration();
}

void
value_proxy::accept() const
{
  switch (type) {
  case ptr_type::note:
    note_ptr->type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      note_ptr->notegroup_member = beam;
    break;
  case ptr_type::rest:
    rest_ptr->type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      rest_ptr->notegroup_member = beam;
    break;
  case ptr_type::whole_measure_rest:
    rest_ptr->type = duration;
    rest_ptr->whole_measure = true;
    break;
  case ptr_type::chord:
    chord_ptr->base.type = undotted_duration();
    if (beam != ast::notegroup_member_type::none)
      chord_ptr->base.notegroup_member = beam;
    break;
  case ptr_type::moving_note:
    moving_note_ptr->base.type = undotted_duration();
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

class notegroup: public boost::static_visitor<void>
{
  ast::value type;
  value_proxy *const stack_begin, *stack_end;
public:
  notegroup( ast::partial_voice::iterator const &begin
           , ast::partial_voice::iterator const &end
           , value_proxy *stack_end
           )
  : type{ast::unknown}
  , stack_begin{stack_end}, stack_end{stack_end}
  { std::for_each(begin, end, apply_visitor(*this)); }

  result_type operator()(ast::note &note)
  {
    BOOST_ASSERT(note.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = note.ambiguous_value;
    new (stack_end++) value_proxy(note, small, type);
  }
  result_type operator()(ast::rest &rest)
  {
    BOOST_ASSERT(rest.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = rest.ambiguous_value;
    new (stack_end++) value_proxy(rest, small, type);
  }
  result_type operator()(ast::chord &chord)
  {
    BOOST_ASSERT(chord.base.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = chord.base.ambiguous_value;
    new (stack_end++) value_proxy(chord, small, type);
  }
  result_type operator()(ast::moving_note &chord)
  {
    BOOST_ASSERT(chord.base.ambiguous_value != ast::unknown);
    if (type == ast::unknown) type = chord.base.ambiguous_value;
    new (stack_end++) value_proxy(chord, small, type);
  }
  result_type operator()(ast::value_distinction const &) { BOOST_ASSERT(false); }
  result_type operator()(braille::ast::tie &) {}
  result_type operator()(braille::hand_sign &) {}
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
  result_type operator()(ast::moving_note &chord)
  { emplace_back(chord, category); }
  result_type operator()(ast::value_distinction const &) {}
  template<typename Sign> result_type operator()(Sign const &)
  { BOOST_ASSERT(false); }
};

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
               apply_visitor(ast::get_ambiguous_value(), *iter) == ast::eighth_or_128th and
               not apply_visitor(ast::is_rest(), *iter))
          ++iter;
        // A note group is only valid if it consists of at least 3 rhythmic signs
        if (std::distance(begin, iter) > 2) return iter;
      }
    }
    return begin;
  }

  bool last_partial_measure;
  global_state const &state;
  rational const &start_position;
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
                      ) const;

public:
  bool on_beat(rational const &position) const
  { return no_remainder(position, state.beat); }
  void recurse( ast::partial_voice::iterator const &iterator
              , value_proxy *stack_end
              , rational const &max_duration
              , rational const &position
              ) const
  {
    if (iterator == voice_end) {
      if (not (last_partial_measure and state.exact_match_found
               and
               static_cast<bool>(max_duration)))
        yield(stack_begin.get(), stack_end, position - start_position);
    } else {
      ast::partial_voice::iterator tail;
      if (on_beat(position) and
          (tail = notegroup_end(iterator, voice_end)) > iterator) {
        // Try all possible notegroups, starting with the longest
        while (std::distance(iterator, tail) >= 3) {
          notegroup const group(iterator, tail, stack_end);
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

        if (stack_begin.get() == stack_end and position == 0 and
            state.time_signature != 1 and
            apply_visitor(maybe_whole_measure_rest(), *iterator)) {
          *stack_end = value_proxy(boost::get<ast::rest&>(*iterator), state.time_signature);
          recurse(iterator + 1, stack_end + 1,
                  zero, position + state.time_signature);
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
  { recurse(voice.begin(), stack_begin.get(), max_duration, start_position); }
  rational const &last_measure_duration() const { return state.last_measure_duration; }
};

class large_and_small_visitor : public boost::static_visitor<bool>
{
  ast::partial_voice::iterator const &rest;
  value_proxy *const proxy;
  rational const &max_duration
               , &position
               ;
  partial_voice_interpreter const &interpreter;
public:
  large_and_small_visitor( ast::partial_voice::iterator const &rest
                         , value_proxy *stack_end
                         , rational const &max_duration
                         , rational const &position
                         , partial_voice_interpreter const &interpreter
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
  result_type operator()(braille::ast::tie &) const { return false; }
  result_type operator()(ast::barline &) const { return false; }
  result_type operator()(ast::simile &simile) const
  {
    if (not position) { // full measure simile
      BOOST_ASSERT(static_cast<bool>(interpreter.last_measure_duration()));
      if (*new(proxy)value_proxy
          (simile, interpreter.last_measure_duration()) > rational(0) and
          static_cast<rational>(*proxy) / simile.count <= max_duration) {
        rational const duration(static_cast<rational>(*proxy) / simile.count);
        interpreter.recurse( rest, proxy + 1
                           , max_duration - duration, position + duration
                           );
      }
    } else { // partial measure simile
      if (interpreter.on_beat(position)) {
        if (*new(proxy)value_proxy(simile, position) <= max_duration)
          interpreter.recurse( rest, proxy + 1
                             , max_duration - *proxy, position + *proxy
                             );
      }
    }
    return true;
  }

  bool is_grace(ast::note const &note) const
  {
    return std::find_if(note.articulations.begin(), note.articulations.end(),
                        [](articulation a) {
                          return a == appoggiatura || a ==  short_appoggiatura;
                        }) != note.articulations.end();
  }
  bool is_grace(ast::rest const &) const { return false; }
  bool is_grace(ast::chord const &chord) const { return is_grace(chord.base); }
  bool is_grace(ast::moving_note const &chord) const { return is_grace(chord.base); }
};

inline
void
partial_voice_interpreter
::
large_and_small( ast::partial_voice::iterator const &iterator
               , value_proxy *stack_end
               , rational const &max_duration
               , rational const &position
               ) const
{
  // Skip this sign if it does not result in at least one possible proxy
  auto rest = iterator; ++rest;
  if (not apply_visitor( large_and_small_visitor
                         (rest, stack_end, max_duration, position, *this)
                       , *iterator
                       )
     )
    recurse(rest, stack_end, max_duration, position);
}

}

void
proxied_partial_voice::foreach( ast::partial_voice &voice
                              , rational const &max_duration
                              , rational const &position
                              , bool last_partial_measure
                              , global_state const &state
                              , proxied_partial_voice::function const &yield
                              )
{
  partial_voice_interpreter
  (voice, position, last_partial_measure, state, yield)
  (max_duration);
}

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
                               , proxied_partial_measure::function const &yield
                               )
{
  if (begin == end) {
    if (stack_begin != stack_end) yield(stack_begin, stack_end);
  } else {
    auto const tail = begin + 1;
    if (stack_begin == stack_end) {
      proxied_partial_voice::foreach
      ( *begin, length, position, last_partial_measure, state
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
      );
    } else {
      proxied_partial_voice::foreach
      ( *begin, length, position, last_partial_measure, state
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
      );
    }
  }
}

void
proxied_partial_measure::foreach( ast::partial_measure &partial_measure
                                , rational const &max_length
                                , rational const &position
                                , bool last_partial_measure
                                , global_state const &state
                                , proxied_partial_measure::function const &callback
                                )
{
  std::unique_ptr<proxied_partial_voice::shared_ptr[]>
  stack(new proxied_partial_voice::shared_ptr[partial_measure.size()]);
  partial_measure_interpretations( partial_measure.begin()
                                 , partial_measure.end()
                                 , stack.get()
                                 , stack.get()
                                 , max_length, position, last_partial_measure
                                 , state
                                 , callback
                                 ) ;
}

inline
void
voice_interpretations( ast::voice::iterator const &begin
                     , ast::voice::iterator const &end
                     , proxied_partial_measure::shared_ptr *stack_begin
                     , proxied_partial_measure::shared_ptr *stack_end
                     , rational const &max_length
                     , rational const &position
                     , global_state const &state
                     , proxied_voice::function const &yield
                     )
{
  if (begin == end) {
    if (stack_begin != stack_end) {
      yield(stack_begin, stack_end, position);
    }
  } else {
    auto const tail = begin + 1;
    proxied_partial_measure::foreach
    ( *begin, max_length, position, tail == end, state
    , [ stack_begin, stack_end, &tail, &end
      , &max_length, &position, &state
      , &yield
      ]
      ( proxied_partial_voice::shared_ptr const *f
      , proxied_partial_voice::shared_ptr const *l
      ) {
        stack_end->reset(new proxied_partial_measure(f, l));
        rational const partial_measure_duration(duration(*stack_end));
        voice_interpretations( tail, end, stack_begin, stack_end + 1
                             , max_length - partial_measure_duration
                             , position + partial_measure_duration
                             , state
                             , yield
                             );
      }
    );
  }
}

void
proxied_voice::foreach( ast::voice &voice
                      , rational const &max_length
                      , global_state const &state
                      , proxied_voice::function const &callback
                      )
{
  std::unique_ptr<proxied_partial_measure::shared_ptr[]>
  stack(new proxied_partial_measure::shared_ptr[voice.size()]);
  voice_interpretations( voice.begin(), voice.end()
                       , stack.get(), stack.get()
                       , max_length, zero
                       , state
                       , callback
                       ) ;
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
    auto const tail = begin + 1;
    proxied_voice::foreach
    ( *begin, length, *this
    , [ stack_begin, stack_end, &tail, &end, &length, this ]
      ( proxied_partial_measure::shared_ptr const *f
      , proxied_partial_measure::shared_ptr const *l
      , rational const &duration
      ) {
        if ((stack_begin == stack_end and not this->exact_match_found) or
            (duration == length)) {
          stack_end->reset(new proxied_voice(f, l, duration));
          this->recurse(tail, end, stack_begin, stack_end + 1, duration);
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

