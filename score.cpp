#include "score_def.hpp"

namespace music { namespace braille {

typedef std::wstring::const_iterator iterator_type;
template struct score_grammar<iterator_type>;

}}
