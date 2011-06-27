#include "numbers.hpp"
#include <boost/spirit/include/qi_parse.hpp>
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
  BOOST_CHECK(attribute == music::time_signature(12, 8));
  BOOST_CHECK(attribute == music::rational(3, 2));
  BOOST_CHECK(attribute.numerator() == 12);
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
  result_type operator()(music::braille::ambiguous::note const&) const {
    return false;
  }
  result_type operator()(music::braille::ambiguous::chord const&) const {
    return false;
  }
  result_type operator()(music::braille::ambiguous::rest const&) const {
    return true;
  }
  result_type operator()(music::braille::ambiguous::value_distinction const&) const {
    return false;
  }
};

BOOST_AUTO_TEST_CASE(measure_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"vu.");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 2);
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
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 2);
  BOOST_CHECK(attribute[0].size() == 1);
  destroyTextTable(textTable);
}

