#include <bmc/braille/reformat.hpp>

#include <bmc/braille/linebreaking.hpp>
#include <bmc/braille/ast/visitor.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace bmc { namespace braille {

namespace {

template<typename T> std::vector<T> digits(T n, unsigned base = 10) {
  std::vector<T> result;
  while (n) {
    result.push_back(n % base);
    n /= base;
  }
  return {result.rbegin(), result.rend()};
}

inline std::u32string utf8toucs4(char const *utf8) {
  return boost::locale::conv::utf_to_utf<char32_t>(utf8);
}

output::fragment const guide_dot{utf8toucs4("\u2804"), "guide dot"};

struct atom: public linebreaking::box {
  output content;

  atom(output const &o) {
    content.fragments.assign(o.fragments.begin(), o.fragments.end());
  }
  atom(std::initializer_list<output::fragment> const& c) {
    content.fragments.assign(c.begin(), c.end());
  }

  int width() const override {
    return std::accumulate(content.fragments.begin(), content.fragments.end(),
                           0, [](int total, output::fragment const &fragment)
                              { return total + fragment.unicode.length(); });
  }

  bool is_guide() const override { return false; }

  bool starts_with_one_of_dots_123() const {
    if (!content.fragments.empty()) {
      auto const &u = content.fragments.front().unicode;
      if (!u.empty()) {
        return ((u[0] >> 8) == 0X28) && (u[0] & 7);
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
  int value() const override { return hyphen ? 8: 5; }
};

struct eop: public newline_opportunity {
  eop(): newline_opportunity{false} {}

  int value() const override { return -linebreaking::infinity; }
};

output::fragment const newline{utf8toucs4("\n"), "new line character"};
output::fragment const indent_2{utf8toucs4("  "), "indent of two spaces"};
output::fragment const dot6_sign {utf8toucs4("\u2820"), "added by transcriber"};
output::fragment const appoggiatura_sign {utf8toucs4("\u2810\u2822"), "appoggiatura"};
output::fragment const mordent_sign {utf8toucs4("\u2810\u2816\u2807"), "mordent"};
output::fragment const staccato_sign {utf8toucs4("\u2826"), "staccato"};
output::fragment const turn_above_or_below_note_sign {utf8toucs4("\u2820\u2832"), "turn"};
output::fragment const number_sign{utf8toucs4("\u283C"), "number sign"};
output::fragment const dash_sign {utf8toucs4("\u2824"), "dash"};
output::fragment const rest_sign[] = {
  {utf8toucs4("\u280D"), ""}, {utf8toucs4("\u2825"), ""},
  {utf8toucs4("\u2827"), ""}, {utf8toucs4("\u282D"), ""}};
output::fragment const note_sign[][steps_per_octave] = {
  {{utf8toucs4("\u283D"), ""},
   {utf8toucs4("\u2835"), ""},
   {utf8toucs4("\u282F"), ""},
   {utf8toucs4("\u283F"), ""},
   {utf8toucs4("\u2837"), ""},
   {utf8toucs4("\u282E"), ""},
   {utf8toucs4("\u283E"), ""}},
  {{utf8toucs4("\u281D"), ""},
   {utf8toucs4("\u2815"), ""},
   {utf8toucs4("\u280F"), ""},
   {utf8toucs4("\u281F"), ""},
   {utf8toucs4("\u2817"), ""},
   {utf8toucs4("\u280E"), ""},
   {utf8toucs4("\u281E"), ""}},
  {{utf8toucs4("\u2839"), ""},
   {utf8toucs4("\u2831"), ""},
   {utf8toucs4("\u282B"), ""},
   {utf8toucs4("\u283B"), ""},
   {utf8toucs4("\u2833"), ""},
   {utf8toucs4("\u282A"), ""},
   {utf8toucs4("\u283A"), ""}},
  {{utf8toucs4("\u2819"), ""},
   {utf8toucs4("\u2811"), ""},
   {utf8toucs4("\u280B"), ""},
   {utf8toucs4("\u281B"), ""},
   {utf8toucs4("\u2813"), ""},
   {utf8toucs4("\u280A"), ""},
   {utf8toucs4("\u281A"), ""}}};
output::fragment octave_sign[] = {
  {utf8toucs4("\u2808\u2808"), "subcontra octave sign"},
  {utf8toucs4("\u2808"), ""},
  {utf8toucs4("\u2818"), ""},
  {utf8toucs4("\u2838"), ""},
  {utf8toucs4("\u2810"), ""},
  {utf8toucs4("\u2828"), ""},
  {utf8toucs4("\u2830"), ""},
  {utf8toucs4("\u2820"), ""},
  {utf8toucs4("\u2820\u2820"), ""},
};
output::fragment const augmentation_dot{utf8toucs4("\u2804"), "augmentation dot"};
output::fragment const finger_sign[] = {
  {utf8toucs4("\u2801"), ""},
  {utf8toucs4("\u2803"), ""},
  {utf8toucs4("\u2807"), ""},
  {utf8toucs4("\u2802"), ""},
  {utf8toucs4("\u2805"), ""},
};
output::fragment const partial_voice_separator{utf8toucs4("\u2810\u2802"), ""};
output::fragment const partial_measure_separator{utf8toucs4("\u2828\u2805"), ""};
output::fragment const voice_separator{utf8toucs4("\u2823\u281C"), ""};
output::fragment const moving_note_separator {utf8toucs4("\u2820"), "moving note separator"};
output::fragment const slur_sign{utf8toucs4("\u2809"), "slur"};
output::fragment const tie_sign{utf8toucs4("\u2808\u2809"), "tie"};
output::fragment const simile_sign {utf8toucs4("\u2836"), "simile"};
output::fragment const eom_sign{utf8toucs4("\u2823\u2805"), ""};
output::fragment const hyphen_sign{utf8toucs4("\u2810"), "hyphen"};
output::fragment const begin_repeat_sign {utf8toucs4("\u2823\u2836"), "begin repeat" };
output::fragment const end_repeat_sign {utf8toucs4("\u2823\u2806"), "end repeat" };
output::fragment const end_part_sign {utf8toucs4("\u2823\u2805\u2804"), "end part"};
output::fragment const hand_sign[] = {
  {utf8toucs4("\u2828\u281C"), "right hand sign", true},
  {utf8toucs4("\u2838\u281C"), "left hand sign", true}
};
output::fragment const natural_sign {utf8toucs4("\u2821"), "natural"};
output::fragment const flat_sign {utf8toucs4("\u2823"), "flat"};
output::fragment const sharp_sign {utf8toucs4("\u2829"), "sharp"};
output::fragment const upper_digit_sign[10] = {
  {utf8toucs4("\u281A"), "zero" },
  {utf8toucs4("\u2801"), "one"},
  {utf8toucs4("\u2803"), "two"},
  {utf8toucs4("\u2809"), "three"},
  {utf8toucs4("\u2819"), "four"},
  {utf8toucs4("\u2811"), "five"},
  {utf8toucs4("\u280B"), "six"},
  {utf8toucs4("\u281B"), "seven"},
  {utf8toucs4("\u2813"), "eight"},
  {utf8toucs4("\u280A"), "nine"}
};
output::fragment const lower_digit_sign[10] = {
  {utf8toucs4("\u2834"), "lower zero"},
  {utf8toucs4("\u2802"), "lower one"},
  {utf8toucs4("\u2806"), "lower two"},
  {utf8toucs4("\u2812"), "lower three"},
  {utf8toucs4("\u2832"), "lower four"},
  {utf8toucs4("\u2822"), "lower five"},
  {utf8toucs4("\u2816"), "lower six"},
  {utf8toucs4("\u2836"), "lower seven"},
  {utf8toucs4("\u2826"), "lower eight"},
  {utf8toucs4("\u2814"), "lower nine"}
};
output::fragment const interval_sign[] = {
  {utf8toucs4("\u280C"), "second"},
  {utf8toucs4("\u282C"), "third"},
  {utf8toucs4("\u283C"), "fourth"},
  {utf8toucs4("\u2814"), "fifth"},
  {utf8toucs4("\u2834"), "sixth"},
  {utf8toucs4("\u2812"), "seventh"},
  {utf8toucs4("\u2824"), "octave"}
};
output::fragment const distinct_value_sign {utf8toucs4("\u2823\u2802"), "distinct values"};
output::fragment const small_value_sign {utf8toucs4("\u2820\u2823\u2802"), "small values"};
output::fragment const large_value_sign {utf8toucs4("\u2818\u2823\u2802"), "large values"};

std::size_t length(output const &o) {
  std::size_t len = 0;
  for (auto &&fragment: o.fragments) {
    auto str = std::u32string{fragment.unicode};
    len += str.length();
  }
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
    if (!s.time_sigs.empty()) {
      result.fragments.push_back(number_sign);
      for (auto digit: digits(s.time_sigs.front().numerator()))
        result.fragments.push_back(upper_digit_sign[digit]);
      for (auto digit: digits(s.time_sigs.front().denominator()))
        result.fragments.push_back(lower_digit_sign[digit]);
    }
    result.fragments.push_back(newline);

    return true;
  }

  bool visit_part(ast::part const &p) {
    section_n = p.size();

    return true;
  }

  bool visit_section(ast::section const &s) {
    last_section = !bool(--section_n);
    staves = s.paragraphs.size();
    staff = 0;

    add_to_para(new atom{indent_2});
    if (s.number) {
      output tmp;
      tmp.fragments.push_back(number_sign);
      for (auto digit: digits(*s.number))
        tmp.fragments.push_back(upper_digit_sign[digit]);
      add_to_para(new atom{tmp});
      add_to_para(new whitespace{});
    }
    if (s.range) {
      output tmp;
      tmp.fragments.push_back(number_sign);
      for (auto digit: digits(s.range->first.number))
        tmp.fragments.push_back(lower_digit_sign[digit]);
      tmp.fragments.push_back(dash_sign);
      for (auto digit: digits(s.range->last.number))
        tmp.fragments.push_back(lower_digit_sign[digit]);
      add_to_para(new atom{tmp});
      add_to_para(new whitespace{});
    }

    return true;
  }

  bool visit_paragraph(ast::paragraph const &)
  {
    if (para.empty()) add_to_para(new atom{indent_2});

    if (staves > 1) {
      add_to_para(new atom{hand_sign[staff]});
      add_to_para(new newline_opportunity{true});
    }

    return true;
  }

  bool end_of_paragraph(ast::paragraph const &) {
    if (last_section) add_to_para(new atom{eom_sign});
    add_to_para(new eop{});

    auto const breaks = linebreaking::breakpoints(para, {style.columns});
    BOOST_ASSERT(!breaks.empty());
    auto i = para.begin();
    for (auto &&j: breaks) {
      while (i < j) {
        if (dynamic_cast<whitespace const *>(i->get())) {
          result.fragments.push_back(output::fragment{utf8toucs4(" "), "space"});
        } else if (auto a = dynamic_cast<atom const *>(i->get())) {
          for (auto &&f: a->content.fragments) result.fragments.push_back(f);
        }
        ++i;
      }
      if (auto nl = dynamic_cast<newline_opportunity const *>(i->get())) {
        if (nl->hyphen) result.fragments.push_back(hyphen_sign);
        result.fragments.push_back(newline);
        if (dynamic_cast<guide const *>(std::next(i)->get())) i += 2;
      } else if (dynamic_cast<whitespace const *>(i->get())) {
        result.fragments.push_back(newline);
        ++i;
      }
    }
    while (i < para.end()) {
      if (dynamic_cast<whitespace const *>(i->get())) {
        result.fragments.push_back(output::fragment{utf8toucs4(" "), "space"});
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

  bool visit_measure(ast::measure const &m) {
    if (m.ending) {
      output tmp;
      tmp.fragments.push_back(number_sign);
      for (auto digit: digits(*m.ending))
        tmp.fragments.push_back(lower_digit_sign[digit]);
      tmp.fragments.back().needs_guide_dot = true;
      add_to_para(new atom{tmp});
    }

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
    if (!in_notegroup) add_to_para(new newline_opportunity{true});

    return true;
  }

  bool visit_note(ast::note const &n) {
    if (n.notegroup_member == ast::notegroup_member_type::begin ||
        n.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else { in_notegroup = false; }

    output res;
    for (auto &&articulation: n.articulations)
      switch (articulation) {
      case appoggiatura: res.fragments.push_back(appoggiatura_sign); break;
      case mordent: res.fragments.push_back(mordent_sign); break;
      case staccato: res.fragments.push_back(staccato_sign); break;
      case turn_above_or_below_note: res.fragments.push_back(turn_above_or_below_note_sign); break;
      }
    if (n.acc) {
      switch (*n.acc) {
      case natural: res.fragments.push_back(natural_sign); break;
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
    for (auto &&s: n.slurs) {
      switch (s.value) {
      case ast::slur::single:
        res.fragments.push_back(slur_sign);
        break;
      default: BOOST_ASSERT(false);
      }
    }
    if (n.tie) {
      switch (n.tie->value) {
      case ast::tie::single:
        res.fragments.push_back(tie_sign);
        break;
      }
    }
    add_to_para(new atom{res});
    return true;
  }

  bool visit_rest(ast::rest const &r)
  {
    if (r.notegroup_member == ast::notegroup_member_type::begin ||
        r.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else { in_notegroup = false; }

    output res;
    if (r.by_transcriber) res.fragments.push_back(dot6_sign);
    res.fragments.push_back(rest_sign[r.ambiguous_value]);
    std::fill_n(std::back_inserter(res.fragments), r.dots, augmentation_dot);

    add_to_para(new atom{res});

    return true;
  }

  bool visit_interval(ast::interval const &i)
  {
    output res;
    if (i.acc) {
      switch (*i.acc) {
      case natural: res.fragments.push_back(natural_sign); break;
      case flat: std::fill_n(std::back_inserter(res.fragments), 1, flat_sign); break;
      case double_flat: std::fill_n(std::back_inserter(res.fragments), 2, flat_sign); break;
      case sharp: std::fill_n(std::back_inserter(res.fragments), 1, sharp_sign); break;
      case double_sharp: std::fill_n(std::back_inserter(res.fragments), 2, sharp_sign); break;
      default: BOOST_ASSERT(false);
      }
    }
    if (i.octave_spec) res.fragments.push_back(octave_sign[*i.octave_spec - 1]);
    res.fragments.push_back(interval_sign[i.steps - 1]);

    fingering_print_visitor fingering_printer{res};
    std::for_each(i.fingers.begin(), i.fingers.end(), apply_visitor(fingering_printer));
    if (i.tie) {
      switch (i.tie->value) {
      case ast::tie::single:
        res.fragments.push_back(tie_sign);
        break;
      }
    }

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

  bool visit_value_prefix(ast::value_prefix const &vp) {
    switch (vp.value) {
    case ast::value_prefix::distinct:
      add_to_para(new atom{distinct_value_sign});
      break;
    case ast::value_prefix::small_follows:
      add_to_para(new atom{small_value_sign});
      break;
    case ast::value_prefix::large_follows:
      add_to_para(new atom{large_value_sign});
      break;
    default: BOOST_ASSERT(false);
    }

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
    case ast::end_part:
      add_to_para(new atom{end_part_sign});
      break;
    default: BOOST_ASSERT(false);
    }

    return true;
  }

  bool visit_hand_sign(ast::hand_sign const &h) {
    add_to_para(new atom{hand_sign[h.value]});

    return true;
  }

private:
  void add_to_para(linebreaking::object *object) {
    if (auto a = dynamic_cast<atom *>(object)) {
      if (a->starts_with_one_of_dots_123()) {
        atom *box = nullptr;
        if (!para.empty()) {
          box = dynamic_cast<atom *>(para.back().get());
          if (!box && dynamic_cast<newline_opportunity *>(para.back().get())) {
            box = dynamic_cast<atom *>(para[para.size() - 2].get());
          }
        }
        if (box && box->content.fragments.back().needs_guide_dot)
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
