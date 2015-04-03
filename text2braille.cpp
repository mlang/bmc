#include <bmc/braille/text2braille.hpp>

#include <algorithm>
#include <iostream>
#include <locale.h>
#include <unordered_map>
#include <string>

namespace {

#define BEGIN_MAPPING(name) \
std::pair<char32_t, uint8_t> constexpr name##_mappings[] = {
#define BEGIN_ALIASES(name) \
std::pair<char32_t, char32_t> constexpr name##_aliases[] = {

#define BRLTTY_TEXT_TABLE_CHARACTER(unicode, braille, isPrimary, name) \
  { unicode, braille },
#define BRLTTY_TEXT_TABLE_END_CHARACTERS };
#define BRLTTY_TEXT_TABLE_ALIAS(from, to, name) { from, to },
#define BRLTTY_TEXT_TABLE_END_ALIASES };

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(brf)
#undef BRLTTY_TEXT_TABLE_END_ALIASES
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES
#define BRLTTY_TEXT_TABLE_END_ALIASES
#include "tables/brf.cpp"
  BEGIN_ALIASES(brf) { 0XFFFFFFFF, 0 } };
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES
#undef BRLTTY_TEXT_TABLE_END_ALIASES
#define BRLTTY_TEXT_TABLE_END_ALIASES };

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(de)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(de)
#include "tables/de.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(en_CA)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(en_CA)
#include "tables/en_CA.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(en_GB)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(en_GB)
#include "tables/en_GB.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(en_US)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(en_US)
#include "tables/en_US.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(es)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(es)
#include "tables/es.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(fr_CA)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(fr_CA)
#include "tables/fr_CA.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

#define BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS BEGIN_MAPPING(fr_FR)
#define BRLTTY_TEXT_TABLE_BEGIN_ALIASES BEGIN_ALIASES(fr_FR)
#include "tables/fr_FR.cpp"
#undef BRLTTY_TEXT_TABLE_BEGIN_CHARACTERS
#undef BRLTTY_TEXT_TABLE_BEGIN_ALIASES

template<size_t N, size_t M>
uint8_t find(char32_t c,
	     std::pair<char32_t, uint8_t> const (&mapping)[N],
	     std::pair<char32_t, char32_t> const (&aliases)[M])
{
  if (c >= 0X2800 && c <= 0X28FF) return c & 0XFF;
  auto i = std::lower_bound(std::begin(mapping), std::end(mapping),
			    std::make_pair(c, 0),
			    [](std::pair<char32_t, uint8_t> const &lhs,
			       std::pair<char32_t, uint8_t> const &rhs)
			    { return lhs.first < rhs.first; });
  if (i == std::end(mapping) || i->first != c) {
    auto j = std::lower_bound(std::begin(aliases), std::end(aliases),
			      std::make_pair(c, 0),
			      [](std::pair<char32_t, char32_t> const &lhs,
				 std::pair<char32_t, char32_t> const &rhs)
			      { return lhs.first < rhs.first; });
    if (j != std::end(aliases) && j->first == c) {
      i = std::lower_bound(std::begin(mapping), std::end(mapping),
			   std::make_pair(j->second, 0),
			   [](std::pair<char32_t, uint8_t> const &lhs,
			      std::pair<char32_t, uint8_t> const &rhs)
			   { return lhs.first < rhs.first; });
      if (i != std::end(mapping) && i->first != j->second)
	i = std::end(mapping);
    }
  }
  if (i != std::end(mapping)) return i->second;

  throw std::runtime_error("no mapping");
}

std::string const locales[] = {
  "de", "es", "fr_CA", "fr_FR", "en_CA", "en_GB", "en_US"
};

}

namespace bmc { namespace braille {

std::string default_table = "brf";

void set_default_table_from_locale () {
  std::string locale{setlocale(LC_CTYPE, NULL)};
  if (locale == "C" || locale == "POSIX") {
    default_table = "brf";
  } else {
    auto pos = locale.find(".");
    if (pos != std::string::npos) {
      locale = locale.substr(0, pos);
      auto i = std::find(std::begin(locales), std::end(locales), locale);
      if (i != std::end(locales)) {
        default_table = *i;
      } else {
        pos = locale.find("_");
        if (pos != std::string::npos) {
          locale = locale.substr(0, pos);
          i = std::find(std::begin(locales), std::end(locales), locale);
          if (i != std::end(locales)) {
            default_table = *i;
          }
        }
      }
    }
  }
}

uint8_t get_dots_for_character(char32_t c, std::string const &locale) {
#define CHECK(l) if (locale == #l) return ::find(c, l##_mappings, l##_aliases);
  CHECK(brf);
  CHECK(de);
  CHECK(es);
  CHECK(en_CA);
  CHECK(en_GB);
  CHECK(en_US);
  CHECK(fr_CA);
  CHECK(fr_FR);
  throw std::runtime_error("Unknown braille table");
#undef CHECK
}

}}

