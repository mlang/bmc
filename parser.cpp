#include <bmc/braille/parser/ast_adapted.hpp>
#include <bmc/braille/parser/parser.hpp>
#include <boost/spirit/home/support/char_encoding/unicode.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>

namespace bmc { namespace braille { namespace parser {

using boost::spirit::x3::_attr;
using boost::spirit::x3::_val;
using boost::spirit::x3::annotate_on_success;
using boost::spirit::x3::attr;
using boost::spirit::x3::traits::attribute_of;
namespace char_encoding = boost::spirit::char_encoding;
using boost::spirit::x3::char_parser;
using boost::spirit::x3::eoi;
using boost::spirit::x3::eol;
using boost::spirit::x3::eps;
using boost::spirit::x3::inf;
using boost::spirit::x3::matches;
using boost::spirit::x3::omit;
using boost::spirit::x3::repeat;
using boost::spirit::x3::rule;
using boost::spirit::x3::with;
using boost::spirit::x3::unused_type;

template < typename Encoding
         , template<class> class BinaryPredicate = std::equal_to
         , uint8_t Mask = 0XFF
         >
struct brl_parser : char_parser<brl_parser<Encoding, BinaryPredicate, Mask>>
{
  using attribute_type = unused_type;
  using char_type = typename Encoding::char_type;
  using encoding = Encoding;
  static bool const has_attribute = false;

  brl_parser(unsigned decimal_dots)
  : dots(0)
  {
    while (decimal_dots) {
      auto const digit = decimal_dots % 10;
      BOOST_ASSERT((digit > 0) && (digit < 9));
      auto const bit = 1 << (digit - 1);
      BOOST_ASSERT(dots ^ bit);
      dots |= bit;
      decimal_dots /= 10;
    }
    BOOST_ASSERT(!(~Mask & dots));
  }

  template <typename Char, typename Context>
  bool test(Char ch, Context const& context) const
  {
    return ((sizeof(Char) <= sizeof(char_type)) || encoding::ischar(ch))
        && encoding::isprint(ch)
        && BinaryPredicate<uint8_t>{}(get_dots_for_character(ch) & Mask, dots);
  }

  uint8_t dots;
};

inline brl_parser<char_encoding::unicode, std::equal_to, 0X3F>
brl(unsigned decimal_dots)
{
  return { decimal_dots };
}

inline auto brl(unsigned a, unsigned b)
{
  return brl(a) >> brl(b);
}

inline auto brl(unsigned a, unsigned b, unsigned c)
{
  return brl(a) >> brl(b) >> brl(c);
}

inline auto brl(unsigned a, unsigned b, unsigned c, unsigned d)
{
  return brl(a) >> brl(b) >> brl(c) >> brl(d);
}

inline brl_parser<char_encoding::unicode, std::bit_and, 0X3F>
brl_mask(unsigned decimal_dots)
{
  return { decimal_dots };
}

inline brl_parser<char_encoding::unicode, std::equal_to, 0X1B>
brl_1245(unsigned decimal_dots)
{
  return { decimal_dots };
}

inline brl_parser<char_encoding::unicode, std::equal_to, 0X24>
brl_36(unsigned decimal_dots)
{
  return { decimal_dots };
}

rule<struct upper_digit, unsigned> const upper_digit = "upper_digit";
rule<struct lower_digit, unsigned> const lower_digit = "lower_digit";
rule<struct upper_number, unsigned> const upper_number = "upper_number";
rule<struct lower_number, unsigned> const lower_number = "lower_number";
struct time_signature;
rule<struct time_signature, ast::time_signature> const
time_signature = "time_signature";
rule<struct upper_number_as_negative, unsigned> const
upper_number_as_negative = "upper_number_as_negative";
struct key_signature;
rule<struct key_signature, ast::key_signature> const
key_signature = "key_signature";
rule<struct clef, ast::clef> const clef = "clef";
rule<struct augmentation_dots, unsigned> const
augmentation_dots = "augmentation_dots";
struct fingering;
rule<struct fingering, ast::fingering> const fingering = "fingering";
rule<struct note, ast::note> const note = "note";
rule<struct rest, ast::rest> const rest = "rest";
rule<struct moving_note, ast::moving_note> const moving_note = "moving_note";
rule<struct chord, ast::chord> const chord = "chord";
rule<struct value_distinction, ast::value_distinction> const
value_distinction = "value_distinction";
rule<struct tie, ast::tie> const tie = "tie";
rule<struct tuplet_start, ast::tuplet_start> const tuplet_start = "tuplet_start";
rule<struct simile, ast::simile> const simile = "simile";
rule<struct hand_sign, ast::hand_sign> const hand_sign = "hand_sign";
rule<struct barline, ast::barline> const barline = "barline";
rule<struct measure, ast::measure> const measure = "measure";
rule<struct key_and_time_signature, ast::key_and_time_signature> const
key_and_time_signature = "key_and_time_signature";
rule<struct paragraph_element, ast::paragraph_element> const
paragraph_element = "paragraph_element";
rule<struct paragraph, ast::paragraph> const paragraph = "paragraph";
rule<struct measure_specification, ast::measure_specification> const
measure_specification = "measure_specification";
rule<struct measure_range, ast::measure_range> const
measure_range = "measure_range";
rule<struct section_number, ast::section::number_type> const
section_number = "section_number";
rule<struct solo_section, ast::section> const solo_section = "solo_section";
rule<struct last_solo_section, ast::section> const
last_solo_section = "last_solo_section";
rule<struct solo_part, ast::part> const solo_part = "solo_part";
rule<struct keyboard_section, ast::section> const keyboard_section = "keyboard_section";
rule<struct last_keyboard_section, ast::section> const
last_keyboard_section = "last_keyboard_section";
rule<struct keyboard_part, ast::part> const keyboard_part = "keyboard_part";
rule<struct score, ast::score> const score = "score";

auto const upper_digit_def = brl(245)  >> attr(0)
                           | brl(1)    >> attr(1)
                           | brl(12)   >> attr(2)
                           | brl(14)   >> attr(3)
                           | brl(145)  >> attr(4)
                           | brl(15)   >> attr(5)
                           | brl(124)  >> attr(6)
                           | brl(1245) >> attr(7)
                           | brl(125)  >> attr(8)
                           | brl(24)   >> attr(9)
                           ;
auto const lower_digit_def = brl(356)  >> attr(0)
                           | brl(2)    >> attr(1)
                           | brl(23)   >> attr(2)
                           | brl(25)   >> attr(3)
                           | brl(256)  >> attr(4)
                           | brl(26)   >> attr(5)
                           | brl(235)  >> attr(6)
                           | brl(2356) >> attr(7)
                           | brl(236)  >> attr(8)
                           | brl(35)   >> attr(9)
                           ;

auto assign_0 = [](auto& ctx)
{
  _attr(ctx) = 0;
};

auto multiply_by_10_plus_attr = [](auto& ctx)
{
  _val(ctx) = 10 * _val(ctx) + _attr(ctx);
};

auto const upper_number_def =
    eps[assign_0] >> +upper_digit[multiply_by_10_plus_attr]
  ;

auto const lower_number_def =
    eps[assign_0] >> +lower_digit[multiply_by_10_plus_attr]
  ;

auto const number_sign = brl(3456);

auto const time_signature_def =
    number_sign  >> upper_number >> lower_number
  | brl(46, 14)  >> attr(4)      >> attr(4)
  | brl(456, 14) >> attr(4)      >> attr(4)
  ;

auto const sharp_sign = brl(146);
auto const flat_sign = brl(126);

auto multiply_by_10_minus_attr = [](auto& ctx)
{
  _val(ctx) = 10 * _val(ctx) - _attr(ctx);
};

auto const upper_number_as_negative_def =
    eps[assign_0] >> +upper_digit[multiply_by_10_minus_attr]
  ;

auto const key_signature_def =
    sharp_sign >> ( sharp_sign >> ( sharp_sign >> attr(3)
                                  |               attr(2)                 )
                  |                               attr(1)                 )
  | flat_sign  >> ( flat_sign  >> ( flat_sign  >> attr(-3)
                                  |               attr(-2)                )
                  |                               attr(-1)                )
  | number_sign                >> ( upper_number             >> sharp_sign
                                  | upper_number_as_negative >> flat_sign )
  |                                 attr(0)
  ;

auto const optional_dot = (!brl_mask(123)) | (brl(3) > &brl_mask(123));

auto const clef_def =
    brl(345)
 >> ( brl(34)   >> attr(ast::clef::type::G)
    | brl(346)  >> attr(ast::clef::type::C)
    | brl(3456) >> attr(ast::clef::type::F)
    )
 >> -( brl(4)   >> attr(1)
     | brl(45)  >> attr(2)
     | brl(456) >> attr(3)
     | brl(5)   >> attr(4)
     | brl(46)  >> attr(5)
     )
 >> ( brl(123)  >> attr(false)
    | brl(13)   >> attr(true)
    )
  > optional_dot
  ;

auto plus_1 = [](auto& ctx)
{
  _val(ctx) += 1;
};

auto const augmentation_dots_def =
    eps[assign_0] >> *brl(3)[plus_1]
  ;

auto const articulation =
    brl(26)             >> attr(short_appoggiatura)
  | brl(5, 26)          >> attr(appoggiatura)
  | brl(236)            >> attr(staccato)
  | brl(6, 236)         >> attr(staccatissimo)
  | brl(5, 236)         >> attr(mezzo_staccato)
  | brl(456, 236)       >> attr(agogic_accent)
  | brl(46, 236)        >> attr(accent)
  | brl(256)            >> attr(turn_between_notes)
  | brl(6, 256)         >> attr(turn_above_or_below_note)
  | brl(256, 123)       >> attr(inverted_turn_between_notes)
  | brl(6, 256, 123)    >> attr(inverted_turn_above_or_below_note)
  | brl(5, 235)         >> attr(short_trill)
  | brl(56, 235)        >> attr(extended_short_trill)
  | brl(5, 235, 123)    >> attr(mordent)
  | brl(56, 235, 123)   >> attr(extended_mordent)
  | brl(345, 13)        >> attr(arpeggio_up)
  | brl(5, 345, 13)     >> attr(arpeggio_up_multi_staff)
  | brl(345, 13, 13)    >> attr(arpeggio_down)
  | brl(5, 345, 13, 13) >> attr(arpeggio_down_multi_staff)
  ;

auto const natural_sign = brl(16);

auto const accidental =
    sharp_sign >> ( sharp_sign >> ( sharp_sign >> attr(triple_sharp)
                                  |               attr(double_sharp) )
                  |                               attr(sharp)        )
  | flat_sign  >> ( flat_sign  >> ( flat_sign  >> attr(triple_flat)
                                  |               attr(double_flat) )
                  |                               attr(flat)        )
  | natural_sign                               >> attr(natural)
  ;

auto const octave =
    brl(4, 4) >> attr(1)
  | brl(4)    >> attr(2)
  | brl(45)   >> attr(3)
  | brl(456)  >> attr(4)
  | brl(5)    >> attr(5)
  | brl(46)   >> attr(6)
  | brl(56)   >> attr(7)
  | brl(6)    >> attr(8)
  | brl(6, 6) >> attr(9)
  ;

auto const step =
    &brl_1245(145)  >> attr(C)
  | &brl_1245(15)   >> attr(D)
  | &brl_1245(124)  >> attr(E)
  | &brl_1245(1245) >> attr(F)
  | &brl_1245(125)  >> attr(G)
  | &brl_1245(24)   >> attr(A)
  | &brl_1245(245)  >> attr(B)
  ;

auto const value =
    brl_36(36) >> attr(ast::whole_or_16th)
  | brl_36(3)  >> attr(ast::half_or_32th)
  | brl_36(6)  >> attr(ast::quarter_or_64th)
  | brl_36(0)  >> attr(ast::eighth_or_128th)
  ;

auto const finger =
    brl(1)   >> attr(1)
  | brl(12)  >> attr(2)
  | brl(123) >> attr(3)
  | brl(2)   >> attr(4)
  | brl(13)  >> attr(5)
  ;

auto const fingering_def =
    +finger >> -(brl(36) >> +finger)
  | *finger
  ;

auto const note_def =
    *articulation
 >> -accidental
 >> -octave
 >> step >> value
 >> augmentation_dots
 >> fingering
  ;

auto const rest_def =
    matches[brl(6)]
 >> ( brl(134)  >> attr(ast::whole_or_16th)
    | brl(136)  >> attr(ast::half_or_32th)
    | brl(1236) >> attr(ast::quarter_or_64th)
    | brl(1346) >> attr(ast::eighth_or_128th)
    )
 >> augmentation_dots
  ;

auto const single_tie = brl(4, 14);
auto const chord_tie = brl(46, 14);
auto const arpeggio_tie = brl(5, 14);

auto const tie_def =
    single_tie   >> attr(ast::tie::single)
  | chord_tie    >> attr(ast::tie::chord)
  | arpeggio_tie >> attr(ast::tie::arpeggio)
  ;

struct number_tag {};
rule<struct doubled_tuplet_number, unsigned> const
doubled_tuplet_number = "doubled_tuplet_number";
auto const doubled_tuplet_number_def = []{
  unsigned number;
  return with<number_tag>(number)[
      number_sign
   >> lower_number[([](auto &ctx){ get<number_tag>(ctx) = _attr(ctx); })]
   >> number_sign
   >> lower_number[
        ([](auto &ctx){ _pass(ctx) = (get<number_tag>(ctx) == _attr(ctx)); })
      ]
   >> brl(3)[([](auto &ctx){ _val(ctx) = get<number_tag>(ctx); })]
  ];
}();
BOOST_SPIRIT_DEFINE(doubled_tuplet_number)

auto const tuplet_start_def =
    brl(23, 23) >> attr(3)                >> attr(true) >> attr(true)
  | brl(23)     >> attr(3)                >> attr(true) >> attr(false)
  | doubled_tuplet_number                 >> attr(false) >> attr(true)
  | number_sign >> lower_number >> brl(3) >> attr(false) >> attr(false)
  ;

auto const interval_sign =
    brl(34)   >> attr(second)
  | brl(346)  >> attr(third)
  | brl(3456) >> attr(fourth)
  | brl(35)   >> attr(fifth)
  | brl(356)  >> attr(sixth)
  | brl(25)   >> attr(seventh)
  | brl(36)   >> attr(bmc::octave)
  ;

struct interval;
rule<struct interval, ast::interval> const interval = "interval";

auto const interval_def =
    -accidental
 >> -octave
 >> interval_sign
 >> fingering
  ;

BOOST_SPIRIT_DEFINE(interval)

rule<struct moving_note_intervals, std::vector<ast::interval>> const
moving_note_intervals = "moving_note_intervals";
auto const moving_note_intervals_def =
    interval >> repeat(1, 2)[brl(6) >> interval]
  ;
BOOST_SPIRIT_DEFINE(moving_note_intervals)

auto const moving_note_def = note >> moving_note_intervals;

auto const chord_def =
    note >> +interval >> matches[chord_tie]
  ;

auto const value_distinction_def =
    brl(126, 2)     >> attr(ast::value_distinction::distinct)
  | brl(6, 126, 2)  >> attr(ast::value_distinction::small_follows)
  | brl(45, 126, 2) >> attr(ast::value_distinction::large_follows)
  ;

auto const simile_def =
    -octave
 >> brl(2356) >> ( brl(2356) >> ( brl(2356) >> attr(3)
                                |              attr(2)       )
                 | ( number_sign             > upper_number )
                 |                             attr(1)       )
  ;

auto const partial_voice_sign =
    moving_note
  | chord
  | note
  | rest
  | simile
  | value_distinction
  | tie
  | tuplet_start
  | hand_sign
  | barline
  ;

auto const partial_voice             = +partial_voice_sign;
auto const partial_voice_separator   = brl(5, 2) >> *eol;
auto const partial_measure           = partial_voice % partial_voice_separator;
auto const partial_measure_separator = brl(46, 13) >> *eol;
auto const voice                     = partial_measure % partial_measure_separator;
auto const voice_separator           = brl(126, 345) >> *eol;

auto const ending = number_sign >> lower_number >> optional_dot;

auto const measure_def = -ending >> (voice % voice_separator);

auto const key_and_time_signature_def = key_signature >> time_signature;

auto const whitespace = brl(0);
auto const paragraph_element_def =
    key_and_time_signature
  | measure
  ;

auto const paragraph_def = paragraph_element % (whitespace | eol);

auto const measure_specification_def =
    lower_number
 >> -(number_sign >> lower_number)
  ;

auto const measure_range_def =
    number_sign
 >> measure_specification
 >> brl(36)
  > measure_specification
  ;

auto const section_number_def = number_sign >> upper_number;

auto const indent = repeat(2, inf)[whitespace];

auto const initial_key_and_time_signature =
    *whitespace >> key_and_time_signature >> *whitespace >> eol
  ;

auto const solo_section_def =
    -initial_key_and_time_signature
 >> -indent
 >> -(section_number >> whitespace)
 >> -(measure_range >> whitespace)
 >> paragraph;
  ;

auto const eom = brl(126, 13) >> !brl(3);

auto const left_hand_sign =
    (brl(456, 345) > optional_dot) >> attr(ast::hand_sign::left_hand)
  ;

auto const right_hand_sign =
    (brl(46, 345) > optional_dot) >> attr(ast::hand_sign::right_hand)
  ;

auto const hand_sign_def =
    right_hand_sign
  | left_hand_sign
  ;

auto const barline_def =
    brl(126, 2356)   >> attr(ast::barline::begin_repeat)
  | brl(126, 23)     >> attr(ast::barline::end_repeat)
  | brl(126, 13, 3)  >> attr(ast::barline::end_part)
  ;

auto const last_solo_section_def =
    -initial_key_and_time_signature
 >> -indent
 >> -(section_number >> whitespace)
 >> -(measure_range >> whitespace)
 >> paragraph
 >> eom
  ;

auto const solo_part_def = *(solo_section >> eol) > last_solo_section;

rule<struct keyboard_section_body, std::vector<ast::paragraph>> const
keyboard_section_body = "keyboard_section_body";
auto const keyboard_section_body_def =
    omit[right_hand_sign]
 >> paragraph
 >> eol
 >> indent
 >> omit[left_hand_sign]
 >> paragraph
  ;
BOOST_SPIRIT_DEFINE(keyboard_section_body)

auto const keyboard_section_def =
    -initial_key_and_time_signature
 >> indent
 >> -(section_number >> whitespace)
 >> -(measure_range >> whitespace)
 >> keyboard_section_body
  ;

rule<struct last_keyboard_section_body, std::vector<ast::paragraph>> const
last_keyboard_section_body = "last_keyboard_section_body";
auto const last_keyboard_section_body_def =
    omit[right_hand_sign]
 >> paragraph
 >> eom
 >> eol
 >> indent
 >> omit[left_hand_sign]
 >> paragraph
 >> eom
  ;
BOOST_SPIRIT_DEFINE(last_keyboard_section_body)

auto const last_keyboard_section_def =
    -initial_key_and_time_signature
 >> indent
 >> -(section_number >> whitespace)
 >> -(measure_range >> whitespace)
 >> last_keyboard_section_body
  ;

auto const keyboard_part_def =
    *(keyboard_section >> eol) >> last_keyboard_section
  ;

auto const part =
    keyboard_part
  | solo_part
  ;

auto const score_def =
    (part % (repeat(2, inf)[eol] >> !eoi))
 >> *eol
  ;

struct key_signature : annotate_on_success {};
struct clef : annotate_on_success {};
struct time_signature : annotate_on_success {};
struct fingering : annotate_on_success {};
struct note : annotate_on_success {};
struct rest : annotate_on_success {};
struct interval : annotate_on_success {};
struct moving_note : annotate_on_success {};
struct chord : annotate_on_success {};
struct value_distinction : annotate_on_success {};
struct tie : annotate_on_success {};
struct simile : annotate_on_success {};
struct hand_sign : annotate_on_success {};
struct barline : annotate_on_success {};
struct measure : annotate_on_success {};
struct score : annotate_on_success, report_on_error {};

BOOST_SPIRIT_DEFINE(
  upper_digit, upper_number, lower_digit, lower_number, upper_number_as_negative,
  time_signature, key_signature, clef,
  augmentation_dots, fingering,
  note, rest, moving_note, chord, simile,
  value_distinction, tie, tuplet_start, hand_sign, barline,
  measure, key_and_time_signature, paragraph_element,
  paragraph, section_number, measure_specification, measure_range,
  keyboard_section, last_keyboard_section, keyboard_part,
  solo_section, last_solo_section, solo_part,
  score
)

template <typename Iterator, typename Parser, typename Context = parser::unused_type>
result_t<typename attribute_of<Parser, Context>::type, Iterator>
parse_with_error_handler(Iterator &first, Iterator const &last, Parser const &p,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
)
{
  error_handler<Iterator> error_handler(first, last, out, filename);

  typename attribute_of<Parser, Context>::type attribute;
  bool success = parse(first, last,
    with<error_handler_tag>(ref(error_handler))[p], attribute);

  if (success && first != last && full_match) {
    error_handler(first, "Error! Expected end of input here:");
    success = false;
  }

  if (success)
    return std::make_pair(std::move(attribute), std::move(error_handler));

  return std::make_tuple(boost::none, std::move(error_handler));
}

}

auto parse_time_signature(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::time_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::time_signature, out, filename, full_match);
}

auto parse_key_signature(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::key_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::key_signature, out, filename, full_match);
}

auto parse_note(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::note,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::note, out, filename, full_match);
}

auto parse_key_and_time_signature(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::key_and_time_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::key_and_time_signature, out, filename, full_match);
}

auto parse_measure(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::measure,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::measure, out, filename, full_match);
}

auto parse_score(std::u32string const& input,
  std::ostream &out, std::string filename, bool full_match
) -> parser::result_t<parser::ast::score,
                      std::remove_reference<decltype(input)>::type::const_iterator>
{
  auto iter = input.begin();
  return parser::parse_with_error_handler(
    iter, input.end(), parser::score, out, filename, full_match);
}

}}

