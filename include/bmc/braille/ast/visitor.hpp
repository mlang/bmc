// Copyright (C) 2015  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AST_VISITOR_HPP
#define BMC_AST_VISITOR_HPP

#include <bmc/braille/ast/ast.hpp>
#include <boost/variant/static_visitor.hpp>
#include <type_traits>

namespace bmc { namespace braille { namespace ast {

template <template <typename> class Ref, class Derived> class visitor_base {
  Derived const &derived() const { return *static_cast<Derived const *>(this); }
  Derived &derived() { return *static_cast<Derived *>(this); }

public:
  using base_type = visitor_base<Ref, Derived>;
  visitor_base()
  : paragraph_element_visitor{derived()}, sign_visitor{derived()} {}
  visitor_base(visitor_base<Ref, Derived> const &other)
  : paragraph_element_visitor{derived()}, sign_visitor{derived()} {}

  template <typename Container>
  bool all_of(Container &c,
              bool (Derived::*fn)(Ref<typename Container::value_type>))
  {
    for (auto &v: c) if (!(derived().*fn)(v)) return false;
    return true;
  }

  template <typename Container, typename Value = typename Container::value_type>
  bool all_of(Container &c,
              bool (Derived::*unary)(Ref<Value>),
              bool (Derived::*binary)(Ref<Value>, Ref<Value>))
  {
    auto lhs = c.begin(), end = c.end();
    if (lhs != end) {
      if (!(derived().*unary)(*lhs)) return false;
      for (auto rhs = std::next(lhs); rhs != end; lhs = rhs++)
        if (!((derived().*binary)(*lhs, *rhs) && (derived().*unary)(*rhs)))
          return false;
    }
    return true;
  }

#define SIMPLE_CONTAINER(NAME, CLASS, VAR, ACCESSOR, ELEMENT_NAME)             \
  bool traverse_##NAME(Ref<CLASS> VAR) {                                       \
    return derived().walk_up_from_##NAME(VAR) &&                            \
           all_of(ACCESSOR, &Derived::traverse_##ELEMENT_NAME, &Derived::between_##ELEMENT_NAME) && \
           derived().end_of_##NAME(VAR);                                       \
  }                                                                            \
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {                                   \
    return derived().visit_##NAME(VAR);                                        \
  }                                                                            \
  bool visit_##NAME(Ref<CLASS>) { return true; }                               \
  bool end_of_##NAME(Ref<CLASS>) { return true; }
#define LOCATABLE_CONTAINER(NAME, CLASS, VAR, ACCESSOR, ELEMENT_NAME)             \
  bool traverse_##NAME(Ref<CLASS> VAR) {                                       \
    return derived().walk_up_from_##NAME(VAR) &&                              \
           all_of(ACCESSOR, &Derived::traverse_##ELEMENT_NAME, &Derived::between_##ELEMENT_NAME) && \
           derived().end_of_##NAME(VAR);                                       \
  }                                                                            \
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {                                   \
    return derived().walk_up_from_locatable(                                   \
               static_cast<Ref<ast::locatable>>(VAR)) &&                      \
           derived().visit_##NAME(VAR);                                        \
  }                                                                            \
  bool visit_##NAME(Ref<CLASS>) { return true; }                               \
  bool end_of_##NAME(Ref<CLASS>) { return true; }
#define SIMPLE_VARIANT(NAME, CLASS, VAR, VISITOR)                              \
  bool traverse_##NAME(Ref<CLASS> VAR) {                                       \
    return derived().walk_up_from_##NAME(VAR) &&                               \
           apply_visitor(derived().VISITOR, VAR);                              \
  }                                                                            \
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {                                   \
    return derived().visit_##NAME(VAR);                                        \
  }                                                                            \
  bool visit_##NAME(Ref<CLASS>) { return true; }
#define SIMPLE_BASE(NAME, CLASS, VAR)                                          \
  bool traverse_##NAME(Ref<CLASS> VAR) {                                       \
    return derived().walk_up_from_##NAME(VAR);                                 \
  }                                                                            \
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {                                   \
    return derived().visit_##NAME(VAR);                                        \
  }                                                                            \
  bool visit_##NAME(Ref<CLASS>) { return true; }

  SIMPLE_CONTAINER(score, ast::score, s, s.parts, part)
  bool between_part(Ref<ast::part>, Ref<ast::part>) { return true; }
  SIMPLE_CONTAINER(part, ast::part, p, p, section)
  bool between_section(Ref<ast::section>, Ref<ast::section>) { return true; }
  SIMPLE_CONTAINER(section, ast::section, s, s.paragraphs, paragraph)
  bool between_paragraph(Ref<ast::paragraph>, Ref<ast::paragraph>) { return true; }
  SIMPLE_CONTAINER(paragraph, ast::paragraph, p, p, paragraph_element)
  bool between_paragraph_element(Ref<ast::paragraph_element>, Ref<ast::paragraph_element>) { return true; }
  SIMPLE_VARIANT(paragraph_element, ast::paragraph_element, pe,
                 paragraph_element_visitor)
  LOCATABLE_CONTAINER(measure, ast::measure, m, m.voices, voice)
  bool between_voice(Ref<ast::voice>, Ref<ast::voice>) { return true; }
  LOCATABLE_CONTAINER(voice, ast::voice, v, v, partial_measure)
  bool between_partial_measure(Ref<ast::partial_measure>, Ref<ast::partial_measure>) { return true; }
  LOCATABLE_CONTAINER(partial_measure, ast::partial_measure, pm, pm, partial_voice)
  bool between_partial_voice(Ref<ast::partial_voice>, Ref<ast::partial_voice>) { return true; }
  LOCATABLE_CONTAINER(partial_voice, ast::partial_voice, pv, pv, sign)
  bool between_sign(Ref<ast::sign>, Ref<ast::sign>) { return true; }
  SIMPLE_VARIANT(sign, ast::sign, s, sign_visitor)

  bool traverse_note(Ref<ast::note> n) {
    return derived().walk_up_from_note(n);
  }
  bool walk_up_from_note(Ref<ast::note> n) {
    return derived().walk_up_from_rhythmic(
               static_cast<Ref<ast::rhythmic>>(n)) &&
           derived().walk_up_from_pitched(static_cast<Ref<ast::pitched>>(n)) &&
           derived().walk_up_from_locatable(static_cast<Ref<ast::locatable>>(n)) &&
           derived().visit_note(n);
  }
  bool visit_note(Ref<ast::note>) { return true; }

  bool traverse_rest(Ref<ast::rest> r) {
    return derived().walk_up_from_rest(r);
  }
  bool walk_up_from_rest(Ref<ast::rest> r) {
    return derived().walk_up_from_rhythmic(
               static_cast<Ref<ast::rhythmic>>(r)) &&
           derived().walk_up_from_locatable(static_cast<Ref<ast::locatable>>(r)) &&
           derived().visit_rest(r);
  }
  bool visit_rest(Ref<ast::rest>) { return true; }

  SIMPLE_BASE(rhythmic, ast::rhythmic, r)
  SIMPLE_BASE(pitched, ast::pitched, p)
  SIMPLE_BASE(locatable, ast::locatable, l)

  bool traverse_chord(Ref<ast::chord> c) {
    return derived().walk_up_from_chord(c) && traverse_note(c.base) &&
           all_of(c.intervals, &Derived::traverse_interval) &&
           derived().end_of_chord(c);
  }
  bool walk_up_from_chord(Ref<ast::chord> c) {
    return derived().walk_up_from_locatable(static_cast<Ref<ast::locatable>>(c)) &&
           derived().visit_chord(c);
  }
  bool visit_chord(Ref<ast::chord>) { return true; }
  bool end_of_chord(Ref<ast::chord>) { return true; }

  bool traverse_moving_note(Ref<ast::moving_note> mn) {
    return derived().walk_up_from_moving_note(mn) &&
           derived().traverse_note(mn.base) &&
           all_of(mn.intervals, &Derived::traverse_interval, &Derived::between_moving_note_interval) &&
           derived().end_of_moving_note(mn);
  }
  bool walk_up_from_moving_note(Ref<ast::moving_note> mn) {
    return derived().walk_up_from_locatable(static_cast<Ref<ast::locatable>>(mn)) &&
           derived().visit_moving_note(mn);
  }
  bool visit_moving_note(Ref<ast::moving_note>) { return true; }
  bool between_moving_note_interval(Ref<ast::interval>, Ref<ast::interval>) { return true; }
  bool end_of_moving_note(Ref<ast::moving_note>) { return true; }

  SIMPLE_BASE(interval, ast::interval, i)

  SIMPLE_BASE(value_prefix, ast::value_prefix, v)
  SIMPLE_BASE(hyphen, ast::hyphen, h)
  SIMPLE_BASE(tie, ast::tie, t)
  SIMPLE_BASE(tuplet_start, ast::tuplet_start, t)
  SIMPLE_BASE(clef, ast::clef, c)
  SIMPLE_BASE(simile, ast::simile, s)
  SIMPLE_BASE(hand_sign, ast::hand_sign, h)
  SIMPLE_BASE(barline, ast::barline, b)

  bool traverse_key_and_time_signature(Ref<ast::key_and_time_signature> kt) {
    return derived().walk_up_from_key_and_time_signature(kt);
  }
  bool
  walk_up_from_key_and_time_signature(Ref<ast::key_and_time_signature> kt) {
    return derived().visit_key_and_time_signature(kt);
  }
  bool visit_key_and_time_signature(Ref<ast::key_and_time_signature> kt) {
    return true;
  }

#undef SIMPLE_CONTAINER
#undef SIMPLE_VARIANT
#undef SIMPLE_BASE

#define BEGIN_STATIC_VISITOR(TYPE)                                             \
  class TYPE : public boost::static_visitor<bool> {                            \
    Derived &derived;                                                          \
                                                                               \
  public:                                                                      \
    TYPE(Derived &derived) : derived{derived} {}
#define CALL_OPERATOR(CLASS, NAME)                                             \
  result_type operator()(Ref<CLASS> arg) const {                               \
    return derived.traverse_##NAME(arg);                                       \
  }
#define END_STATIC_VISITOR(NAME) } NAME;

  BEGIN_STATIC_VISITOR(paragraph_element_visitor_type)
  CALL_OPERATOR(ast::measure, measure)
  CALL_OPERATOR(ast::key_and_time_signature, key_and_time_signature)
  END_STATIC_VISITOR(paragraph_element_visitor)

  BEGIN_STATIC_VISITOR(sign_visitor_type)
  CALL_OPERATOR(ast::note, note)
  CALL_OPERATOR(ast::rest, rest)
  CALL_OPERATOR(ast::chord, chord)
  CALL_OPERATOR(ast::moving_note, moving_note)
  CALL_OPERATOR(ast::value_prefix, value_prefix)
  CALL_OPERATOR(ast::hyphen, hyphen)
  CALL_OPERATOR(ast::tie, tie)
  CALL_OPERATOR(ast::tuplet_start, tuplet_start)
  CALL_OPERATOR(ast::clef, clef)
  CALL_OPERATOR(ast::simile, simile)
  CALL_OPERATOR(ast::hand_sign, hand_sign)
  CALL_OPERATOR(ast::barline, barline)
  END_STATIC_VISITOR(sign_visitor)
#undef BEGIN_STATIC_VISITOR
#undef CALL_OPERATOR
#undef END_STATIC_VISITOR
};

template <typename T>
using make_ref = typename std::add_lvalue_reference<T>::type;
template <typename T>
using make_const_ref =
    typename std::add_lvalue_reference<typename std::add_const<T>::type>::type;

template <class Derived> using visitor = visitor_base<make_ref, Derived>;
template <class Derived>
using const_visitor = visitor_base<make_const_ref, Derived>;

}
}
}

#endif
