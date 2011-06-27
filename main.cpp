#include "numbers.hpp"
#include "brlsym.hpp"
#include <boost/spirit/include/qi_parse.hpp>

TextTable *textTable;

int
main()
{
  std::locale::global(std::locale(""));

  textTable = compileTextTable("de.ttb");

  std::istreambuf_iterator<wchar_t> wcin_begin = std::wcin.rdbuf();
  std::istreambuf_iterator<wchar_t> wcin_end;
  std::wstring source(wcin_begin, wcin_end);
  source = L"dd";
  typedef std::wstring::const_iterator iterator_type;
  iterator_type iter = source.begin();
  iterator_type end = source.end();

  upper_number<iterator_type> parser;                              // Our parser
  unsigned ast;
  bool success = boost::spirit::qi::parse(iter, end, parser, ast);

  if (success && iter == end) {
    std::wcout << "Success\n" << ast << std::endl;
  } else {
    std::wcout << "Parse failure\n";
  }

  return 0;
}


