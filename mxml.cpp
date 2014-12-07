// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/musicxml.hpp"
#include "xsdcxx-musicxml/musicxml.hxx"

void music::musicxml(std::ostream &os, music::braille::ast::score const &score)
{
  ::musicxml::score_partwise xml_score { ::musicxml::part_list {} };
  unsigned c { 1 };
  for (auto &&p: score.unfolded_part) {
    ::musicxml::score_partwise::part_type part { "P" + std::to_string(c) };

    xml_score.part_list().score_part().push_back({
      "Part-" + std::to_string(c++), part.id()
    });
    xml_score.part().push_back(part);
  }
  ::musicxml::score_partwise_(os, xml_score);
}

