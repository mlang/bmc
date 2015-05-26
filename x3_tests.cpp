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
  BOOST_REQUIRE(ast);
  BOOST_CHECK_EQUAL(ast->numerator, 12);
  BOOST_CHECK_EQUAL(ast->denominator, 8);
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

