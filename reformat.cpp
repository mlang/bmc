#include <bmc/braille/reformat.hpp>

#include <bmc/braille/linebreaking.hpp>
#include <bmc/braille/ast/visitor.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace bmc {
namespace braille {

namespace {

output::fragment const guide_dot{U"\u2804", "guide dot"};

struct atom: public linebreaking::box {
  output content;

  atom(output const &o) {
    content.fragments.assign(o.fragments.begin(), o.fragments.end());
  }
  atom(std::initializer_list<output::fragment> c) {
    content.fragments.assign(c.begin(), c.end());
  }

  int width() const override {
    int result = 0;
    for (auto &&c: content.fragments) result += c.unicode.length();
    return result;
  }

  bool is_guide() const override { return false; }

  bool starts_with_one_of_dots_123() const {
    if (not content.fragments.empty()) {
      auto const &u = content.fragments.front().unicode;
      if (not u.empty()) {
        if (((u[0] | 0X28FF) == 0X28FF) and (u[0] & 7)) return true;
      }
    }
    return false;
  }
};

struct guide: public atom {
  guide(): atom{guide_dot} {}

  bool is_guide() const override { return true; }
};

struct whitespace: public linebreaking::glue {
  int width() const override { return 1; }
};

struct newline_opportunity: public linebreaking::penalty {
  bool hyphen;

  newline_opportunity(bool hyphen): hyphen{hyphen} {}

  int width() const override { return hyphen? 1: 0; }
  int value() const override { return hyphen ? 10: 4; }

  friend std::ostream &operator<<(std::ostream &os,
                                  newline_opportunity const &nl) {
    if (nl.hyphen == 1) os << u8"\u2810";
    os << std::endl;

    return os;
  }
};

struct eop: public newline_opportunity {
  eop(): newline_opportunity{false} {}

  int value() const override { return -linebreaking::infinity; }
};

output::fragment const newline{U"\n", "new line character"};
output::fragment const indent_2{U"  ", "indent of two spaces"};
output::fragment const number_sign{U"\u283C", "number sign"};
output::fragment const rest_sign[] = {
  {U"\u280D", ""}, {U"\u2825", ""}, {U"\u2827", ""}, {U"\u282D", ""}};
output::fragment const note_sign[][steps_per_octave] = {{{U"\u283D", ""},
                                                         {U"\u2835", ""},
                                                         {U"\u282F", ""},
                                                         {U"\u283F", ""},
                                                         {U"\u2837", ""},
                                                         {U"\u282E", ""},
                                                         {U"\u283E", ""}},
                                                        {{U"\u281D", ""},
                                                         {U"\u2815", ""},
                                                         {U"\u280F", ""},
                                                         {U"\u281F", ""},
                                                         {U"\u2817", ""},
                                                         {U"\u280E", ""},
                                                         {U"\u281E", ""}},
                                                        {{U"\u2839", ""},
                                                         {U"\u2831", ""},
                                                         {U"\u282B", ""},
                                                         {U"\u283B", ""},
                                                         {U"\u2833", ""},
                                                         {U"\u282A", ""},
                                                         {U"\u283A", ""}},
                                                        {{U"\u2819", ""},
                                                         {U"\u2811", ""},
                                                         {U"\u280B", ""},
                                                         {U"\u281B", ""},
                                                         {U"\u2813", ""},
                                                         {U"\u280A", ""},
                                                         {U"\u281A", ""}}};
output::fragment octave_sign[] = {
  {U"\u2808\u2808", "subcontra octave sign"},
  {U"\u2808", ""},
  {U"\u2818", ""},
  {U"\u2838", ""},
  {U"\u2810", ""},
  {U"\u2828", ""},
  {U"\u2830", ""},
  {U"\u2820", ""},
  {U"\u2820\u2820", ""},
};
output::fragment const augmentation_dot{U"\u2804", "augmentation dot"};
output::fragment const finger_sign[] = {
  {U"\u2801", ""},
  {U"\u2803", ""},
  {U"\u2807", ""},
  {U"\u2802", ""},
  {U"\u2805", ""},
};
output::fragment const partial_voice_separator{U"\u2810\u2802", ""};
output::fragment const partial_measure_separator{U"\u2828\u2805", ""};
output::fragment const voice_separator{U"\u2823\u281C", ""};
output::fragment const moving_note_separator {U"\u2820", "moving note separator"};
output::fragment const slur_sign{U"\u2809", ""};
output::fragment const simile_sign {U"\u2836", "simile"};
output::fragment const eom_sign{U"\u2823\u2805", ""};
output::fragment const hyphen_sign{U"\u2810", "hyphen"};
output::fragment const begin_repeat_sign { U"\u2823\u2836", "begin repeat" };
output::fragment const end_repeat_sign { U"\u2823\u2806", "end repeat" };
output::fragment const hand_sign[] = {
  {U"\u2828\u281C", "right hand sign", true},
  {U"\u2838\u281C", "left hand sign", true}
};
output::fragment const flat_sign {U"\u2823", "flat"};
output::fragment const sharp_sign {U"\u2829", "sharp"};
output::fragment const upper_digit_sign[10] = {
  {U"\u281A", "zero" },
  {U"\u2801", "one"},
  {U"\u2803", "two"},
  {U"\u2809", "three"},
  {U"\u2819", "four"},
  {U"\u2811", "five"},
  {U"\u280B", "six"},
  {U"\u281A", "seven"},
  {U"\u2813", "eight"},
  {U"\u280A", "nine"}
};
output::fragment const lower_digit_sign[10] = {
  {U"\u2834", "lower zero"},
  {U"\u2802", "lower one"},
  {U"\u2806", "lower two"},
  {U"\u2812", "lower three"},
  {U"\u2832", "lower four"},
  {U"\u2822", "lower five"},
  {U"\u2816", "lower six"},
  {U"\u2836", "lower seven"},
  {U"\u2826", "lower eight"},
  {U"\u2814", "lower nine"}
};
output::fragment const interval_sign[] = {
  {U"\u280C", "second"},
  {U"\u282C", "third"},
  {U"\u283C", "fourth"},
  {U"\u2814", "fifth"},
  {U"\u2834", "sixth"},
  {U"\u2812", "seventh"},
  {U"\u2824", "octave"}
};

std::size_t length(output const &o) {
  std::size_t len = 0;
  for (auto &&fragment: o.fragments)
    len += std::u32string{fragment.unicode}.length();
  return len;
}

class fingering_print_visitor: public boost::static_visitor<void> {
  output &out;

public:
  fingering_print_visitor(output &out): out{out} {}

  result_type operator()(unsigned finger) const {
    out.fragments.push_back(finger_sign[finger - 1]);
  }
  result_type operator()(finger_change const &change) const {
    (*this)(change.first);
    out.fragments.push_back(slur_sign);
    (*this)(change.second);
  }
};

struct print_visitor: public ast::const_visitor<print_visitor> {
  output result;

  print_visitor(format_style const &style): style{style} {}

  bool visit_score(ast::score const &s) {
    std::fill_n(std::back_inserter(result.fragments),
                std::abs(s.key_sig), s.key_sig < 0? flat_sign: sharp_sign);
    if (not s.time_sigs.empty()) {
      result.fragments.push_back(number_sign);
      result.fragments.push_back(upper_digit_sign[s.time_sigs.front().numerator()]);
      result.fragments.push_back(lower_digit_sign[s.time_sigs.front().denominator()]);
    }
    result.fragments.push_back(newline);

    return true;
  }

  bool visit_part(ast::part const &p) {
    section_n = p.size();

    return true;
  }

  bool visit_section(ast::section const &s) {
    last_section = !--section_n;
    staves = s.paragraphs.size();
    staff = 0;
    return true;
  }

  bool visit_paragraph(ast::paragraph const &)
  {
    add_to_para(new atom{indent_2});

    if (staves > 1) {
      add_to_para(new atom{hand_sign[staff]});
    }
    return true;
  }

  bool end_of_paragraph(ast::paragraph const &) {
    if (last_section) add_to_para(new atom{eom_sign});
    add_to_para(new eop{});

    auto const breaks = linebreaking::breakpoints(para, {style.columns});
    BOOST_ASSERT(not breaks.empty());
    auto i = para.begin();
    for (auto &&j: breaks) {
      while (i < j) {
        if (dynamic_cast<whitespace const *>(i->get())) {
          result.fragments.push_back(output::fragment{U" ", "space"});
        } else if (auto a = dynamic_cast<atom const *>(i->get())) {
          for (auto &&f: a->content.fragments) result.fragments.push_back(f);
        }
        ++i;
      }
      if (auto nl = dynamic_cast<newline_opportunity const *>(i->get())) {
        if (nl->hyphen) result.fragments.push_back(hyphen_sign);
        result.fragments.push_back(newline);
      } else if (dynamic_cast<whitespace const *>(i->get())) {
        result.fragments.push_back(newline);
        ++i;
      }
    }
    while (i < para.end()) {
      if (dynamic_cast<whitespace const *>(i->get())) {
        result.fragments.push_back(output::fragment{U" ", "space"});
      } else if (auto a = dynamic_cast<atom const *>(i->get())) {
        for (auto &&f: a->content.fragments) result.fragments.push_back(f);
      }
      ++i;
    }
    result.fragments.push_back(newline);

    para.clear();
    ++staff;

    return true;
  }

  bool between_paragraph_element(ast::paragraph_element const &,
                                 ast::paragraph_element const &) {
    add_to_para(new whitespace{});

    return true;
  }

  bool between_voice(ast::voice const &, ast::voice const &) {
    add_to_para(new atom{voice_separator});
    add_to_para(new newline_opportunity{false});

    return true;
  }

  bool between_partial_measure(ast::partial_measure const &,
                               ast::partial_measure const &) {
    add_to_para(new atom{partial_measure_separator});
    add_to_para(new newline_opportunity{false});

    return true;
  }

  bool between_partial_voice(ast::partial_voice const &,
                             ast::partial_voice const &) {
    add_to_para(new atom{partial_voice_separator});
    add_to_para(new newline_opportunity{false});

    return true;
  }

  bool between_sign(ast::sign const &, ast::sign const &) {
    if (not in_notegroup) add_to_para(new newline_opportunity{true});

    return true;
  }

  bool visit_note(ast::note const &n) {
    if (n.notegroup_member == ast::notegroup_member_type::begin or
        n.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else { in_notegroup = false; }

    output res;
    if (n.acc) {
      switch(*n.acc) {
      case flat: std::fill_n(std::back_inserter(res.fragments), 1, flat_sign); break;
      case double_flat: std::fill_n(std::back_inserter(res.fragments), 2, flat_sign); break;
      case sharp: std::fill_n(std::back_inserter(res.fragments), 1, sharp_sign); break;
      case double_sharp: std::fill_n(std::back_inserter(res.fragments), 2, sharp_sign); break;
      default: BOOST_ASSERT(false);
      }
    }
    if (n.octave_spec) res.fragments.push_back(octave_sign[*n.octave_spec - 1]);
    res.fragments.push_back(note_sign[n.ambiguous_value][n.step]);
    std::fill_n(std::back_inserter(res.fragments), n.dots, augmentation_dot);
    fingering_print_visitor fingering_printer{res};
    std::for_each(n.fingers.begin(), n.fingers.end(),
                  apply_visitor(fingering_printer));
    add_to_para(new atom{res});
    return true;
  }

  bool visit_rest(ast::rest const &r)
  {
    if (r.notegroup_member == ast::notegroup_member_type::begin or
        r.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else { in_notegroup = false; }

    output res;
    res.fragments.push_back(rest_sign[r.ambiguous_value]);
    std::fill_n(std::back_inserter(res.fragments), r.dots, augmentation_dot);

    add_to_para(new atom{res});

    return true;
  }

  bool visit_interval(ast::interval const &i)
  {
    output res;
    if (i.octave_spec) res.fragments.push_back(octave_sign[*i.octave_spec - 1]);
    res.fragments.push_back(interval_sign[i.steps - 1]);

    fingering_print_visitor fingering_printer{res};
    std::for_each(i.fingers.begin(), i.fingers.end(), apply_visitor(fingering_printer));
    add_to_para(new atom{res});

    return true;
  }

  bool between_moving_note_interval(ast::interval const &, ast::interval const &)
  {
    add_to_para(new atom{moving_note_separator});

    return true;
  }

  bool visit_simile(ast::simile const &s) {
    output res;
    std::fill_n(std::back_inserter(res.fragments), s.count, simile_sign);
    add_to_para(new atom{res});

    return true;
  }

  bool visit_barline(ast::barline const &b)
  {
    switch (b) {
    case ast::begin_repeat:
      add_to_para(new atom{begin_repeat_sign});
      break;
    case ast::end_repeat:
      add_to_para(new atom{end_repeat_sign});
      break;
    default: BOOST_ASSERT(false);
    }

    return true;
  }

private:
  void add_to_para(linebreaking::object *object) {
    if (auto a = dynamic_cast<atom *>(object)) {
      if (a->starts_with_one_of_dots_123()) {
        atom *box = nullptr;
        if (not para.empty()) {
          box = dynamic_cast<atom *>(para.back().get());
          if (not box and dynamic_cast<newline_opportunity *>(para.back().get())) {
            box = dynamic_cast<atom *>(para[para.size() - 2].get());
          }
        }
        if (box and box->content.fragments.back().needs_guide_dot)
          para.emplace_back(new guide{});
      }
    }
    para.emplace_back(object);
  }

  format_style const &style;
  int section_n;
  bool last_section;
  size_t staves;
  size_t staff;
  bool in_notegroup;
  linebreaking::objects para;
};
}

output reformat(ast::score const &score, format_style const &style) {
  print_visitor printer{style};
  bool const ok = printer.traverse_score(score);
  BOOST_ASSERT(ok);
  return printer.result;
}

std::ostream &operator<<(std::ostream &os, output const &out) {
  for (auto &&f: out.fragments)
    os << boost::locale::conv::utf_to_utf<char>(f.unicode);

  return os;
}
}
}
