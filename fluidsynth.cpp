// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "fluidsynth.hpp"
#include <thread>
#include <boost/range/numeric.hpp>

namespace music {

fluidsynth::fluidsynth(std::string const& soundfont)
: settings(new_fluid_settings())
, synth(new_fluid_synth(settings))
, bpm(70)
{
  fluid_settings_setstr(settings, "audio.driver", "alsa");
  fluid_settings_setint(settings, "audio.jack.autoconnect", 1);
  fluid_synth_sfload(synth, soundfont.c_str(), 1);
  audio_driver = new_fluid_audio_driver(settings, synth);
}

fluidsynth::fluidsynth(fluidsynth&& rvalue)
: settings(rvalue.settings)
, synth(rvalue.synth)
, audio_driver(rvalue.audio_driver)
, bpm(rvalue.bpm)
{ rvalue.settings = 0, rvalue.synth = 0, rvalue.audio_driver = 0; }

fluidsynth::~fluidsynth()
{
  if (audio_driver != 0) delete_fluid_audio_driver(audio_driver);
  if (synth != 0) delete_fluid_synth(synth);
  if (settings != 0) delete_fluid_settings(settings);
}

void
fluidsynth::operator()(midi::note_on const& note)
{
  fluid_synth_noteon(synth, note.channel, note.note, note.velocity);
  queue.push(midi::note_off(note.begin+note.duration, note.channel, note.note));
}

void
fluidsynth::operator()(midi::note_off const& note)
{ fluid_synth_noteoff(synth, note.channel, note.note); }

void
fluidsynth::operator()(braille::ambiguous::score const& score)
{
  for(braille::ambiguous::part const& part: score.parts) {
    for(braille::ambiguous::staff const& staff: part)
    {
      current_position = zero;
      std::for_each(staff.begin(), staff.end(), boost::apply_visitor(*this));
    }
  }
  ppq = queue.ppq();
  play();
}

void
fluidsynth::play()
{
  typedef std::chrono::high_resolution_clock clock;
  clock::duration const ppm(clock::duration::period::den * 60 / ppq);
  clock::time_point event_time(clock::now());
  rational position;
  rational pulse(rational(1, 4) / ppq);
  while (!queue.empty()) {
    midi::event const event(queue.top());
    queue.pop();
    rational const delta = event.begin() - position;
    if (delta > 0) {
      position += delta;
      event_time += ppm / bpm * boost::rational_cast<rational::int_type>(delta / pulse);
      std::this_thread::sleep_until(event_time);
    }
    event.apply_visitor(*this);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

class midi_performer : public boost::static_visitor<void>
{
  midi::event_queue& queue;
  rational& position;
public:
  midi_performer(midi::event_queue& queue, rational& position)
  : queue(queue), position(position) {}
  result_type operator()(braille::ambiguous::note const& note) const
  {
    int const chromatic[7] = { 0, 2, 4, 6, 7, 9, 11 };
    queue.push(midi::note_on(position, 0,
                             (note.octave*12) + chromatic[note.step] + note.alter,
                             90, note.as_rational()));
    position += note.as_rational();
  }
  result_type operator()(braille::ambiguous::rest const& rest) const
  { position += rest.as_rational(); }
  result_type operator()(braille::ambiguous::chord const& chord) const
  {
    (*this)(chord.base);
    // ...
  }
  template<typename T> result_type operator()(T const&) const {}
};

}


namespace music {

void
fluidsynth::operator()(braille::ambiguous::measure const& measure)
{
  for (braille::ambiguous::voice const& voice: measure.voices) {
    rational voice_position(current_position);
    for (braille::ambiguous::partial_measure const& partial_measure: voice) {
      for (braille::ambiguous::partial_voice const& partial_voice: partial_measure) {
        rational position(voice_position);
        midi_performer perform(queue, position);
        std::for_each(partial_voice.begin(), partial_voice.end(),
                      boost::apply_visitor(perform));
      }
      voice_position += duration(partial_measure);      
    }
  }
  current_position += duration(measure);
}

}

