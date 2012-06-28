// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <fluidsynth.h>
#include "bmc/braille/ast.hpp"
#include "midi.hpp"

namespace music {

class fluidsynth
: public boost::static_visitor<void>
{
  fluid_settings_t *settings;
  fluid_synth_t *synth;
  fluid_audio_driver_t *audio_driver;
  midi::event_queue queue;
  unsigned int ppq, bpm;
  rational current_position;
public:
  fluidsynth(std::string const& soundfont);
  fluidsynth(fluidsynth const&) = delete;
  fluidsynth& operator=(fluidsynth const&) = delete;
  fluidsynth(fluidsynth&&);
  ~fluidsynth();

  void operator()(midi::note_on const&);
  void operator()(midi::note_off const&);

  void operator()(braille::ast::score const&);
  void operator()(braille::ast::measure const&);
  result_type operator()(braille::ast::key_and_time_signature const&) {};
private:
  void play();
};

}

