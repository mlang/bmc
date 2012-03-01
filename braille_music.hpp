#ifndef BMC_BRAILLE_MUSIC_HPP
#define BMC_BRAILLE_MUSIC_HPP

namespace music { namespace braille {

enum class interval_direction
{ down
, up
};

enum hand_sign { right_hand, left_hand };

typedef std::pair<unsigned, unsigned> finger_change;
typedef boost::variant<unsigned, finger_change> fingering;
typedef std::list<fingering> fingering_list;

}}

#endif
