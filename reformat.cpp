#include <bmc/braille/reformat.hpp>

#include <bmc/braille/linebreaking.hpp>
#include <bmc/braille/ast/visitor.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace bmc { namespace braille {

namespace {

struct atom: public linebreaking::box {
  output content;

  atom(output const &o) {
    content.fragments.assign(o.fragments.begin(), o.fragments.end());
  }
  atom(std::initializer_list<output::fragment> c) {
    content.fragments.assign(c.begin(), c.end());
  }

  int width() const override 
  {
    int result = 0;
    for (auto &&c: content.fragments) result += c.unicode.length();
    return result;
  }

  friend std::ostream &operator<<(std::ostream &os, atom const &a) {
    for (auto &&c: a.content.fragments)
      os << boost::locale::conv::utf_to_utf<char>(c.unicode);

    return os;
  }
};

class whitespace: public linebreaking::glue {
public:
  int width() const override { return 1; }
};

struct newline_opportunity: public linebreaking::penalty {
  bool hyphen;

  newline_opportunity(bool hyphen): hyphen{hyphen} {}

  int width() const override { return hyphen? 1: 0; }
  int value() const override { return hyphen? 4: 16; }

  friend std::ostream &operator<<(std::ostream &os, newline_opportunity const &nl) {
    if (nl.hyphen == 1) os << u8"\u2810";
    os << std::endl;

    return os;
  }
};

class eop: public newline_opportunity {
public:
  eop(): newline_opportunity{false} {}

  int value() const override { return -linebreaking::infinity; }
};

output::fragment const newline { U"\n", "new line character" };
output::fragment const guide_dot { U"\u2804", "guide dot" };
output::fragment const number_sign { U"\u283C", "number sign" };
output::fragment const rest_sign[] = {
  { U"\u280D", ""},
  { U"\u2825", "" },
  { U"\u2827", "" },
  { U"\u282D", "" }
};
output::fragment const note_sign[][steps_per_octave] = {
  {
    { U"\u283D", "" }, { U"\u2835", "" }, { U"\u282F", "" }, { U"\u283F", "" },
    { U"\u2837", "" }, { U"\u282E", "" }, { U"\u283E", "" }
  },
  {
    { U"\u281D", "" }, { U"\u2815", "" }, { U"\u280F", "" }, { U"\u281F", "" },
    { U"\u2817", "" }, { U"\u280E", "" }, { U"\u281E", "" }
  },
  {
    { U"\u2839", "" }, { U"\u2831", "" }, { U"\u282B", "" }, { U"\u283B", "" },
    { U"\u2833", "" }, { U"\u282A", "" }, { U"\u283A", "" }
  },
  {
    { U"\u2819", "" }, { U"\u2811", "" }, { U"\u280B", "" }, { U"\u281B", "" },
    { U"\u2813", "" }, { U"\u280A", "" }, { U"\u281A", "" }
  }
};
output::fragment octave_sign[] = {
  { U"\u2808\u2808", "subcontra octave sign" },
  { U"\u2808", "" },
  { U"\u2818", "" },
  { U"\u2838", "" },
  { U"\u2810", "" },
  { U"\u2828", "" },
  { U"\u2830", "" },
  { U"\u2820", "" },
  { U"\u2820\u2820", "" },
};
output::fragment const augmentation_dot { U"\u2804", "augmentation dot" };
output::fragment const finger_sign[] = {
  { U"\u2801", "" },
  { U"\u2803", "" },
  { U"\u2807", "" },
  { U"\u2802", "" },
  { U"\u2805", "" },
};
output::fragment const partial_voice_separator { U"\u2810\u2802", "" };
output::fragment const partial_measure_separator { U"\u2828\u2805", "" };
output::fragment const voice_separator { U"\u2823\u281C", ""};
output::fragment const slur_sign { U"\u2809", "" };
output::fragment const eom_sign { U"\u2823\u2805", "" };
output::fragment const hyphen_sign { U"\u2810", "hyphen" };

std::size_t length(output const &o) {
  std::size_t len = 0;
  for (auto &&fragment: o.fragments) len += std::u32string{fragment.unicode}.length();
  return len;
}

class fingering_print_visitor: public boost::static_visitor<void>
{
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

struct print_visitor: public ast::const_visitor<print_visitor>
{
  output result;
  linebreaking::objects para;

  bool visit_part(ast::part const &p)
  {
    section_n = p.size();

    return true;
  }

  bool visit_section(ast::section const &s)
  {
    last_section = !--section_n;

    return true;
  }

  bool visit_paragraph(ast::paragraph const &)
  {
    return true;
  }

  bool end_of_paragraph(ast::paragraph const &)
  {
    if (last_section)
      para.emplace_back(new atom{eom_sign});
    para.emplace_back(new eop{});

    auto breaks = linebreaking::breakpoints(para, {32});
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
    return true;
  }

  bool between_paragraph_element(ast::paragraph_element const &, ast::paragraph_element const &)
  {
    para.emplace_back(new whitespace{});

    return true;
  }

  bool between_voice(ast::voice const &, ast::voice const &)
  {
    para.emplace_back(new atom{voice_separator});
    para.emplace_back(new newline_opportunity{false});

    return true;
  }

  bool between_partial_measure(ast::partial_measure const &, ast::partial_measure const &)
  {
    para.emplace_back(new atom{partial_measure_separator});
    para.emplace_back(new newline_opportunity{false});

    return true;
  }

  bool between_partial_voice(ast::partial_voice const &, ast::partial_voice const &)
  {
    para.emplace_back(new atom{partial_voice_separator});
    para.emplace_back(new newline_opportunity{false});

    return true;
  }

  bool between_sign(ast::sign const &, ast::sign const &)
  {
    if (not in_notegroup) para.emplace_back(new newline_opportunity{true});

    return true;
  }

  bool visit_note(ast::note const &n)
  {
    if (n.notegroup_member == ast::notegroup_member_type::begin or
        n.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else {
      in_notegroup = false;
    }

    output res;
    if (n.octave_spec)
      res.fragments.push_back(octave_sign[*n.octave_spec - 1]);
    res.fragments.push_back(note_sign[n.ambiguous_value][n.step]);
    std::fill_n(std::back_inserter(res.fragments), n.dots, augmentation_dot);
    fingering_print_visitor fingering_printer{res};
    std::for_each(n.fingers.begin(), n.fingers.end(),
                  apply_visitor(fingering_printer));
    para.emplace_back(new atom{res});
    return true;
  }

  bool visit_rest(ast::rest const &r)
  {
    if (r.notegroup_member == ast::notegroup_member_type::begin or
        r.notegroup_member == ast::notegroup_member_type::middle) {
      in_notegroup = true;
    } else {
      in_notegroup = false;
    }

    output res;
    res.fragments.push_back(rest_sign[r.ambiguous_value]);
    std::fill_n(std::back_inserter(res.fragments), r.dots, augmentation_dot);

    para.emplace_back(new atom{res});

    return true;
  }

private:
  int section_n;
  bool last_section;
  bool in_notegroup;
};

}

output reformat(ast::score const &score, format_style const &style)
{
  print_visitor printer;
  bool const ok = printer.traverse_score(score);
  BOOST_ASSERT(ok);
  return printer.result;
}

std::ostream &operator<<(std::ostream &os, output const &out)
{
  for (auto &&f: out.fragments)
    os << boost::locale::conv::utf_to_utf<char>(f.unicode);

  return os;
}

}}
