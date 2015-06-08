#include <bmc/braille/parser/parser.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <iostream>

using bmc::braille::parse_score;
using boost::locale::conv::utf_to_utf;
using std::cerr;
using std::cin;
using std::get;
using std::istreambuf_iterator;
using std::string;
using std::u32string;

int main()
{
  bmc::braille::default_table = "de";
  istreambuf_iterator<char> cin_begin{cin.rdbuf()}, cin_end;
  u32string const input{utf_to_utf<char32_t>(string{cin_begin, cin_end})};
  auto result = parse_score(input, cerr);
  auto &ast = get<0>(result);
  return ast? EXIT_SUCCESS: EXIT_FAILURE;
}
