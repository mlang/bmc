#include <bmc/braille/parser/ast_visitor.hpp>
#include <bmc/braille/parser/parser.hpp>
#define BOOST_TEST_MODULE bmc_test
#if defined(BOOST_TEST_DYN_LINK)
#include <boost/test/unit_test.hpp>
#else
#include <boost/test/included/unit_test.hpp>
#endif

struct text_table
{
  text_table() { bmc::braille::default_table = "de"; }
};

BOOST_GLOBAL_FIXTURE(text_table);

BOOST_AUTO_TEST_CASE(time_signature_1)
{
  std::u32string const input(U"#ab(");
  auto result = bmc::braille::parse_time_signature(input, std::cout);
  auto &ast = std::get<0>(result);
  auto const &error_handler = std::get<1>(result);
  BOOST_REQUIRE(ast);
  BOOST_CHECK_EQUAL(ast->numerator, 12);
  BOOST_CHECK_EQUAL(ast->denominator, 8);
  auto const position = error_handler.position_of(*ast);
  BOOST_CHECK(!position.empty());
  BOOST_CHECK(position.begin() == input.begin());
  BOOST_CHECK(position.end() == input.end());
  std::cout << ast->id_first << ' ' << ast->id_last << std::endl;
  //BOOST_CHECK_EQUAL(*ast, bmc::rational(3, 2));
}

BOOST_AUTO_TEST_CASE(key_signature_1)
{
  auto test_key_signature = [](std::u32string input, int fifths)
  {
    auto result = bmc::braille::parse_key_signature(input, std::cout);
    auto &ast = std::get<0>(result);
    BOOST_REQUIRE(ast);
    BOOST_CHECK_EQUAL(ast->fifths, fifths);
  };
  test_key_signature(U"", 0);
  test_key_signature(U"3", 1);
  test_key_signature(U"2", -1);
  test_key_signature(U"#d3", 4);
  test_key_signature(U"#d2", -4);
}

BOOST_AUTO_TEST_CASE(key_and_time_signature_1)
{
  auto test_key_and_time_signature = [](std::u32string input, int fifths, int numerator, int denominator)
  {
    auto result = bmc::braille::parse_key_and_time_signature(input, std::cout);
    auto &ast = std::get<0>(result);
    BOOST_REQUIRE(ast);
    BOOST_CHECK_EQUAL(ast->key.fifths, fifths);
    BOOST_CHECK_EQUAL(ast->time.numerator, numerator);
    BOOST_CHECK_EQUAL(ast->time.denominator, denominator);
  };
  test_key_and_time_signature(U"#d/", 0, 4, 4);
  test_key_and_time_signature(U"3#c/", 1, 3, 4);
  test_key_and_time_signature(U"2#f(", -1, 6, 8);
  test_key_and_time_signature(U"#d3#d/", 4, 4, 4);
  test_key_and_time_signature(U"#d2#d/", -4, 4, 4);
}

BOOST_AUTO_TEST_CASE(note_1)
{
  auto test_note = [](std::u32string input, boost::optional<bmc::accidental> acc, bmc::diatonic_step step, bmc::braille::parser::ast::ambiguous_value value)
  {
    auto result = bmc::braille::parse_note(input, std::cout, "", false);
    auto &ast = std::get<0>(result);
    BOOST_REQUIRE(ast);
    BOOST_CHECK_EQUAL(ast->acc, acc);
    BOOST_CHECK_EQUAL(ast->step, step);
    BOOST_CHECK_EQUAL(ast->ambiguous_value, value);
  };
  auto const no_accidental = boost::optional<bmc::accidental>{};
  test_note(U"2$y.", bmc::flat, bmc::C, bmc::braille::parser::ast::whole_or_16th);
}

BOOST_AUTO_TEST_CASE(score_1)
{
  std::u32string const input = U"!y2k";
  auto result = bmc::braille::parse_score(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_REQUIRE_EQUAL(ast->parts.size(), 1);
}

BOOST_AUTO_TEST_CASE(score_2)
{
  std::u32string const input = 
    U"  #a !y rr 8888\n"
    U"44442k\n"
    ;
  auto result = bmc::braille::parse_score(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_REQUIRE_EQUAL(ast->parts.size(), 1);
}

BOOST_AUTO_TEST_CASE(score_3)
{
  std::u32string const input =
    U"  #a #,-: !y rr 8888\n"
    U"  #b #/-? !z 88882k\n"
    ;
  auto result = bmc::braille::parse_score(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_REQUIRE_EQUAL(ast->parts.size(), 1);
  BOOST_REQUIRE_EQUAL(ast->parts[0].size(), 2);
}

BOOST_AUTO_TEST_CASE(score_4)
{
  std::u32string const input =
    U"  #a #,-: $`!y rr 88882k\n"
    U"  _`>z 8888 rr2k\n"
    ;
  auto result = bmc::braille::parse_score(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_REQUIRE_EQUAL(ast->parts.size(), 1);
  BOOST_REQUIRE_EQUAL(ast->parts[0].size(), 1);
  BOOST_REQUIRE_EQUAL(ast->parts[0][0].paragraphs.size(), 2);
}

BOOST_AUTO_TEST_CASE(visitor_1)
{
  class visitor : public bmc::braille::parser::ast::const_visitor<visitor>
  {
    std::size_t count = 0;
  public:
    bool visit_rhythmic(bmc::braille::parser::ast::rhythmic const& rhythmic)
    {
      count++;
      return true;
    }
    std::size_t get_count() const { return count; }
  } count_rhythmic;
  bmc::braille::parser::ast::score s;
  BOOST_REQUIRE(count_rhythmic.traverse_score(s));
  BOOST_CHECK_EQUAL(count_rhythmic.get_count(), 0);
}

BOOST_AUTO_TEST_CASE(tuplet_start_1)
{
  std::u32string const input = U"#=#=.!ydddddd==#=.=2k";
  auto result = bmc::braille::parse_score(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_REQUIRE_EQUAL(ast->parts.size(), 1);
  BOOST_REQUIRE_EQUAL(ast->parts[0].size(), 1);
  BOOST_REQUIRE_EQUAL(ast->parts[0][0].paragraphs.size(), 1);
}

