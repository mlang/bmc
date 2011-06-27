#include "measure_def.hpp"

namespace music { namespace braille {

typedef std::wstring::const_iterator iterator_type;
template struct measure_grammar<iterator_type>;

}}
