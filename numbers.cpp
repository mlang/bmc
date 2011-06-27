#include "numbers_def.hpp"

namespace music { namespace braille {

typedef std::wstring::const_iterator iterator_type;
template struct upper_number_grammar<iterator_type>;
template struct lower_number_grammar<iterator_type>;
template struct time_signature_grammar<iterator_type>;

}}
