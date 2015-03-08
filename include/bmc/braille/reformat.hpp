// Copyright (C) 2015  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_BRAILLE_REFORMAT_HPP_INCLUDED
#define BMC_BRAILLE_REFORMAT_HPP_INCLUDED

#include "bmc/braille/ast.hpp"

#include <ostream>

namespace bmc { namespace braille {

struct format_style {
  unsigned columns = 40;
};

struct output {
  struct fragment {
    std::u32string unicode;
    std::string description;
    bool needs_guide_dot;
    fragment(std::u32string const &unicode, std::string const &description, bool needs_guide_dot = false)
    : unicode{unicode}, description{description}, needs_guide_dot{needs_guide_dot} {}
  };
  std::vector<fragment> fragments;
};

output reformat(ast::score const &, format_style const &style = format_style());

std::ostream &operator<<(std::ostream &, output const &);

}}

#endif
