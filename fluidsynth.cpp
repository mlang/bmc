#include "fluidsynth.hpp"
#include <thread>
#include <boost/range/numeric.hpp>

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
{ fluid_synth_noteoff(synth, note.channel, note.note); }

void
fluidsynth::operator()(braille::ambiguous::score const& score)
{
  for(braille::ambiguous::part const& part: score) {
    for(braille::ambiguous::staff const& staff: part)
    {
      current_position = rational(0);
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

class push_to : public boost::static_visitor<void>
{
  midi::event_queue& queue;
  rational& position;
public:
  push_to(midi::event_queue& queue, rational& position)
  : queue(queue), position(position) {}
  result_type operator()(braille::ambiguous::note const& note) const {
    queue.push(midi::note_on(position, 0, 60, 90, note.as_rational()));
    position += note.as_rational();
  }
  result_type operator()(braille::ambiguous::rest const& rest) const {
    position += rest.as_rational();
  }
  result_type operator()(braille::ambiguous::chord const& chord) const {
    (*this)(chord.base);
  }
  template<typename T> result_type operator()(T const&) const {}
};

struct duration_visitor : boost::static_visitor<rational>
{
  result_type operator()(braille::ambiguous::note const& r) const
  { return r.as_rational(); }
  result_type operator()(braille::ambiguous::rest const& r) const
  { return r.as_rational(); }
  result_type operator()(braille::ambiguous::chord const& r) const
  { return r.as_rational(); }
  template<typename T> result_type operator()(T const&) const
  { return rational(0); }
};

rational
operator+(rational const& r, braille::ambiguous::sign const& element)
{ return r + boost::apply_visitor(duration_visitor(), element); }

rational
duration(braille::ambiguous::partial_voice const& partial_voice)
{ rational accumulator(0);
  for(braille::ambiguous::partial_voice::const_reference sign: partial_voice)
    accumulator = accumulator + sign;
  return accumulator;
}

rational
duration(braille::ambiguous::partial_measure const& partial_measure)
{ return duration(partial_measure.front()); }

rational
operator+(rational const& r, braille::ambiguous::voice::const_reference p)
{ return r + duration(p); }

rational
duration(braille::ambiguous::voice const& voice)
{
  rational accumulator(0);
  for(braille::ambiguous::voice::const_reference partial_measure: voice)
    accumulator = accumulator + partial_measure;
  return accumulator;
}

rational
duration(braille::ambiguous::measure const& measure)
{ return duration(measure.front()); }

void
fluidsynth::operator()(braille::ambiguous::measure const& measure)
{
  std::cout << current_position << std::endl;
  for(braille::ambiguous::voice const& voice: measure) {
    rational voice_position(current_position);
    for(braille::ambiguous::partial_measure partial_measure: voice) {
      rational part_position(voice_position);
      for(braille::ambiguous::partial_voice partial_voice: partial_measure) {
        rational position(part_position);
        push_to pusher(queue, position);
        std::for_each(partial_voice.begin(), partial_voice.end(),
                      boost::apply_visitor(pusher));
      }
      voice_position += duration(partial_measure);      
    }
  }
  current_position += duration(measure);
}

}

