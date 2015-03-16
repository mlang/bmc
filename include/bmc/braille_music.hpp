// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_BRAILLE_MUSIC_HPP
#define BMC_BRAILLE_MUSIC_HPP

#include <vector>

namespace bmc { namespace braille {

enum class interval_direction
{ down
, up
};

typedef std::pair<unsigned, unsigned> finger_change;
typedef boost::variant<unsigned, finger_change> fingering;
typedef std::vector<fingering> fingering_list;

}}

#endif
