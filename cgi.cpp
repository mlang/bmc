// Copyright (C) 2013  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <bmc/ttb/ttb.h>

#include "config.hpp"
#include <fstream>
#include <boost/spirit/include/qi_parse.hpp>
#include <boost/locale/encoding_utf.hpp>
#include "bmc/braille/parsing/grammar/score.hpp"
#include "bmc/braille/semantic_analysis.hpp"
#include <boost/program_options.hpp>

#include "bmc/lilypond.hpp"

#include <cgicc/Cgicc.h>
#include <cgicc/CgiUtils.h>
#include <cgicc/HTMLClasses.h>
#include <cgicc/HTTPContentHeader.h>
#include <cgicc/XHTMLDoctype.h>

// Output the has of an arbitrary string in hex.
std::string hash(std::string const &string)
{
  std::stringstream stream;
  stream << std::hex << std::hash<std::string>()(string);
  return stream.str();
}

int bmc2ly(std::wistream &wistream, bool include_locations, std::string instrument, bool no_tagline) {
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
      if (no_tagline) generate.remove_tagline();
      generate(score);

      return EXIT_SUCCESS;
    } else {
      std::wcerr << "Failed to compile:" << std::endl << source << std::endl;
    }
  } else {
    std::wcerr << "Failed to Parse:" << std::endl << source << std::endl;
  }

  return EXIT_FAILURE;
}

cgicc::option table_option(std::string const &id, cgicc::Cgicc const &cgi) {
  cgicc::option o(id);
  o.set("value", id);
  if (cgi.getElement("table") != cgi.getElements().end() and
      cgi.getElement("table")->getValue() == id)
    o.set("selected", "selected");
  return o;
}

std::ostream &table_select(std::ostream &os, cgicc::Cgicc const &cgi) {
  os << cgicc::select().set("name", "table").set("id", "table")
     << table_option("brf", cgi)
     << table_option("ar", cgi)
     << cgicc::option("as").set("value", "as")
     << cgicc::option("awa").set("value", "awa")
     << cgicc::option("bg").set("value", "bg")
     << cgicc::option("bh").set("value", "bh")
     << cgicc::option("bn").set("value", "bn")
     << cgicc::option("bo").set("value", "bo")
     << cgicc::option("bra").set("value", "bra")
     << cgicc::option("cs").set("value", "cs")
     << cgicc::option("cy").set("value", "cy")
     << cgicc::option("da").set("value", "da")
     << cgicc::option("da-1252").set("value", "da-1252")
     << cgicc::option("da-lt").set("value", "da-lt")
     << table_option("de", cgi)
     << cgicc::option("dra").set("value", "dra")
     << cgicc::option("el").set("value", "el")
     << cgicc::option("en").set("value", "en")
     << cgicc::option("en_CA").set("value", "en_CA")
     << cgicc::option("en_GB").set("value", "en_GB")
     << cgicc::option("en-nabcc").set("value", "en-nabcc")
     << cgicc::option("en_US").set("value", "en_US")
     << cgicc::option("eo").set("value", "eo")
     << cgicc::option("es").set("value", "es")
     << cgicc::option("et").set("value", "et")
     << cgicc::option("fi").set("value", "fi")
     << cgicc::option("fr").set("value", "fr")
     << cgicc::option("fr-2007").set("value", "fr-2007")
     << cgicc::option("fr_CA").set("value", "fr_CA")
     << cgicc::option("fr-cbifs").set("value", "fr-cbifs")
     << cgicc::option("fr_FR").set("value", "fr_FR")
     << cgicc::option("fr-vs").set("value", "fr-vs")
     << cgicc::option("ga").set("value", "ga")
     << cgicc::option("gd").set("value", "gd")
     << cgicc::option("gon").set("value", "gon")
     << cgicc::option("gu").set("value", "gu")
     << cgicc::option("he").set("value", "he")
     << cgicc::option("hi").set("value", "hi")
     << cgicc::option("hr").set("value", "hr")
     << cgicc::option("hu").set("value", "hu")
     << cgicc::option("hy").set("value", "hy")
     << cgicc::option("is").set("value", "is")
     << cgicc::option("it").set("value", "it")
     << cgicc::option("kha").set("value", "kha")
     << cgicc::option("kn").set("value", "kn")
     << cgicc::option("kok").set("value", "kok")
     << cgicc::option("kru").set("value", "kru")
     << cgicc::option("lt").set("value", "lt")
     << cgicc::option("lv").set("value", "lv")
     << cgicc::option("mg").set("value", "mg")
     << cgicc::option("mi").set("value", "mi")
     << cgicc::option("ml").set("value", "ml")
     << cgicc::option("mni").set("value", "mni")
     << cgicc::option("mr").set("value", "mr")
     << cgicc::option("mt").set("value", "mt")
     << cgicc::option("mun").set("value", "mun")
     << cgicc::option("mwr").set("value", "mwr")
     << cgicc::option("ne").set("value", "ne")
     << cgicc::option("new").set("value", "new")
     << cgicc::option("nl").set("value", "nl")
     << cgicc::option("nl_BE").set("value", "nl_BE")
     << cgicc::option("nl_NL").set("value", "nl_NL")
     << cgicc::option("no").set("value", "no")
     << cgicc::option("no-generic").set("value", "no-generic")
     << cgicc::option("no-oub").set("value", "no-oub")
     << cgicc::option("nwc").set("value", "nwc")
     << cgicc::option("or").set("value", "or")
     << cgicc::option("pa").set("value", "pa")
     << cgicc::option("pi").set("value", "pi")
     << cgicc::option("pl").set("value", "pl")
     << cgicc::option("pt").set("value", "pt")
     << cgicc::option("ro").set("value", "ro")
     << cgicc::option("ru").set("value", "ru")
     << cgicc::option("sa").set("value", "sa")
     << cgicc::option("sat").set("value", "sat")
     << cgicc::option("sd").set("value", "sd")
     << cgicc::option("sk").set("value", "sk")
     << cgicc::option("sv").set("value", "sv")
     << cgicc::option("sv-1989").set("value", "sv-1989")
     << cgicc::option("sv-1996").set("value", "sv-1996")
     << cgicc::option("sw").set("value", "sw")
     << cgicc::option("ta").set("value", "ta")
     << cgicc::option("te").set("value", "te")
     << cgicc::option("tr").set("value", "tr")
     << cgicc::option("vi").set("value", "vi")
     << cgicc::select();
  return os;
}

int
main(int argc, char const *argv[])
{
  std::locale::global(std::locale(""));
  cgicc::Cgicc cgi;

  if (cgi.getElement("table") != cgi.getElements().end()) {
    replaceTextTable(TABLES_DIRECTORY, cgi.getElement("table")->getValue().c_str());
  }

  cgicc::textarea music_input(cgi("music"));
  music_input.set("id", "music");
  music_input.set("cols", "32");
  music_input.set("rows", "10");
  music_input.set("name", "music");
  cgicc::const_form_iterator braille(cgi.getElement("music"));
  std::string prefix;
  if (braille != cgi.getElements().end()) {
    std::wstring source(boost::locale::conv::utf_to_utf<wchar_t>(braille->getValue()));
    typedef std::wstring::const_iterator iterator_type;
    iterator_type const end = source.end();
    iterator_type iter = source.begin();
    typedef music::braille::error_handler<iterator_type> error_handler_type;
    error_handler_type error_handler(iter, end);
    typedef music::braille::score_grammar<iterator_type> parser_type;
    parser_type parser(error_handler);
    boost::spirit::traits::attribute_of<parser_type>::type score;

    bool const success = parse(iter, end, parser, score);

    if (success and iter == end) {
      music::braille::compiler<error_handler_type> compile(error_handler);
      if (compile(score)) {
        prefix = hash(braille->getValue());
        std::string dir("/tmp/bmc.cgi/");
        std::ofstream bmc(dir + prefix + ".bmc");
        bmc << braille->getValue();
        bmc.close();
        std::ofstream ly(dir + prefix + ".ly");
        music::lilypond::generator generate(ly, true, true, false);
        generate.remove_tagline();
        generate(score);
        ly.close();        
        std::string cmd("lilypond -lNONE --png -dpaper-size='\"a5landscape\"' -o " + dir + prefix + " " + dir + prefix + ".ly");
        if (system(cmd.c_str()) != 0) {
          prefix = "";
        }
      }
    }
  }
  if (not prefix.empty() and
      cgi.getElement("type") != cgi.getElements().end()) {
    if (cgi.getElement("type")->getValue() == "play") {
      std::ifstream midi_file("/tmp/bmc.cgi/" + prefix + ".midi");
      if (midi_file.good()) {
        std::cout << "Content-type: audio/midi" << std::endl << std::endl;
        std::cout << midi_file.rdbuf();
        exit(EXIT_SUCCESS);
      }
    } else if (cgi.getElement("type")->getValue() == "png") {
      std::ifstream png_file("/tmp/bmc.cgi/" + cgi("hash") + ".png");
      if (png_file.good()) {
        std::cout << "Content-type: image/png" << std::endl << std::endl;
        std::cout << png_file.rdbuf();
        exit(EXIT_SUCCESS);
      }
    }
  }
  std::cout << "Content-type: text/html; charset=utf-8"
            << std::endl
            << std::endl
            << cgicc::XHTMLDoctype(cgicc::XHTMLDoctype::eStrict)
            << cgicc::html()
            << cgicc::head()
            << cgicc::title("Braille Music Compiler")
            << cgicc::head()
            << cgicc::body();
  if (not cgi("music").empty() and prefix.empty()) {
    std::cout << cgicc::p("Unable to translate braille music, try again.").set("class", "error") << std::endl;
  }
  std::cout << cgicc::p() << "See a "
                          << cgicc::a("braille music code tutorial")
                            .set("href", "https://bmc.branchable.com/tutorial/")
                          << " for details."
                          << cgicc::p() << std::endl;
  std::cout << cgicc::form();
  std::cout << cgicc::label("Select braille table: ").set("for", "table");
  table_select(std::cout, cgi);
  std::cout << cgicc::div()
            << cgicc::label("Enter braille music: ").set("for", "music");
  std::cout << music_input << cgicc::div();
  std::cout << cgicc::input().set("type", "submit").set("value", "translate");
  if (not prefix.empty()) {
    std::cout << cgicc::input().set("type", "submit").set("name", "type").set("value", "play");
  }
  std::cout << cgicc::form();
  if (not prefix.empty()) {
    std::string alt;
    std::ifstream ly("/tmp/bmc.cgi/" + prefix + ".ly");
    if (ly.good()) {
      std::istreambuf_iterator<char> ly_begin(ly.rdbuf()), ly_end;
      alt = std::string(ly_begin, ly_end);
      std::string::size_type i = 0;
      while ((i = alt.find("\"", i)) != std::string::npos) {
        alt.replace(i, 1, "&quot;");
      }
    }
    std::cout << cgicc::img().set("src", cgi.getEnvironment().getScriptName() + "?hash=" + prefix + "&type=png").set("alt", alt);
  }
  std::cout << cgicc::body()
            << cgicc::html();
  return EXIT_SUCCESS;
}

