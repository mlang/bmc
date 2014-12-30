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

namespace {

// Output the hash of an arbitrary string in hex.
std::string hash(std::string const &string)
{
  std::stringstream stream;
  stream << std::hex << std::hash<std::string>()(string);
  return stream.str();
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
     << table_option("as", cgi)
     << table_option("awa", cgi)
     << table_option("bg", cgi)
     << table_option("bh", cgi)
     << table_option("bn", cgi)
     << table_option("bo", cgi)
     << table_option("bra", cgi)
     << table_option("cs", cgi)
     << table_option("cy", cgi)
     << table_option("da", cgi)
     << table_option("da-1252", cgi)
     << table_option("da-lt", cgi)
     << table_option("de", cgi)
     << table_option("dra", cgi)
     << table_option("el", cgi)
     << table_option("en", cgi)
     << table_option("en_CA", cgi)
     << table_option("en_GB", cgi)
     << table_option("en-nabcc", cgi)
     << table_option("en_US", cgi)
     << table_option("eo", cgi)
     << table_option("es", cgi)
     << table_option("et", cgi)
     << table_option("fi", cgi)
     << table_option("fr", cgi)
     << table_option("fr-2007", cgi)
     << table_option("fr_CA", cgi)
     << table_option("fr-cbifs", cgi)
     << table_option("fr_FR", cgi)
     << table_option("fr-vs", cgi)
     << table_option("ga", cgi)
     << table_option("gd", cgi)
     << table_option("gon", cgi)
     << table_option("gu", cgi)
     << table_option("he", cgi)
     << table_option("hi", cgi)
     << table_option("hr", cgi)
     << table_option("hu", cgi)
     << table_option("hy", cgi)
     << table_option("is", cgi)
     << table_option("it", cgi)
     << table_option("kha", cgi)
     << table_option("kn", cgi)
     << table_option("kok", cgi)
     << table_option("kru", cgi)
     << table_option("lt", cgi)
     << table_option("lv", cgi)
     << table_option("mg", cgi)
     << table_option("mi", cgi)
     << table_option("ml", cgi)
     << table_option("mni", cgi)
     << table_option("mr", cgi)
     << table_option("mt", cgi)
     << table_option("mun", cgi)
     << table_option("mwr", cgi)
     << table_option("ne", cgi)
     << table_option("new", cgi)
     << table_option("nl", cgi)
     << table_option("nl_BE", cgi)
     << table_option("nl_NL", cgi)
     << table_option("no", cgi)
     << table_option("no-generic", cgi)
     << table_option("no-oub", cgi)
     << table_option("nwc", cgi)
     << table_option("or", cgi)
     << table_option("pa", cgi)
     << table_option("pi", cgi)
     << table_option("pl", cgi)
     << table_option("pt", cgi)
     << table_option("ro", cgi)
     << table_option("ru", cgi)
     << table_option("sa", cgi)
     << table_option("sat", cgi)
     << table_option("sd", cgi)
     << table_option("sk", cgi)
     << table_option("sv", cgi)
     << table_option("sv-1989", cgi)
     << table_option("sv-1996", cgi)
     << table_option("sw", cgi)
     << table_option("ta", cgi)
     << table_option("te", cgi)
     << table_option("tr", cgi)
     << table_option("vi", cgi)
     << cgicc::select();
  return os;
}

cgicc::option instrument_option(std::string const &instrument, cgicc::Cgicc const &cgi) {
  cgicc::option o(instrument);
  o.set("value", instrument);
  if (cgi.getElement("instrument") != cgi.getElements().end() and
      cgi.getElement("instrument")->getValue() == instrument)
    o.set("selected", "selected");
  return o;
}

std::ostream &instrument_select(std::ostream &os, cgicc::Cgicc const &cgi) {
  os << cgicc::select().set("name", "instrument").set("id", "instrument");
  for (auto instrument: ::bmc::lilypond::instruments) {
    os << instrument_option(instrument, cgi);
  }
  os << cgicc::select();
  return os;
}

} // namespace

int main()
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
    typedef ::bmc::braille::error_handler<iterator_type> error_handler_type;
    error_handler_type error_handler(iter, end);
    typedef ::bmc::braille::score_grammar<iterator_type> parser_type;
    parser_type parser(error_handler);
    boost::spirit::traits::attribute_of<parser_type>::type score;

    bool const success = parse(iter, end, parser, score);

    if (success and iter == end) {
      ::bmc::braille::compiler<error_handler_type> compile(error_handler);
      if (compile(score)) {
        prefix = hash(braille->getValue());
        std::string dir("/tmp/bmc.cgi/");
        std::ofstream bmc(dir + prefix + ".bmc");
        bmc << braille->getValue();
        bmc.close();
        std::ofstream ly(dir + prefix + ".ly");
        ::bmc::lilypond::generator generate(ly, true, true, false);
        generate.remove_tagline();
        if (not cgi("instrument").empty())
          generate.instrument(cgi("instrument"));
        generate(score);
        ly.close();        
        std::string cmd("lilypond --png -dpaper-size='\"a5\"' -o " + dir + prefix + " " + dir + prefix + ".ly" + " >/tmp/bmc.cgi/log 2>&1");
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
    }
  }
  if (not cgi("hash").empty()) {
    if (cgi.getElement("type")->getValue() == "png") {
      std::ifstream png_file("/tmp/bmc.cgi/" + cgi("hash") + ".png");
      if (png_file.good()) {
        std::cout << "Content-type: image/png" << std::endl << std::endl;
        std::cout << png_file.rdbuf();
        exit(EXIT_SUCCESS);
      }
    } else if (cgi.getElement("type")->getValue() == "pdf") {
      std::string cmd("lilypond --pdf -o /tmp/bmc.cgi/" + cgi("hash") + " /tmp/bmc.cgi/" + cgi("hash") + ".ly");
      if (system(cmd.c_str()) == 0) {
        std::ifstream pdf_file("/tmp/bmc.cgi/" + cgi("hash") + ".pdf");
        if (pdf_file.good()) {
          std::cout << "Content-type: application/pdf" << std::endl << std::endl;
          std::cout << pdf_file.rdbuf();
          exit(EXIT_SUCCESS);
        }
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
  std::cout << cgicc::h1("Automatic transcription of braille music code to print and sound") << std::endl;
  if (cgi("music").empty()) {
    std::cout << cgicc::p() << "See this "
                            << cgicc::a("braille music code tutorial")
                              .set("href", "https://bmc.branchable.com/tutorial/")
                            << " for details and inspiration."
                            << cgicc::p() << std::endl;
    std::cout << cgicc::p("To allow for several parts in a single score, music always needs to end with a final bar sign.") << std::endl;
  }
  if (not cgi("music").empty() and prefix.empty()) {
    std::cout << cgicc::p("Unable to translate braille music, try again.").set("class", "error") << std::endl;
    std::cout << cgicc::p() << "The "
                            << cgicc::a("tutorial").set("href", "https://bmc.branchable.com/tutorial/")
                            << " contains working examples which can be used as a starting point.  Just invoke the Edit link below each example to load it back in here." << cgicc::p() << std::endl;
  }
  std::cout << cgicc::form();
  std::cout << cgicc::label("Select braille table: ").set("for", "table");
  table_select(std::cout, cgi);
  std::cout << cgicc::div()
            << cgicc::label("Enter braille music: ").set("for", "music");
  std::cout << music_input << cgicc::div();
  std::cout << cgicc::input().set("type", "submit").set("value", "Transcribe to print");
  if (not prefix.empty()) {
    instrument_select(std::cout, cgi);
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
    std::cout << cgicc::div() << cgicc::a("Download PDF").set("href", cgi.getEnvironment().getScriptName() + "?hash=" + prefix + "&type=pdf") << cgicc::div() << std::endl;
  }
  std::cout << cgicc::body()
            << cgicc::html();
  return EXIT_SUCCESS;
}

