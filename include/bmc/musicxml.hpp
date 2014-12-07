// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MUSICXML_HPP_INCLUDED
#define BMC_MUSICXML_HPP_INCLUDED

#include "bmc/braille/ast.hpp"

#include <ostream>

namespace music {

void musicxml(std::ostream &, braille::ast::score const &);

}

#endif
