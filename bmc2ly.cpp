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
#include <boost/program_options.hpp>

#include "bmc/lilypond.hpp"

int bmc2ly(std::wistream &wistream, bool include_locations, std::string instrument) {
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
      music::lilypond::generator generate(std::cout, true, true, include_locations);
      if (not instrument.empty()) generate.instrument(instrument);
      generate(score);

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

  using namespace boost::program_options;
  std::string instrument;
  bool locations;
  std::vector<std::string> input_files;

  options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "print usage message")
  ("input-file", value(&input_files), "input file")
  ("instrument,i", value(&instrument), "default MIDI instrument")
  ("locations,l", bool_switch(&locations), "Include braille locations in LilyPond output")
  ;
  positional_options_description positional_desc;
  positional_desc.add("input-file", -1);

  variables_map vm;
  try {
    store(command_line_parser(argc, argv)
          .options(desc).positional(positional_desc).run(), vm);
    notify(vm);
  } catch (unknown_option) {
    std::cerr << "Unknown option" << std::endl << desc << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  {
    char *localeTable = selectTextTable(TABLES_DIRECTORY);
    if (localeTable) {
      replaceTextTable(TABLES_DIRECTORY, localeTable);
      free(localeTable);
    }
  }

  for (auto const &file: input_files) {
    if (file == "-") bmc2ly(std::wcin, locations, instrument);
    else {
      std::wifstream f(file);
      if (f.good()) bmc2ly(f, locations, instrument);
    }
  }

  return EXIT_FAILURE;
}

