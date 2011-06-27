#include "music.hpp"

namespace music {

bool time_signature::operator==(time_signature const& rhs) const {
  return d_num == rhs.d_num && d_den == rhs.d_den &&
         d_anacrusis == rhs.d_anacrusis;
}

}

