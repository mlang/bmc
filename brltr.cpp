// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <fstream>
#include <iostream>
#include <locale>
#include <cstring>

#include "ttb/prologue.h"
#include "ttb/brldots.h"
#include "ttb/file.h"
#include "ttb/ttb.h"
#include "ttb/unicode.h"

static char *
resolveTablePath (const char *tableName) {
  char *relative = ensureTextTableExtension(tableName);

  if (relative) {
    char *absolute = makePath(TABLES_DIRECTORY, relative);
    if (absolute) {
      if (testPath(absolute)) {
        free(relative);
        return absolute;
      }

      free(absolute);
    }
    if (testPath(relative)) return relative;

    free(relative);
  }

  return NULL;
}

static char *
findTableByLocale() {
  const char *locale = setlocale(LC_CTYPE, NULL);

  if (locale) {
    char *name = (char *)malloc(strlen(locale) + 1);

    {
      size_t const length = strcspn(locale, ".@");
      strncpy(name, locale, length)[length] = 0;
    }

    if (strcmp(name, "C") == 0) {
      free(name);
      return NULL;
    }

    char *tablePath = resolveTablePath(name);
    if (!tablePath) {
      char *delimiter = strchr(name, '_');

      if (delimiter) {
        *delimiter = 0;
        tablePath = resolveTablePath(name);
      }
    }

    free(name);

    return tablePath;
  }

  return NULL;
}

using namespace std;

static void
translate( wistream &in
         , wostream &out
         , bool six_dots = false
         , bool reverse = false
         ) {
  istreambuf_iterator<wchar_t> end;
  for (istreambuf_iterator<wchar_t> iter = in.rdbuf(); iter != end; ++iter) {
    wchar_t character(*iter);
    if (reverse) {
      if ((character & UNICODE_BRAILLE_ROW) == UNICODE_BRAILLE_ROW) {
        character = convertDotsToCharacter(textTable, character&0XFF);
      }
    } else {
      if (character > 0X20) {
        unsigned char dots = convertCharacterToDots(textTable, character);
        if (six_dots) dots &= ~(BRL_DOT7 | BRL_DOT8);
        character = UNICODE_BRAILLE_ROW | dots;
      }
    }
    out << character;
  }
}

static void
print_help_and_exit() {
  wcout << "Usage: " << "brltr" << " [-6 | -8 | -t] [FILE | -]..." << endl
	<< "OPTIONS:" << endl
	<< "\t-6\tOutput six dot braille only" << endl
	<< "\t-8\tOutput eight dot braille (default)" << endl
	<< "\t-t FILE, --table=FILE" << endl << "\t\tUse given table" << endl
        ;
  exit(EXIT_SUCCESS);
}

static void
process_args(int argc, const char **argv) {
  bool six_dots = false, process_options = true, input_processed = false;
  for (int i = 0; i < argc; ++i) {
    string arg(argv[i]);
    if (process_options) {
      if (arg == "-?" || arg == "-h" || arg == "--help") {
	print_help_and_exit();
      } else if (arg == "-8") {
        six_dots = false;
        continue;
      } else if (arg == "-6") {
        six_dots = true;
        continue;
      } else if (arg.substr(0, 2) == "-t" || arg.substr(0, 7) == "--table") {
        string table;
        if (arg == "-t" || arg == "--table") {
          if (i+1 < argc) {
            table = argv[++i];
          } else {
            wcerr << "Missing argument for " << arg.c_str() << endl;
            exit(EXIT_FAILURE);
          }
        } else if (arg.substr(0, 2) == "-t") {
          table = arg.substr(2, arg.length() - 2);
        } else if (arg.substr(0, 8) == "--table=") {
          table = arg.substr(8, arg.length() - 8);
        }
        if (!table.empty()) {
          char *path = resolveTablePath(table.c_str());
          if (path) {
            TextTable *newTable = compileTextTable(path);
            if (newTable) {
              destroyTextTable(textTable);
              textTable = newTable;
            }
          } else {
            wcerr << "Unable to find table \"" << table.c_str() << "\"" << endl;
          }
        }
        continue;
      } else if (arg == "--") {
        process_options = false;
        continue;
      }
    }

    if (arg == "-") {
      translate(wcin, wcout, six_dots);
      input_processed = true;
    } else {
      wifstream in(arg.c_str());
      if (in.good()) {
        translate(in, wcout, six_dots);
        input_processed = true;
      } else {
        wcerr << "Error opening \"" << arg.c_str() << "\"" << endl;
      }
    }
  }

  if (!input_processed) translate(wcin, wcout, six_dots);
}

int main(int argc, char const **argv) {
  locale::global(locale(""));

  {
    char *localeTable = findTableByLocale();
    if (localeTable) {
      textTable = compileTextTable(localeTable);
      free(localeTable);
    }    
  }

  process_args(--argc, ++argv);

  destroyTextTable(textTable);

  return EXIT_SUCCESS;
}
