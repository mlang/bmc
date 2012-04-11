// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "config.hpp"
#include "time_signature.hpp"
#include <boost/spirit/include/qi.hpp>
#include "ttb/ttb.h"

#define BOOST_TEST_MODULE bmc_test
#include <boost/test/included/unit_test.hpp>

#ifdef CMAKE_SOURCE_DIR
#define DIR CMAKE_SOURCE_DIR
#else
#define DIR ""
#endif

BOOST_AUTO_TEST_CASE(time_signature_grammar_test_1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"#ab(");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::time_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, music::time_signature(12, 8));
  BOOST_CHECK_EQUAL(attribute, music::rational(3, 2));
  BOOST_CHECK_EQUAL(attribute.numerator(), 12);
  destroyTextTable(textTable);
}

#include "key_signature.hpp"

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, 0);
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_2) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"#f3");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, 6);
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(key_signature_grammar_test_3) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"22");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::key_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, -2);
  destroyTextTable(textTable);
}

#include "spirit/qi/primitive/brl.hpp"

BOOST_AUTO_TEST_CASE(brl_parser_test) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"#A");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  music::braille::brl_type brl;
  BOOST_CHECK(boost::spirit::qi::parse(begin, end, brl(3456) >> brl(1)));
  BOOST_CHECK(begin == end);
  destroyTextTable(textTable);
}

#include "measure.hpp"

class is_rest: public boost::static_visitor<bool> {
public:
  template<typename T>
  result_type operator()(T const&) const
  { return false; }
  result_type operator()(music::braille::ambiguous::rest const&) const
  { return true; }
};

BOOST_AUTO_TEST_CASE(measure_test1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"vu.");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(2));
  BOOST_CHECK(apply_visitor(is_rest(), attribute.voices[0][0][0][0]));
  BOOST_CHECK(apply_visitor(is_rest(), attribute.voices[0][0][0][1]));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(measure_test2) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][1][1].size(), std::size_t(2));
  BOOST_CHECK(apply_visitor(is_rest(), attribute.voices[1][0][0][0]));
  BOOST_CHECK(apply_visitor(is_rest(), attribute.voices[1][1][1][0]));
  destroyTextTable(textTable);
}

#include "compiler.hpp"

struct get_line : boost::static_visitor<int>
{
  result_type operator()(music::braille::ambiguous::locatable const& lexeme) const
  { return lexeme.line; }
  result_type operator()(music::braille::ambiguous::barline const&) const { return 0; }
  result_type operator()(music::braille::hand_sign const&) const { return 0; }
  result_type operator()(music::braille::ambiguous::simile const&) const { return 0; }
  result_type operator()(music::braille::ambiguous::value_distinction const&) const { return 0; }
};

struct get_column : boost::static_visitor<int>
{
  result_type operator()(music::braille::ambiguous::locatable const& lexeme) const
  { return lexeme.column; }
  result_type operator()(music::braille::ambiguous::barline const&) const { return 0; }
  result_type operator()(music::braille::hand_sign const&) const { return 0; }
  result_type operator()(music::braille::ambiguous::simile const&) const { return 0; }
  result_type operator()(music::braille::ambiguous::value_distinction const&) const { return 0; }
};

BOOST_AUTO_TEST_CASE(measure_interpretations_test1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"_r72`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  music::braille::compiler<error_handler_type> compile(errors);
  compile.global_time_signature = music::time_signature(3, 4);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_line(), attribute.voices[0][0][0][0]), 1);
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_column(), attribute.voices[0][0][0][0]), 1);

  BOOST_CHECK_EQUAL(boost::apply_visitor(get_line(), attribute.voices[0][0][0][1]), 1);
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_column(), attribute.voices[0][0][0][1]), 3);

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(measure_interpretations_test2) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.voices[1][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[1][0][0].size(), std::size_t(1));
  music::braille::compiler<error_handler_type> compile(errors);
  compile.global_time_signature = music::time_signature(3, 4);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(notegroup_test1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.voices.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.voices[0][0][0].size(), std::size_t(9));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

struct get_type : boost::static_visitor<music::rational>
{
  result_type operator()(music::braille::ambiguous::note const& note) const
  { return note.type; }
  result_type operator()(music::braille::ambiguous::rest const& rest) const
  { return rest.type; }
  result_type operator()(music::braille::ambiguous::chord const& chord) const
  { return (*this)(chord.base); }
  template<typename T>
  result_type operator()(T const&) const
  { return music::zero; }
};

BOOST_AUTO_TEST_CASE(compiler_test1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.voices.size() == 1);
  BOOST_CHECK(attribute.voices[0].size() == 1);
  BOOST_CHECK(attribute.voices[0][0].size() == 1);
  BOOST_CHECK(attribute.voices[0][0][0].size() == 9);
  BOOST_CHECK_EQUAL(errors.iters.size(), std::size_t(19));
  BOOST_CHECK(errors.iters[0] == input.begin());
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][0]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][1]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][2]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][3]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][4]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][5]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][6]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][7]), music::rational(1, 16));
  BOOST_CHECK_EQUAL(boost::apply_visitor(get_type(), attribute.voices[0][0][0][8]), music::rational(1, 2));
  destroyTextTable(textTable);
}

#include "score.hpp"

BOOST_AUTO_TEST_CASE(score_solo_test1) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::wstring const input(L"⠨⠽⠅⠐⠯⠃⠵⠁⠯⠃⠽⠁⠯⠃⠵⠁⠯⠃⠨⠽⠐⠯⠵⠯⠽⠯⠵⠯ ⠮⠅⠿⠇⠡⠯⠃⠿⠇⠽⠁⠿⠇⠯⠃⠿⠇⠮⠿⠯⠿⠽⠿⠯⠿2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(2));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(score_solo_test2) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wstring const input(L"#c/\n⠐⠞⠃⠝⠞⠎⠚⠂⠈⠉⠞⠟⠗⠁⠎⠾⠽⠐⠢⠕⠽⠚⠊⠓2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(1));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(rational_gcd) {
  music::rational a(1, 4), b(1, 8);
  BOOST_CHECK_EQUAL(music::gcd(a, b), b);
  BOOST_CHECK_EQUAL(music::gcd(music::rational(1, 8), music::rational(1, 12)),
                    music::rational(1, 24));
}

#include <fstream>

BOOST_AUTO_TEST_CASE(bwv988_v01) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v01.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK(attribute.time_sig);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(bwv988_v02) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v02.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(34));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(34));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(bwv988_v03) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v03.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(16));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(16));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(bwv988_v13) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v13.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(bwv988_v13_de) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v13.de.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(bwv988_v30) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v30.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(18));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(18));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

#include "lilypond.hpp"
#include <sstream>

BOOST_AUTO_TEST_CASE(bwv988_v01_ly) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v01.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  std::stringstream ss;
  music::lilypond_output_format(ss);
  ss << attribute;

  std::ifstream ly_file(DIR "input/bwv988-v01.ly.expected");
  BOOST_CHECK(ly_file.good());
  std::istreambuf_iterator<char> in_begin(ly_file.rdbuf()), in_end;
  std::string expected(in_begin, in_end);
  BOOST_CHECK_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE(bwv988_v13_ly) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v13.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(32));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(32));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  std::stringstream ss;
  music::lilypond_output_format(ss);
  ss << attribute;

  std::ifstream ly_file(DIR "input/bwv988-v13.ly.expected");
  BOOST_CHECK(ly_file.good());
  std::istreambuf_iterator<char> in_begin(ly_file.rdbuf()), in_end;
  std::string expected(in_begin, in_end);
  BOOST_CHECK_EQUAL(ss.str(), expected);
}

BOOST_AUTO_TEST_CASE(bwv988_v30_ly) {
  textTable = compileTextTable(DIR "ttb/Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file(DIR "input/bwv988-v30.bmc");
  BOOST_CHECK(file.good());
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type errors(begin, end);
  parser_type parser(errors);
  boost::spirit::traits::attribute_of<parser_type>::type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.key_sig, 1);
  BOOST_CHECK_EQUAL(attribute.parts.size(), std::size_t(1));
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), std::size_t(2));
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), std::size_t(18));
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), std::size_t(18));
  music::braille::compiler<error_handler_type> compile(errors);
  BOOST_CHECK(compile(attribute));

  std::stringstream ss;
  music::lilypond_output_format(ss);
  ss << attribute;

  std::ifstream ly_file(DIR "input/bwv988-v30.ly.expected");
  BOOST_CHECK(ly_file.good());
  std::istreambuf_iterator<char> in_begin(ly_file.rdbuf()), in_end;
  std::string expected(in_begin, in_end);
  BOOST_CHECK_EQUAL(ss.str(), expected);
}

