#ifndef BMC_BRAILLE_TEXT2BRAILLE_HPP_INCLUDED
#define BMC_BRAILLE_TEXT2BRAILLE_HPP_INCLUDED

#include <string>

namespace bmc { namespace braille {

extern std::string default_table;

void set_default_table_from_locale ();
uint8_t get_dots_for_character(char32_t c, std::string const &table = default_table);

}}

#endif
