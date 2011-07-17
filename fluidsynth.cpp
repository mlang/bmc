#include "fluidsynth.hpp"
#include <thread>

namespace music {

fluidsynth::fluidsynth(std::string const& soundfont)
: settings(new_fluid_settings())
, synth(new_fluid_synth(settings))
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
  queue.push(midi::note_off(note.tick+note.duration, note.channel, note.note));
}

void
fluidsynth::operator()(midi::note_off const& note)
{
  fluid_synth_noteoff(synth, note.channel, note.note);
}

void
fluidsynth::operator()(braille::ambiguous::score const& score)
{
  unsigned int const ppq = 480;
  queue.push(midi::note_on(0, 0, 60, 90, ppq*3));
  queue.push(midi::note_on(ppq, 0, 64, 90, ppq*2));
  queue.push(midi::note_on(ppq*2, 0, 67, 90, ppq));

  unsigned int bpm = 90;
  typedef std::chrono::high_resolution_clock::duration duration;
  typedef std::chrono::high_resolution_clock::time_point time_point;
  duration const ppm(duration::period::den * 60 / ppq);
  time_point event_time(std::chrono::high_resolution_clock::now());
  unsigned int tick = 0;
  while (!queue.empty()) {
    midi::event const event(queue.top());
    queue.pop();
    unsigned int const delta = event.start_tick() - tick;
    tick += delta, event_time += ppm / bpm * delta;
    std::this_thread::sleep_until(event_time), event.apply_visitor(*this);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

}

