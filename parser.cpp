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
using boost::spirit::x3::eps;
using boost::spirit::x3::repeat;
using boost::spirit::x3::rule;
using boost::spirit::x3::unused_type;

template < typename Encoding
         , template<typename> typename BinaryPredicate = std::equal_to
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
      BOOST_ASSERT(dots ^ (1 << (digit - 1)));
      dots |= 1 << (digit - 1);
      decimal_dots /= 10;
    }
    BOOST_ASSERT(!(~Mask & dots));
  }

  template <typename Char, typename Context>
  bool test(Char ch, Context const& context) const
  {
    return ((sizeof(Char) <= sizeof(char_type)) || encoding::ischar(ch))
        && BinaryPredicate<uint8_t>{}(get_dots_for_character(ch) & Mask, dots);
  }

  uint8_t dots;
};

inline brl_parser<char_encoding::unicode, std::equal_to, 0X3F>
brl(unsigned decimal_dots)
{
  return { decimal_dots };
}

inline auto brl(unsigned decimal_dots1, unsigned decimal_dots2)
-> decltype(brl(decimal_dots1) >> brl(decimal_dots2))
{
  return brl(decimal_dots1) >> brl(decimal_dots2);
}

inline auto brl(unsigned a, unsigned b, unsigned c)
-> decltype(brl(a, b) >> brl(c))
{
  return brl(a, b) >> brl(c);
}

inline auto brl(unsigned a, unsigned b, unsigned c, unsigned d)
-> decltype(brl(a, b, c) >> brl(d))
{
  return brl(a, b, c) >> brl(d);
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
rule<struct time_signature, ast::time_signature> const time_signature = "time_signature";
rule<struct upper_number_as_negative, unsigned> const upper_number_as_negative = "upper_number_as_negative";
struct key_signature;
rule<struct key_signature, ast::key_signature> const key_signature = "key_signature";
rule<struct augmentation_dots, unsigned> const augmentation_dots = "augmentation_dots";
rule<struct note, ast::note> const note = "note";

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

auto assign_0 = [](auto& ctx){ _attr(ctx) = 0; };
auto multiply_by_10_plus_attr = [](auto& ctx) { _val(ctx) = 10 * _val(ctx) + _attr(ctx); };
auto const upper_number_def = eps[assign_0] >> +upper_digit[multiply_by_10_plus_attr];
auto const lower_number_def = eps[assign_0] >> +lower_digit[multiply_by_10_plus_attr];

auto const number_sign = brl(3456);
auto const time_signature_def = number_sign >> upper_number >> lower_number
                              | brl(46)     >> attr(4)      >> attr(4)
                              | brl(456)    >> attr(4)      >> attr(4)
                              ;

auto const sharp_sign = brl(146);
auto const flat_sign = brl(126);
auto multiply_by_10_minus_attr = [](auto& ctx) { _val(ctx) = 10 * _val(ctx) - _attr(ctx); };
auto const upper_number_as_negative_def =
  eps[assign_0] >> +upper_digit[multiply_by_10_minus_attr];
auto const key_signature_def =
    repeat(3)[sharp_sign] >> attr(3)
  | repeat(3)[flat_sign]  >> attr(-3)
  | repeat(2)[sharp_sign] >> attr(2)
  | repeat(2)[flat_sign]  >> attr(-2)
  | sharp_sign            >> attr(1)
  | flat_sign             >> attr(-1)
  | number_sign           >> upper_number             >> sharp_sign
  | number_sign           >> upper_number_as_negative >> flat_sign
  | eps                   >> attr(0)
  ;

auto const optional_dot = (!brl_mask(123)) | (brl(3) > &brl_mask(123));

auto plus_1 = [](auto& ctx) { _val(ctx) += 1; };
auto const augmentation_dots_def = eps[assign_0] >> *brl(3)[plus_1];

auto const natural_sign = brl(16);

auto const accidental =
    repeat(3)[sharp_sign] >> attr(triple_sharp)
  | repeat(3)[flat_sign]  >> attr(triple_flat)
  | repeat(2)[sharp_sign] >> attr(double_sharp)
  | repeat(2)[flat_sign]  >> attr(double_flat)
  | sharp_sign            >> attr(sharp)
  | flat_sign             >> attr(flat)
  | natural_sign          >> attr(natural)
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

auto const note_def =
    -accidental
 >> -octave
 >> step >> value
 >> augmentation_dots
  ;

struct key_signature : annotate_on_success {};
struct time_signature : annotate_on_success {};
struct note : annotate_on_success {};

BOOST_SPIRIT_DEFINE(
  upper_digit, upper_number, lower_digit, lower_number, upper_number_as_negative,
  time_signature, key_signature,
  augmentation_dots, note
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
    boost::spirit::x3::with<parser::error_handler_tag>(ref(error_handler))[p],
    attribute);

  if (success && first != last && full_match) {
    error_handler(first, "Error! Expected end of input here: ");
    success = false;
  }

  if (success)
    return std::make_tuple(attribute, std::move(error_handler));

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

}}

