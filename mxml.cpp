// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/musicxml.hpp"

#include "xsdcxx-musicxml/musicxml.hxx"

void music::lilypond(std::ostream &os, music::braille::ast::score const &score)
{
  ::musicxml::part_list part_list { };
  ::musicxml::score_partwise xml_score { part_list };
  ::musicxml::score_partwise_(os, xml_score);
}

