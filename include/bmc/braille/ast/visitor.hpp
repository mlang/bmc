// Copyright (C) 2015  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AST_VISITOR_HPP
#define BMC_AST_VISITOR_HPP

#include <bmc/braille/ast/ast.hpp>
#include <boost/variant/static_visitor.hpp>

namespace bmc { namespace braille { namespace ast {

template<class Derived>
class visitor : public boost::static_visitor<void> {
  Derived &derived() { return *static_cast<Derived *>(this); }
public:
  using base_type = visitor<Derived>;

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

  void operator()(ast::score const &s) {
    if (derived().visit_score(s)) {
      visit_all(s.parts);
    }
  }
  void operator()(ast::score &s) {
    if (derived().visit_score(s)) {
      visit_all(s.parts);
    }
  }

  void operator()(std::vector<ast::section> const &p) {
    if (derived().visit_part(p)) visit_all(p);
  }
  void operator()(std::vector<ast::section> &p) {
    if (derived().visit_part(p)) visit_all(p);
  }

  void operator()(ast::section const &s) {
    if (derived().visit_section(s)) visit_all(s.paragraphs);
  }
  void operator()(ast::section &s) {
    if (derived().visit_section(s)) visit_all(s.paragraphs);
  }

  void operator()(ast::paragraph const &p) {
    if (derived().visit_paragraph(p)) visit_all(p);
  }
  void operator()(ast::paragraph &p) {
    if (derived().visit_paragraph(p)) visit_all(p);
  }

  void operator()(ast::paragraph_element const &pe) {
    if (derived().visit_paragraph_element(pe)) apply_visitor(derived(), pe);
  }
  void operator()(ast::paragraph_element &pe) {
    if (derived().visit_paragraph_element(pe)) apply_visitor(derived(), pe);
  }

  void operator()(ast::measure const &m) {
    if (derived().visit_measure(m)) visit_all(m.voices);
  }
  void operator()(ast::measure &m) {
    if (derived().visit_measure(m)) visit_all(m.voices);
  }

  void operator()(ast::voice const &v) {
    if (derived().visit_voice(v)) visit_all(v);
  }
  void operator()(ast::voice &v) {
    if (derived().visit_voice(v)) visit_all(v);
  }

  void operator()(ast::partial_measure const &pm) {
    if (derived().visit_partial_measure(pm)) {
      visit_all(pm);
      derived().partial_measure_visited(pm);
    }
  }
  void operator()(ast::partial_measure &pm) {
    if (derived().visit_partial_measure(pm)) {
      visit_all(pm);
      derived().partial_measure_visited(pm);
    }
  }

  void operator()(ast::partial_voice const &pv) {
    if (derived().visit_partial_voice(pv)) visit_all(pv);
  }
  void operator()(ast::partial_voice &pv) {
    if (derived().visit_partial_voice(pv)) visit_all(pv);
  }

  void operator()(ast::sign const &s) {
    if (derived().visit_sign(s)) apply_visitor(derived(), s);
  }
  void operator()(ast::sign &s) {
    if (derived().visit_sign(s)) apply_visitor(derived(), s);
  }

  void operator()(ast::note const &n) {
    if (derived().visit_note(n)) {
      derived().rhythmic(static_cast<ast::rhythmic const &>(n));
      derived().pitched(static_cast<ast::pitched const &>(n));
    }
  }
  void operator()(ast::note &n) {
    if (derived().visit_note(n)) {
      derived().rhythmic(static_cast<ast::rhythmic &>(n));
      derived().pitched(static_cast<ast::pitched &>(n));
    }
  }

  void operator()(ast::rest const &r) {
    if (derived().visit_rest(r)) {
      derived().rhythmic(static_cast<ast::rhythmic const &>(r));
    }
  }
  void operator()(ast::rest &r) {
    if (derived().visit_rest(r)) {
      derived().rhythmic(static_cast<ast::rhythmic &>(r));
    }
  }

  void operator()(ast::chord const &c) {
    if (derived().visit_chord(c)) {
      if (derived().visit_chord_base(c.base)) derived()(c.base);
      for (auto &i: c.intervals)
        if (derived().visit_chord_interval(i)) derived()(i);
    }
  }
  void operator()(ast::chord &c) {
    if (derived().visit_chord(c)) {
      if (derived().visit_chord_base(c.base)) derived()(c.base);
      for (auto &i: c.intervals)
        if (derived().visit_chord_interval(i)) derived()(i);
    }
  }

  void operator()(ast::interval const &i) {
    if (derived().visit_interval(i)) {
      derived().pitched(static_cast<ast::pitched const &>(i));
    }
  }
  void operator()(ast::interval &i) {
    if (derived().visit_interval(i)) {
      derived().pitched(static_cast<ast::pitched const &>(i));
    }
  }

  void operator()(ast::moving_note const &mn) {
    if (derived().visit_moving_note(mn)) {
      if (derived().visit_moving_note_base(mn.base)) derived()(mn.base);
      for (auto &mni: mn.intervals)
        if (derived().visit_moving_note_interval(mni)) derived()(mni);
    }
  }
  void operator()(ast::moving_note &mn) {
    if (derived().visit_moving_note(mn)) {
      if (derived().visit_moving_note_base(mn.base)) derived()(mn.base);
      for (auto &mni: mn.intervals)
        if (derived().visit_moving_note_interval(mni)) derived()(mni);
    }
  }

  void operator()(ast::value_distinction const &vd) {
    derived().value_distinction(vd);
  }
  void operator()(ast::value_distinction &vd) {
    derived().value_distinction(vd);
  }

  void operator()(ast::hyphen const &h) {
    derived().hyphen(h);
  }
  void operator()(ast::hyphen &h) {
    derived().hyphen(h);
  }

  void operator()(ast::tie const &) {}
  void operator()(ast::tie &) {}

  void operator()(ast::tuplet_start const &) {}
  void operator()(ast::tuplet_start &) {}

  void operator()(hand_sign const &) {}
  void operator()(hand_sign &) {}

  void operator()(ast::clef const &) {}
  void operator()(ast::clef &) {}

  void operator()(ast::simile const &) {}
  void operator()(ast::simile &) {}

  void operator()(ast::barline const &) {}
  void operator()(ast::barline &) {}

  void operator()(ast::key_and_time_signature const &kt) {}
  void operator()(ast::key_and_time_signature &kt) {}
};

}}}

#endif

