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

BOOST_AUTO_TEST_CASE(time_signature_1) {
  std::u32string const input(U"#ab(");
  auto result = bmc::braille::parse_time_signature(input, std::cout);
  auto &ast = std::get<0>(result);
  BOOST_REQUIRE(ast);
  BOOST_CHECK_EQUAL(ast->numerator, 12);
  BOOST_CHECK_EQUAL(ast->denominator, 8);
  //BOOST_CHECK_EQUAL(*ast, bmc::rational(3, 2));
}

