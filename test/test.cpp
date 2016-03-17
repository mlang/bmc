// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "config.hpp"

#define BOOST_TEST_MODULE bmc_test
#if defined(BOOST_TEST_DYN_LINK)
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

#include <bmc/braille/text2braille.hpp>

struct text_table
{
  text_table() { bmc::braille::default_table = "de"; }
};

BOOST_GLOBAL_FIXTURE(text_table);

#include <boost/spirit/include/qi_parse.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <bmc/braille/parsing/grammar/time_signature.hpp>

BOOST_AUTO_TEST_CASE(time_signature_grammar_test_1) {
  std::wstring const input(L"#ab(");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::time_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, ::bmc::time_signature(12, 8));
  BOOST_CHECK_EQUAL(attribute, ::bmc::rational(3, 2));
  BOOST_CHECK_EQUAL(attribute.numerator(), 12);
  BOOST_CHECK_EQUAL(attribute.denominator(), 8);
}

#include "bmc/braille/parsing/grammar/key_signature.hpp"

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_1) {
  std::wstring const input(L"");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, 0);
}

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_2) {
  std::wstring const input(L"#f3");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, 6);
}

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_3) {
  std::wstring const input(L"22");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, -2);
}

#include "bmc/braille/parsing/qi/primitive/brl.hpp"

BOOST_AUTO_TEST_CASE(brl_parser_test) {
  std::wstring const input(L"#A");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  ::bmc::braille::brl_type brl;
  BOOST_REQUIRE(boost::spirit::qi::parse(begin, end, brl(3456) >> brl(1)));
  BOOST_CHECK(begin == end);
}

#include "bmc/braille/ast/visitors.hpp"
#include "bmc/braille/parsing/grammar/measure.hpp"

BOOST_AUTO_TEST_CASE(measure_test1) {
  std::wstring const input(L"vu.");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(2));
  BOOST_CHECK(::bmc::braille::ast::is_rest(attribute.voices[0][0][0][0]));
  BOOST_CHECK(::bmc::braille::ast::is_rest(attribute.voices[0][0][0][1]));
}

BOOST_AUTO_TEST_CASE(measure_test2) {
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  ::bmc::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_REQUIRE_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_REQUIRE_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.voices[1][1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][1][1].size(), std::size_t(2));
  BOOST_CHECK(::bmc::braille::ast::is_rest(attribute.voices[1][0][0][0]));
  BOOST_CHECK(::bmc::braille::ast::is_rest(attribute.voices[1][1][1][0]));
}

#include "bmc/braille/semantic_analysis.hpp"

#define BMC_CHECK_SIGN_LOCATION(sign, line, column) \
  BOOST_CHECK_EQUAL(boost::apply_visitor(::bmc::braille::ast::get_line(), sign), line);\
  BOOST_CHECK_EQUAL(boost::apply_visitor(::bmc::braille::ast::get_column(), sign), column)
#define BMC_CHECK_LOCATABLE_LOCATION(locatable, LINE, COLUMN) \
  BOOST_CHECK_EQUAL(locatable.begin_line, LINE);\
  BOOST_CHECK_EQUAL(locatable.begin_column, COLUMN)

BOOST_AUTO_TEST_CASE(measure_interpretations_test1) {
  std::wstring const input(L"_r72`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type error_handler(begin, end);
  parser_type parser(error_handler);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(error_handler, ::bmc::time_signature(3, 4));
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute), ::bmc::rational(3, 4));
  BMC_CHECK_LOCATABLE_LOCATION(attribute, 1, 1);
  BMC_CHECK_LOCATABLE_LOCATION(attribute.voices[0], 1, 1);
  BMC_CHECK_LOCATABLE_LOCATION(attribute.voices[0][0], 1, 1);
  BMC_CHECK_LOCATABLE_LOCATION(attribute.voices[0][0][0], 1, 1);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[0][0][0][0], 1, 1);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[0][0][0][1], 1, 3);
}

BOOST_AUTO_TEST_CASE(measure_interpretations_test2) {
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors, ::bmc::time_signature(3, 4));
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute), ::bmc::rational(3, 4));
  BMC_CHECK_LOCATABLE_LOCATION(attribute, 1, 1);
  BMC_CHECK_LOCATABLE_LOCATION(attribute.voices[0][0][0], 1, 1);
  BMC_CHECK_LOCATABLE_LOCATION(attribute.voices[1][0][0], 1, 6);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[0][0][0][0], 1, 1);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[1][0][0][0], 1, 6);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[1][1][0][0], 1, 9);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[1][1][1][0], 1, 13);
  BMC_CHECK_SIGN_LOCATION(attribute.voices[1][1][1][1], 1, 14);
}

BOOST_AUTO_TEST_CASE(notegroup_test1) {
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(9));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

struct get_type : boost::static_visitor<::bmc::rational>
{
  result_type operator()(::bmc::braille::ast::note const& note) const
  { return note.type; }
  result_type operator()(::bmc::braille::ast::rest const& rest) const
  { return rest.type; }
  result_type operator()(::bmc::braille::ast::chord const& chord) const
  { return (*this)(chord.base); }
  result_type operator()(::bmc::braille::ast::moving_note const& chord) const
  { return (*this)(chord.base); }
  template<typename T>
  result_type operator()(T const&) const
  { return ::bmc::zero; }
};

BOOST_AUTO_TEST_CASE(compiler_test1) {
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::measure_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.voices.size() == 1);
  BOOST_CHECK(attribute.voices[0].size() == 1);
  BOOST_CHECK(attribute.voices[0][0].size() == 1);
  BOOST_CHECK(attribute.voices[0][0][0].size() == 9);
  BOOST_CHECK_EQUAL(errors.ranges.size(), std::size_t(31));
  BOOST_CHECK(errors.ranges[0].begin() == input.begin());
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][0]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][1]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][2]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][3]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][4]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][5]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][6]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][7]), ::bmc::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][8]), ::bmc::rational(1, 2));
}

#include "bmc/braille/parsing/grammar/score.hpp"

BOOST_AUTO_TEST_CASE(score_solo_test1) {
  std::wstring const input(L"⠨⠽⠅⠐⠯⠃⠵⠁⠯⠃⠽⠁⠯⠃⠵⠁⠯⠃⠨⠽⠐⠯⠵⠯⠽⠯⠵⠯ ⠮⠅⠿⠇⠡⠯⠃⠿⠇⠽⠁⠿⠇⠯⠃⠿⠇⠮⠿⠯⠿⠽⠿⠯⠿2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_solo_test2) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"#c/\n⠐⠞⠃⠝⠞⠎⠚⠂⠈⠉⠞⠟⠗⠁⠎⠾⠽⠐⠢⠕⠽⠚⠊⠓2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_tuplet_test1) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠐⠹⠱⠆⠋⠛⠓⠆⠊⠚⠙⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_tuplet_test2) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠐⠹⠫⠆⠊⠓⠛⠳⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_tuplet_test3) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠸⠶⠄⠐456789w⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

#include "bmc/lilypond.hpp"
#include <sstream>
#include <fstream>
#include <boost/test/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

BOOST_AUTO_TEST_CASE(score_tuplet_test4) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠨⠹⠆⠋⠛⠓⠆⠛⠸⠒⠄⠯⠿⠯⠑⠹⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  output_test_stream ts{"output/score_tuplet_test4.ly"};
  ::bmc::lilypond_output_format(ts);
  ts << attribute;
  BOOST_REQUIRE(ts.match_pattern());
}

BOOST_AUTO_TEST_CASE(score_tuplet_test5) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠸⠶⠸⠶⠄⠐⠯⠓⠚⠙⠑⠙⠚⠮⠙⠋⠛⠓⠛⠋⠐⠵⠛⠊⠚⠙⠚⠊⠸⠶⠄⠷⠚⠩⠑⠋⠛⠋⠑⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_tuplet_test6) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠼⠃⠆ ⠆⠆⡸⠓⠐⠙⠋⡸⠓⠐⠙⠋⠶⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

// Doubled tuplet across multiple measure
BOOST_AUTO_TEST_CASE(score_tuplet_test7) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"⠐⠹⠱⠆⠆⠙⠑⠙⠑⠙⠑ ⠙⠑⠙⠆⠑⠙⠑⠹⠹⠣⠅");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(score_tuplet_test8) {
  std::locale::global(std::locale(""));
  std::wstring const input {
    L"⠼⠙⠩⠼⠃⠆ ⠆⠆⠸⠓⠐⠙⠋⠸⠓⠐⠙⠋⠶ ⠶ ⠸⠊⠙⠋⠶⠸⠊⠡⠐⠑⠛⠶ ⠸⠓⠩⠚⠐⠛⠸⠓⠐⠙⠋⠸⠓⠐⠙⠑⠆⠸⠛⠚⠑⠣⠅"
  };
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  output_test_stream ts{"output/score_tuplet_test8.ly"};
  ::bmc::lilypond_output_format(ts);
  ts << attribute;
  BOOST_CHECK(ts.match_pattern());
}

struct slur_count_of_first_note : boost::static_visitor<std::size_t> {
  result_type operator()(::bmc::braille::ast::measure const &measure) const {
    for (auto const &voice: measure.voices) {
      for (auto const &partial_measure: voice) {
        for (auto const &partial_voice: partial_measure) {
          if (!partial_voice.empty()) return apply_visitor(*this, partial_voice.front());
        }
      }
    }
    return 0;
  }
  result_type operator()(::bmc::braille::ast::key_and_time_signature const &) const { return 0; }

  result_type operator()(::bmc::braille::ast::note const &note) const
  {
    return note.slurs.size();
  }
  template<typename T> result_type operator()(T const &) const { return 0; }
};

BOOST_AUTO_TEST_CASE(slur_test1) {
  std::wstring const input(L"!rcr2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(apply_visitor(slur_count_of_first_note(), attribute.parts[0][0].paragraphs[0][0]), 1);
}

BOOST_AUTO_TEST_CASE(score_multiple_time_sigs) {
  std::locale::global(std::locale(""));
  std::wstring const input(L"#c/!,#ah,+\n!n.2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].paragraphs.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.time_sigs.size(), 2);
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
}

BOOST_AUTO_TEST_CASE(common_factor) {
  ::bmc::rational const a(1, 4), b(1, 8), c(1, 12), d(1, 24);
  BOOST_CHECK_EQUAL(boost::integer::gcd(a, b), b);
  BOOST_CHECK_EQUAL(boost::integer::gcd(b, c), d);
  BOOST_CHECK_EQUAL(boost::integer::lcm(b, c), a);
}

#include "bmc/musicxml.hpp"
#include "bmc/braille/reformat.hpp"
#include <boost/locale/encoding_utf.hpp>
using boost::locale::conv::utf_to_utf;

bool test_reformat(bmc::braille::ast::score const &score, unsigned width, std::string const &ly_file, std::string const &musicxml_file) {
  bmc::braille::format_style style;

  style.columns = width;

  std::stringstream ss;
  
  ss << bmc::braille::reformat(score, style);

  BOOST_REQUIRE(!ss.str().empty());

  std::u32string unicode{utf_to_utf<char32_t>(ss.str())};
  {
    std::u32string::const_iterator cur = unicode.begin();
    while (cur != unicode.cend()) {
      auto next = std::find(cur, unicode.cend(), U'\n');
      BOOST_REQUIRE_LE(std::distance(cur, next), width);
      if (cur != unicode.cend()) cur = next + 1;
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(bwv988_v01) {
  std::ifstream file{"input/bwv988-v01.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin(file.rdbuf()), file_end;
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK(!attribute.time_sigs.empty());
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(7));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 4));
//BOOST_CHECK_EQUAL(duration(attribute.parts[0][0]), ::bmc::rational(3, 4) * 32);

  {
    output_test_stream ts{"output/bwv988-v01.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // LilyPond output including comments pointing back to braille locations
    output_test_stream ts{"output/bwv988-v01.ly.locations.expected"};
    ::bmc::lilypond_output_format(ts);
    ::bmc::include_locations_for_lilypond(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v01.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }

  for (unsigned i = 25; i <= 88; ++i)
    BOOST_REQUIRE(test_reformat(attribute, i, "output/bwv988-v01.ly",
                                "output/bwv988-v01.xml"));
}

BOOST_AUTO_TEST_CASE(bwv988_v02) {
  std::ifstream file{"input/bwv988-v02.bmc"};
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<char> file_begin(file.rdbuf()), file_end;
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(5));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(2, 4));

  {
    output_test_stream ts{"output/bwv988-v02.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v02.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }

  for (unsigned i = 25; i <= 88; ++i)
    BOOST_REQUIRE(test_reformat(attribute, i, "output/bwv988-v02.ly",
                                "output/bwv988-v02.xml"));
}

BOOST_AUTO_TEST_CASE(bwv988_v03) {
  std::ifstream file{"input/bwv988-v03.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin(file.rdbuf()), file_end;
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(5));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(16));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(16));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(12, 8));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v03.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v03.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }

  for (unsigned i = 25; i <= 88; ++i)
    BOOST_REQUIRE(test_reformat(attribute, i, "output/bwv988-v03.ly",
                                "output/bwv988-v03.xml"));
}

BOOST_AUTO_TEST_CASE(bwv988_v04) {
  std::ifstream file{"input/bwv988-v04.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin(file.rdbuf()), file_end;
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(4));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(34));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(34));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 8));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v04.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v04.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }

  for (unsigned i = 25; i <= 88; ++i)
    BOOST_REQUIRE(test_reformat(attribute, i, "output/bwv988-v04.ly",
                                "output/bwv988-v04.xml"));
}

BOOST_AUTO_TEST_CASE(bwv988_v05) {
  std::ifstream file{"input/bwv988-v05.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin(file.rdbuf()), file_end;
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(4));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v05.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v05.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }

  for (unsigned i = 25; i <= 88; ++i)
    BOOST_REQUIRE(test_reformat(attribute, i, "output/bwv988-v05.ly",
                                "output/bwv988-v05.xml"));
}

BOOST_AUTO_TEST_CASE(bwv988_v06) {
  std::ifstream file{"input/bwv988-v06.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(36));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(36));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v06.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v06.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v07) {
  std::ifstream file{"input/bwv988-v07.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v07.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v07.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v08) {
  std::ifstream file{"input/bwv988-v08.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v08.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v08.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v09) {
  std::ifstream file{"input/bwv988-v09.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(16));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(16));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v09.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v09.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v10) {
  std::ifstream file{"input/bwv988-v10.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(30));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  {
    output_test_stream ts{"output/bwv988-v10.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v10.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v11) {
  std::ifstream file{"input/bwv988-v11.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v11.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v11.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v12) {
  std::ifstream file{"input/bwv988-v12.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
//BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(2));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(attribute.parts.size(), attribute.unfolded_part.size());

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v12.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v12.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v13) {
  std::ifstream file{"input/bwv988-v13.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(8));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 4));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v13.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v13.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v16) {
  std::ifstream file{"input/bwv988-v16.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(8));
  BOOST_REQUIRE(attribute.parts[0][0].number);
  BOOST_CHECK_EQUAL(*attribute.parts[0][0].number, 1);
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v16.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v16.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v13_de) {
  std::ifstream file{"input/bwv988-v13.de.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(8));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 4));
}

BOOST_AUTO_TEST_CASE(bwv988_v14) {
  std::ifstream file{"input/bwv988-v14.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(6));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 4));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v14.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v14.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v15) {
  std::ifstream file{"input/bwv988-v15.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, -2);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(7));
//BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
//BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(1, 2));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v15.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v15.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v17) {
  std::ifstream file{"input/bwv988-v17.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(8));
  BOOST_REQUIRE(attribute.parts[0][0].number);
  BOOST_CHECK_EQUAL(*attribute.parts[0][0].number, 1);
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v17.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v17.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v18) {
  std::ifstream file{"input/bwv988-v18.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(4));
  BOOST_REQUIRE(attribute.parts[0][0].number);
  BOOST_CHECK_EQUAL(*attribute.parts[0][0].number, 1);
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v18.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v18.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v19) {
  std::ifstream file{"input/bwv988-v19.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(4));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(3, 8));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v19.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v19.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v22) {
  std::ifstream file{"input/bwv988-v22.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(4));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(1));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v22.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v22.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}

BOOST_AUTO_TEST_CASE(bwv988_v30) {
  std::ifstream file{"input/bwv988-v30.bmc"};
  BOOST_REQUIRE(file.good());
  std::istreambuf_iterator<char> file_begin{file.rdbuf()}, file_end{};
  auto const input = utf_to_utf<wchar_t>(std::string(file_begin, file_end));
  BOOST_REQUIRE(!input.empty());
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_REQUIRE(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_REQUIRE_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_REQUIRE_EQUAL(attribute.parts[0].size(), std::size_t(4));
  for (int i = 0; i < attribute.parts[0].size(); ++i)
    BOOST_CHECK_EQUAL(attribute.parts[0][i].paragraphs.size(), std::size_t(2));
  ::bmc::braille::compiler<error_handler_type> compile(errors);
  BOOST_REQUIRE(compile(attribute));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][0]), ::bmc::rational(1, 8));
  BOOST_CHECK_EQUAL(duration(attribute.parts[0][0].paragraphs[0][1]), ::bmc::rational(4, 4));

  { // LilyPond output:
    output_test_stream ts{"output/bwv988-v30.ly"};
    ::bmc::lilypond_output_format(ts);
    ts << attribute;
    BOOST_CHECK(ts.match_pattern());
  }

  { // MusicXML output:
    output_test_stream ts{"output/bwv988-v30.xml"};
    ::bmc::musicxml(ts, attribute);
    BOOST_CHECK(ts.match_pattern());
  }
}
