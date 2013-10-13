// Copyright (C) 2011-2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <bmc/ttb/ttb.h>

#include "config.hpp"
#include <fstream>
#include <boost/spirit/include/qi_parse.hpp>
#include "bmc/braille/parsing/grammar/score.hpp"
#include "bmc/braille/semantic_analysis.hpp"

#include "bmc/lilypond.hpp"

int bmc2ly(std::wistream &wistream, bool include_locations) {
  std::istreambuf_iterator<wchar_t> wcin_begin(wistream.rdbuf()), wcin_end;
  std::wstring source(wcin_begin, wcin_end);
  typedef std::wstring::const_iterator iterator_type;

  iterator_type iter = source.begin();
  iterator_type const end = source.end();
  typedef music::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type error_handler(iter, end);
  typedef music::braille::score_grammar<iterator_type> parser_type;
  parser_type parser(error_handler);
  boost::spirit::traits::attribute_of<parser_type>::type score;

  bool const success = parse(iter, end, parser, score);

  if (success and iter == end) {
    music::braille::compiler<error_handler_type> compile(error_handler);
    if (compile(score)) {
      music::lilypond_output_format(std::cout);
      if (include_locations) music::include_locations_for_lilypond(std::cout);
      std::cout << score;

      return EXIT_SUCCESS;
    }
  } else {
    std::wcerr << "Failed to Parse:" << std::endl << source << std::endl;
  }

  return EXIT_FAILURE;
}

int
main(int argc, char const *argv[])
{
  std::locale::global(std::locale(""));

  {
    char *localeTable = selectTextTable(TABLES_DIRECTORY);
    if (localeTable) {
      replaceTextTable(TABLES_DIRECTORY, localeTable);
      free(localeTable);
    }
  }

  bool include_locations = false;
  if (argc > 1) {
    if (argv[1] == std::string("-l")) {
      include_locations = true;
      if (argc == 2) return bmc2ly(std::wcin, include_locations);
    }
    if (argv[1] == std::string("-")) return bmc2ly(std::wcin, include_locations);
    std::wifstream file(argv[1]);
    if (file.good()) return bmc2ly(file, include_locations);
  } else return bmc2ly(std::wcin, include_locations);

  return EXIT_FAILURE;
}

