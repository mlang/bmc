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

template<template<typename> class Ref, class Derived>
class visitor_base {
  Derived const &derived() const { return *static_cast<Derived const *>(this); }
  Derived &derived() { return *static_cast<Derived *>(this); }
public:
  using base_type = visitor_base<Ref, Derived>;
  visitor_base()
  : paragraph_element_visitor{derived()}, sign_visitor{derived()} {}
  visitor_base(visitor_base<Ref, Derived> const &other)
  : paragraph_element_visitor{derived()}, sign_visitor{derived()} {
  }

  template<typename T> void visit_all(T &t) { for (auto &e: t) derived()(e); }

  template<typename Container>
  bool all_of(Container &c, bool (Derived::*fn)(Ref<typename Container::value_type>)) {
    for (auto &e: c) if (not (derived().*fn)(e)) return false;
    return true;
  }

#define SIMPLE_CONTAINER(NAME, CLASS, VAR, ACCESSOR, ELEMENT_NAME) \
  bool traverse_##NAME(Ref<CLASS> VAR) {                                       \
    return derived().walk_up_from_##NAME(VAR) and                              \
           all_of(ACCESSOR, &Derived::traverse_##ELEMENT_NAME) and \
           derived().end_of_##NAME(VAR); \
  }                                                                            \
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {                                   \
    return derived().visit_##NAME(VAR);                                        \
  }                                                                            \
  bool visit_##NAME(Ref<CLASS>) { return true; } \
  bool end_of_##NAME(Ref<CLASS>) { return true; }
#define SIMPLE_VARIANT(NAME, CLASS, VAR, VISITOR) \
  bool traverse_##NAME(Ref<CLASS> VAR) { \
    return derived().walk_up_from_##NAME(VAR) and apply_visitor(derived().VISITOR, VAR); \
  }\
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {\
    return derived().visit_##NAME(VAR);\
  }\
  bool visit_##NAME(Ref<CLASS>) { return true; }
#define SIMPLE_BASE(NAME, CLASS, VAR) \
  bool traverse_##NAME(Ref<CLASS> VAR) {\
    return derived().walk_up_from_##NAME(VAR);\
  }\
  bool walk_up_from_##NAME(Ref<CLASS> VAR) {\
    return derived().visit_##NAME(VAR);\
  }\
  bool visit_##NAME(Ref<CLASS>) { return true; }

  SIMPLE_CONTAINER(score, ast::score, s, s.parts, part)
  SIMPLE_CONTAINER(part, ast::part, p, p, section)
  SIMPLE_CONTAINER(section, ast::section, s, s.paragraphs, paragraph)
  SIMPLE_CONTAINER(paragraph, ast::paragraph, p, p, paragraph_element)
  SIMPLE_VARIANT(paragraph_element, ast::paragraph_element, pe,
                 paragraph_element_visitor)
  SIMPLE_CONTAINER(measure, ast::measure, m, m.voices, voice)
  SIMPLE_CONTAINER(voice, ast::voice, v, v, partial_measure)
  SIMPLE_CONTAINER(partial_measure, ast::partial_measure, pm, pm, partial_voice)
  SIMPLE_CONTAINER(partial_voice, ast::partial_voice, pv, pv, sign)
  SIMPLE_VARIANT(sign, ast::sign, s, sign_visitor)

  bool traverse_note(Ref<ast::note> n) {
    return derived().walk_up_from_note(n);
  }
  bool walk_up_from_note(Ref<ast::note> n) {
    return derived().walk_up_from_rhythmic(static_cast<Ref<ast::rhythmic>>(n)) and
           derived().walk_up_from_pitched(static_cast<Ref<ast::pitched>>(n)) and
           derived().visit_note(n);
  }
  bool visit_note(Ref<ast::note>) { return true; }

  bool traverse_rest(Ref<ast::rest> r) {
    return derived().walk_up_from_rest(r);
  }
  bool walk_up_from_rest(Ref<ast::rest> r) {
    return derived().walk_up_from_rhythmic(static_cast<Ref<ast::rhythmic>>(r)) and
           derived().visit_rest(r);
  }
  bool visit_rest(Ref<ast::rest>) { return true; }

  SIMPLE_BASE(rhythmic, ast::rhythmic, r)
  SIMPLE_BASE(pitched, ast::pitched, p)

  bool traverse_chord(Ref<ast::chord> c) {
    return derived().walk_up_from_chord(c) and
           traverse_note(c.base) and
           all_of(c.intervals, &Derived::traverse_interval) and
           derived().end_of_chord(c);
  }
  bool walk_up_from_chord(Ref<ast::chord> c) {
    return derived().visit_chord(c);
  }
  bool visit_chord(Ref<ast::chord>) { return true; }
  bool end_of_chord(Ref<ast::chord>) { return true; }

  bool traverse_moving_note(Ref<ast::moving_note> mn) {
    return derived().walk_up_from_moving_note(mn) and
           derived().traverse_note(mn.base) and
           all_of(mn.intervals, &Derived::traverse_interval) and
           derived().end_of_moving_note(mn);
  }
  bool walk_up_from_moving_note(Ref<ast::moving_note> mn) {
    return derived().visit_moving_note(mn);
  }
  bool visit_moving_note(Ref<ast::moving_note>) { return true; }
  bool end_of_moving_note(Ref<ast::moving_note>) { return true; }

  SIMPLE_BASE(interval, ast::interval, i)

  SIMPLE_BASE(value_distinction, ast::value_distinction, v)
  SIMPLE_BASE(hyphen, ast::hyphen, h)
  SIMPLE_BASE(tie, ast::tie, t)
  SIMPLE_BASE(tuplet_start, ast::tuplet_start, t)
  SIMPLE_BASE(clef, ast::clef, c)
  SIMPLE_BASE(simile, ast::simile, s)
  SIMPLE_BASE(hand_sign, hand_sign, h)
  SIMPLE_BASE(barline, ast::barline, b)

  bool traverse_key_and_time_signature(Ref<ast::key_and_time_signature> kt) {
    return derived().walk_up_from_key_and_time_signature(kt);
  }
  bool walk_up_from_key_and_time_signature(Ref<ast::key_and_time_signature> kt) {
    return derived().visit_key_and_time_signature(kt);
  }
  bool visit_key_and_time_signature(Ref<ast::key_and_time_signature> kt) { return true; }

#define BEGIN_STATIC_VISITOR(TYPE) \
  class TYPE : public boost::static_visitor<bool> { \
    Derived &derived;\
  public:\
    TYPE(Derived &derived): derived{derived} {}
#define CALL_OPERATOR(CLASS, NAME) \
    result_type \
    operator()(Ref<CLASS> arg) const { return derived.traverse_##NAME(arg); }
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
    CALL_OPERATOR(ast::value_distinction, value_distinction)
    CALL_OPERATOR(ast::hyphen, hyphen)
    CALL_OPERATOR(ast::tie, tie)
    CALL_OPERATOR(ast::tuplet_start, tuplet_start)
    CALL_OPERATOR(ast::clef, clef)
    CALL_OPERATOR(ast::simile, simile)
    CALL_OPERATOR(hand_sign, hand_sign)
    CALL_OPERATOR(ast::barline, barline)
  END_STATIC_VISITOR(sign_visitor)
#undef BEGIN_STATIC_VISITOR
#undef CALL_OPERATOR
#undef END_STATIC_VISITOR
};

template<typename T>
using make_ref = typename std::add_lvalue_reference<T>::type;
template<typename T>
using make_const_ref = typename std::add_lvalue_reference<typename std::add_const<T>::type>::type;

template<class Derived>
using visitor = visitor_base<make_ref, Derived>;
template<class Derived>
using const_visitor = visitor_base<make_const_ref, Derived>;

}}}

#endif

