#include <fluidsynth.h>
#include "ambiguous.hpp"

namespace music {

class fluidsynth
{
  fluid_settings_t *settings;
  fluid_synth_t *synth;
  fluid_audio_driver_t *audio_driver;
public:
  fluidsynth(std::string const& soundfont);
  fluidsynth(fluidsynth&&);
  ~fluidsynth();

  void operator()(braille::ambiguous::score const&);
};

}

