#include "fluidsynth.hpp"
#include <thread>

namespace music {

fluidsynth::fluidsynth(std::string const& soundfont)
: settings(new_fluid_settings())
, synth(new_fluid_synth(settings))
, bpm(90)
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
{
  fluid_synth_noteoff(synth, note.channel, note.note);
}

void
fluidsynth::operator()(braille::ambiguous::score const& score)
{
  for(braille::ambiguous::part const& part: score) {
    for(braille::ambiguous::staff const& staff: part)
    { std::for_each(staff.begin(), staff.end(), boost::apply_visitor(*this)); }
  }
  queue.push(midi::note_on(rational(0), 0, 60, 90, rational(3, 4)));
  queue.push(midi::note_on(rational(1, 4), 0, 64, 90, rational(2, 4)));
  queue.push(midi::note_on(rational(2, 4), 0, 67, 90, rational(1, 4)));
  BOOST_ASSERT(queue.ppq() == 1);
  typedef std::chrono::high_resolution_clock clock;
  ppq = queue.ppq();
  clock::duration const ppm(clock::duration::period::den * 60 / ppq);
  clock::time_point event_time(clock::now());
  rational position(0);
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

void
fluidsynth::operator()(braille::ambiguous::measure const& measure)
{
}

}

