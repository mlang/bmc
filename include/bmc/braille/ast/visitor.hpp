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
class visitor_base : public boost::static_visitor<void> {
  Derived &derived() { return *static_cast<Derived *>(this); }
public:
  using base_type = visitor_base<Ref, Derived>;

  bool visit_score(ast::score const &) { return true; }
  bool visit_part(std::vector<ast::section> const &) { return true; }
  bool visit_section(ast::section const &) { return true; }
  bool visit_paragraph(ast::paragraph const &) { return true; }
  bool visit_paragraph_element(ast::paragraph_element const &) { return true; }
  bool visit_measure(ast::measure const &) { return true; }
  bool visit_voice(ast::voice const &) { return true; }
  bool visit_partial_measure(ast::partial_measure const &) { return true; }
  void partial_measure_visited(ast::partial_measure const &) {}
  bool visit_partial_voice(ast::partial_voice const &) { return true; }
  bool visit_sign(ast::sign const &) { return true; }
  bool visit_note(ast::note const &) { return true; }
  void rhythmic(ast::rhythmic const &) {}
  void pitched(ast::pitched const &) {}
  bool visit_rest(ast::rest const &) { return true; }
  bool visit_chord(ast::chord const &) { return true; }
  bool visit_chord_base(ast::note const &) { return true; }
  bool visit_chord_interval(ast::interval const &) { return true; }
  bool visit_moving_note(ast::moving_note const &) { return true; }
  bool visit_moving_note_base(ast::note const &) { return true; }
  bool visit_moving_note_interval(ast::interval const &) { return true; }
  bool visit_interval(ast::interval const &) { return true; }
  void value_distinction(ast::value_distinction const &) {}
  void hyphen(ast::hyphen const &) {}


  template<typename T> void visit_all(T &t) { for (auto &e: t) derived()(e); }

  void operator()(Ref<ast::score> s) {
    if (derived().visit_score(s)) {
      visit_all(s.parts);
    }
  }

  void operator()(Ref<std::vector<ast::section>> p) {
    if (derived().visit_part(p)) visit_all(p);
  }

  void operator()(Ref<ast::section> s) {
    if (derived().visit_section(s)) visit_all(s.paragraphs);
  }

  void operator()(Ref<ast::paragraph> p) {
    if (derived().visit_paragraph(p)) visit_all(p);
  }

  void operator()(Ref<ast::paragraph_element> pe) {
    if (derived().visit_paragraph_element(pe)) apply_visitor(derived(), pe);
  }

  void operator()(Ref<ast::measure> m) {
    if (derived().visit_measure(m)) visit_all(m.voices);
  }

  void operator()(Ref<ast::voice> v) {
    if (derived().visit_voice(v)) visit_all(v);
  }

  void operator()(Ref<ast::partial_measure> pm) {
    if (derived().visit_partial_measure(pm)) {
      visit_all(pm);
      derived().partial_measure_visited(pm);
    }
  }

  void operator()(Ref<ast::partial_voice> pv) {
    if (derived().visit_partial_voice(pv)) visit_all(pv);
  }

  void operator()(Ref<ast::sign> s) {
    if (derived().visit_sign(s)) apply_visitor(derived(), s);
  }

  void operator()(Ref<ast::note> n) {
    if (derived().visit_note(n)) {
      derived().rhythmic(static_cast<Ref<ast::rhythmic>>(n));
      derived().pitched(static_cast<Ref<ast::pitched>>(n));
    }
  }

  void operator()(Ref<ast::rest> r) {
    if (derived().visit_rest(r)) {
      derived().rhythmic(static_cast<Ref<ast::rhythmic>>(r));
    }
  }

  void operator()(Ref<ast::chord> c) {
    if (derived().visit_chord(c)) {
      if (derived().visit_chord_base(c.base)) derived()(c.base);
      for (auto &i: c.intervals)
        if (derived().visit_chord_interval(i)) derived()(i);
    }
  }

  void operator()(Ref<ast::interval> i) {
    if (derived().visit_interval(i)) {
      derived().pitched(static_cast<Ref<ast::pitched>>(i));
    }
  }

  void operator()(Ref<ast::moving_note> mn) {
    if (derived().visit_moving_note(mn)) {
      if (derived().visit_moving_note_base(mn.base)) derived()(mn.base);
      for (auto &mni: mn.intervals)
        if (derived().visit_moving_note_interval(mni)) derived()(mni);
    }
  }

  void operator()(Ref<ast::value_distinction> vd) {
    derived().value_distinction(vd);
  }

  void operator()(Ref<ast::hyphen> h) {
    derived().hyphen(h);
  }

  void operator()(Ref<ast::tie>) {}
  void operator()(Ref<ast::tuplet_start>) {}
  void operator()(Ref<hand_sign>) {}
  void operator()(Ref<ast::clef>) {}
  void operator()(Ref<ast::simile>) {}
  void operator()(Ref<ast::barline>) {}
  void operator()(Ref<ast::key_and_time_signature>) {}
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

