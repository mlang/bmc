#include <fluidsynth.h>
#include "ambiguous.hpp"
#include "midi.hpp"

namespace music {

class fluidsynth
: public boost::static_visitor<void>
{
  fluid_settings_t *settings;
  fluid_synth_t *synth;
  fluid_audio_driver_t *audio_driver;
  midi::event_queue queue;
public:
  fluidsynth(std::string const& soundfont);
  fluidsynth(fluidsynth const&) = delete;
  fluidsynth& operator=(fluidsynth const&) = delete;
  fluidsynth(fluidsynth&&);
  ~fluidsynth();

  void operator()(midi::note_on const&);
  void operator()(midi::note_off const&);

  void operator()(braille::ambiguous::score const&);
};

}

