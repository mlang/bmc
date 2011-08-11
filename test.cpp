// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "numbers.hpp"
#include <boost/spirit/include/qi.hpp>
#include "ttb.h"

#define BOOST_TEST_MODULE bmc_test
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(time_signature_grammar_test) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"#ab(");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::time_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute, music::time_signature(12, 8));
  BOOST_CHECK_EQUAL(attribute, music::rational(3, 2));
  BOOST_CHECK_EQUAL(attribute.numerator(), 12);
  destroyTextTable(textTable);
}

#include "brl.hpp"

BOOST_AUTO_TEST_CASE(brl_parser_test) {
  textTable = compileTextTable("Tables/de.ttb");
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
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"vu.");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.size(), 1);
  BOOST_CHECK_EQUAL(attribute[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[0][0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[0][0][0].size(), 2);
  BOOST_CHECK(apply_visitor(is_rest(), attribute[0][0][0][0]));
  BOOST_CHECK(apply_visitor(is_rest(), attribute[0][0][0][1]));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(measure_test2) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.size(), 2);
  BOOST_CHECK_EQUAL(attribute[0].size(), 1);
  destroyTextTable(textTable);
}

#include "compiler.hpp"

BOOST_AUTO_TEST_CASE(measure_interpretations_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"_r72`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.size(), 2);
  BOOST_CHECK_EQUAL(attribute[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[1].size(), 2);
  BOOST_CHECK_EQUAL(attribute[1][0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[1][0][0].size(), 1);
  music::braille::compiler compile(errors);
  compile.global_time_signature = music::time_signature(3, 4);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(measure_interpretations_test2) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.size(), 2);
  BOOST_CHECK_EQUAL(attribute[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[1].size(), 2);
  BOOST_CHECK_EQUAL(attribute[1][0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[1][0][0].size(), 1);
  music::braille::compiler compile(errors);
  compile.global_time_signature = music::time_signature(3, 4);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(notegroup_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.size(), 1);
  BOOST_CHECK_EQUAL(attribute[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[0][0].size(), 1);
  BOOST_CHECK_EQUAL(attribute[0][0][0].size(), 9);
  music::braille::compiler compile(errors);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

class is_type
: public boost::static_visitor<bool>
{
  music::rational expected;
public:
  is_type(music::rational const& expected)
  : expected(expected)
  {}
  bool operator()(music::braille::ambiguous::note const& note) const
  { return note.type == expected; }
  bool operator()(music::braille::ambiguous::rest const& rest) const
  { return rest.type == expected; }
  bool operator()(music::braille::ambiguous::chord const& chord) const
  { return (*this)(chord.base); }
  template<typename T>
  bool operator()(T const&) const
  { return false; }
};

BOOST_AUTO_TEST_CASE(compiler_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"!yefg{ijd_n");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 9);
  BOOST_CHECK_EQUAL(errors.iters.size(), 19);
  BOOST_CHECK(errors.iters[0] == input.begin());
  music::braille::compiler compile(errors);
  BOOST_CHECK(compile(attribute));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][0]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][1]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][2]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][3]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][4]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][5]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][6]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 16)),
                                   attribute[0][0][0][7]));
  BOOST_CHECK(boost::apply_visitor(is_type(music::rational(1, 2)),
                                   attribute[0][0][0][8]));
  destroyTextTable(textTable);
}

#include "score.hpp"

BOOST_AUTO_TEST_CASE(score_solo__test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"⠨⠽⠅⠐⠯⠃⠵⠁⠯⠃⠽⠁⠯⠃⠵⠁⠯⠃⠨⠽⠐⠯⠵⠯⠽⠯⠵⠯ ⠮⠅⠿⠇⠡⠯⠃⠿⠇⠽⠁⠿⠇⠯⠃⠿⠇⠮⠿⠯⠿⠽⠿⠯⠿2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.parts.size(), 1);
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), 2);
  music::braille::compiler compile(errors);
  BOOST_CHECK(compile(attribute));

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(score_solo_test2) {
  textTable = compileTextTable("Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wstring const input(L"#c/\n⠐⠞⠃⠝⠞⠎⠚⠂⠈⠉⠞⠟⠗⠁⠎⠾⠽⠐⠢⠕⠽⠚⠊⠓2k");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.parts.size(), 1);
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), 1);
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), 1);
  music::braille::compiler compile(errors);
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

BOOST_AUTO_TEST_CASE(bwv_988_v01) {
  textTable = compileTextTable("Tables/de.ttb");
  std::locale::global(std::locale(""));
  std::wifstream file("input/bwv988-v01.bmc");
  std::istreambuf_iterator<wchar_t> file_begin(file.rdbuf()), file_end;
  std::wstring const input(file_begin, file_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::score_grammar<iterator_type> parser_type;
  music::braille::error_handler<iterator_type> errors(begin, end);
  parser_type parser(errors);
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK_EQUAL(attribute.parts.size(), 1);
  BOOST_CHECK_EQUAL(attribute.parts[0].size(), 2);
  BOOST_CHECK_EQUAL(attribute.parts[0][0].size(), 32);
  BOOST_CHECK_EQUAL(attribute.parts[0][1].size(), 32);
  music::braille::compiler compile(errors);
  BOOST_CHECK(compile(attribute));
//BOOST_CHECK_EQUAL(music::braille::duration(attribute.parts[0][0][0]),
//                  music::rational(3, 4));
  destroyTextTable(textTable);
}

