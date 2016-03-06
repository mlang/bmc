// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "key_signature_def.hpp"

namespace bmc { namespace braille {

typedef std::wstring::const_iterator iterator_type;
template struct key_signature_grammar<iterator_type>;

}}
