#include <bmc/braille/reformat.hpp>
#include <bmc/braille/ast/visitor.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace bmc { namespace braille {

namespace {

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

  bool end_of_paragraph(ast::paragraph const &) {
    if (last_section) result.fragments.push_back(output::fragment{U"\u2823\u2805", ""});
    result.fragments.push_back(newline);

    return true;
  }

  bool between_paragraph_element(ast::paragraph_element const &, ast::paragraph_element const &)
  {
    result.fragments.push_back(output::fragment{U" ", "space"});

    return true;
  }

  bool between_voice(ast::voice const &, ast::voice const &)
  {
    result.fragments.push_back(voice_separator);

    return true;
  }

  bool between_partial_measure(ast::partial_measure const &, ast::partial_measure const &)
  {
    result.fragments.push_back(partial_measure_separator);

    return true;
  }

  bool between_partial_voice(ast::partial_voice const &, ast::partial_voice const &)
  {
    result.fragments.push_back(partial_voice_separator);

    return true;
  }

  bool visit_note(ast::note const &n) {
    if (n.octave_spec)
      result.fragments.push_back(octave_sign[*n.octave_spec - 1]);
    result.fragments.push_back(note_sign[n.ambiguous_value][n.step]);
    std::fill_n(std::back_inserter(result.fragments), n.dots, augmentation_dot);
    fingering_print_visitor fingering_printer{result};
    std::for_each(n.fingers.begin(), n.fingers.end(),
                  apply_visitor(fingering_printer));

    return true;
  }

  bool visit_rest(ast::rest const &r) {
    result.fragments.push_back(rest_sign[r.ambiguous_value]);
    std::fill_n(std::back_inserter(result.fragments), r.dots, augmentation_dot);

    return true;
  }

private:
  int section_n;
  bool last_section;
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
