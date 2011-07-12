#include "config.hpp"
#include "ambiguous.hpp"
#include "compiler.hpp"
#include "score.hpp"
#include <boost/spirit/include/qi_parse.hpp>
#include "ttb.h"

int
main()
{
  std::locale::global(std::locale(""));

  textTable = compileTextTable("Tables/de.ttb");

  std::istreambuf_iterator<wchar_t> wcin_begin = std::wcin.rdbuf();
  std::istreambuf_iterator<wchar_t> wcin_end;
  std::wstring source(wcin_begin, wcin_end);
  typedef std::wstring::const_iterator iterator_type;
  iterator_type iter = source.begin();
  iterator_type end = source.end();
  music::braille::error_handler<iterator_type> error_handler(iter, end);
  typedef music::braille::score_grammar<iterator_type> parser_type;
  parser_type parser(error_handler);                              // Our parser
  parser_type::start_type::attr_type score;

  bool const success = boost::spirit::qi::parse(iter, end, parser, score);

  if (success && iter == end) {
    music::braille::compiler compile(error_handler);
    if (compile(score)) {
      return 0;
    }
  } else {
    std::wcout << "Parse failure\n";
  }

  return 1;
}


