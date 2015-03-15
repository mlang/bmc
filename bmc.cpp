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
#include "bmc/braille/reformat.hpp"
#include "bmc/braille/semantic_analysis.hpp"
#include <boost/program_options.hpp>

#include "bmc/lilypond.hpp"
#include "bmc/musicxml.hpp"

namespace {

int bmc2ly( std::wistream &wistream
          , bool lilypond, bool musicxml
          , bool include_locations, std::string instrument, bool no_tagline
          , ::bmc::braille::format_style const &style
          ) {
  std::istreambuf_iterator<wchar_t> wcin_begin(wistream.rdbuf()), wcin_end;
  std::wstring source(wcin_begin, wcin_end);
  typedef std::wstring::const_iterator iterator_type;

  iterator_type iter = source.begin();
  iterator_type const end = source.end();
  typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
  error_handler_type error_handler(iter, end);
  typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
  parser_type parser(error_handler);
  boost::spirit::traits::attribute_of<parser_type>::type score;

  bool const success = parse(iter, end, parser, score);

  if (success and iter == end) {
    ::bmc::braille::compiler<error_handler_type> compile(error_handler);
    if (compile(score)) {
      std::wcerr << error_handler;
      if (lilypond) {
        ::bmc::lilypond::generator generate(std::cout, true, true, include_locations);
        if (not instrument.empty()) generate.instrument(instrument);
        if (no_tagline) generate.remove_tagline();
        generate(score);
      } else if (musicxml) {
        ::bmc::musicxml(std::cout, score);
      } else {
        std::cout << ::bmc::braille::reformat(score, style);
      }

      return EXIT_SUCCESS;
    } else {
      std::wcerr << "Failed to compile:" << std::endl << error_handler << std::endl;
    }
  } else {
    std::wcerr << "Failed to Parse:" << std::endl << source << std::endl;
  }

  return EXIT_FAILURE;
}

} // namespace

int main(int argc, char const *argv[])
{
  std::locale::global(std::locale(""));

  using namespace boost::program_options;
  std::string instrument;
  bool locations;
  bool no_tagline = false;
  ::bmc::braille::format_style style;
  std::vector<std::string> input_files;

  options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "print usage message")
  ("input-file", value(&input_files), "input file")
  ("instrument,i", value(&instrument), "default MIDI instrument")
  ("lilypond", "Produce LilyPond output.")
  ("musicxml", "Produce MusicXML output.")
  ("locations,l", bool_switch(&locations), "Include braille locations in LilyPond output")
  ("no-tagline", bool_switch(&no_tagline)->default_value(false), "Supress LilyPond default tagline")
  ("width,w", value(&style.columns), "Line width for reformatting")
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

  int status = EXIT_SUCCESS;
  bool const do_lilypond { bool(vm.count("lilypond")) }
           , do_musicxml { bool(vm.count("musicxml")) };
  for (auto const &file: input_files) {
    if (file == "-") status = bmc2ly(std::wcin, do_lilypond, do_musicxml, locations, instrument, no_tagline, style);
    else {
      std::wifstream f(file);
      if (f.good()) status = bmc2ly(f, do_lilypond, do_musicxml, locations, instrument, no_tagline, style);
    }
  }

  return status;
}

