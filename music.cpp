// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "music.hpp"

namespace music {

bool time_signature::operator==(time_signature const& rhs) const {
  return d_num == rhs.d_num && d_den == rhs.d_den;
}

}

