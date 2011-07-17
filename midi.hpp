#include <boost/variant.hpp>
#include <queue>

namespace music { namespace midi {

struct channel_event
{
  int tick;
  int channel;
  channel_event(int tick, int channel) : tick(tick), channel(channel) {}
};

struct note_event : channel_event
{
  int note;
  note_event(int tick, int channel, int note)
  : channel_event(tick, channel), note(note) {}
};

struct note_on : note_event
{
  int velocity;
  int duration;
  note_on(int tick, int channel, int note, int velocity, int duration)
  : note_event(tick, channel, note), velocity(velocity), duration(duration) {}
};

struct note_off : note_event
{
  note_off(int tick, int channel, int note)
  : note_event(tick, channel, note) {}
};

class event : public boost::variant<note_on, note_off>
            , public boost::static_visitor<int>
{
public:
  event(note_on const& note) : boost::variant<note_on, note_off>(note) {}
  event(note_off const& note) : boost::variant<note_on, note_off>(note) {}
  int start_tick() const
  { return this->apply_visitor(*this); }
  bool operator<(event const& rhs) const
  { return start_tick() > rhs.start_tick(); }
  int operator()(channel_event const& e) const
  { return e.tick; }
};

typedef std::priority_queue<event> event_queue;

}}

