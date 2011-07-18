#include "music.hpp"
#include <boost/variant.hpp>
#include <queue>

namespace music { namespace midi {

struct channel_event
{
  rational begin;
  int channel;
  channel_event(rational begin, int channel) : begin(begin), channel(channel) {}
};

struct note_event : channel_event
{
  int note;
  note_event(rational begin, int channel, int note)
  : channel_event(begin, channel), note(note) {}
};

struct note_on : note_event
{
  int velocity;
  rational duration;
  note_on(rational begin, int channel, int note, int velocity, rational duration)
  : note_event(begin, channel, note), velocity(velocity), duration(duration) {}
};

struct note_off : note_event
{
  note_off(rational const& begin, int channel, int note)
  : note_event(begin, channel, note) {}
};

typedef boost::variant<note_on, note_off> event_base;

class event : public event_base
{
  struct begin_ : boost::static_visitor<rational>
  {
    result_type operator()(channel_event const& event) const
    { return event.begin; }
  };
  struct duration_ : public boost::static_visitor<rational>
  {
    result_type operator()(note_on const& note) const { return note.duration; }
    result_type operator()(channel_event const&) const { return rational(0); }
  };
public:
  template<typename T> event(T const& t) : event_base(t) {}
  rational begin() const { return boost::apply_visitor(begin_(), *this); }
  rational duration() const { return boost::apply_visitor(duration_(), *this); }
  bool operator>(event const& rhs) const { return begin() > rhs.begin(); }
};

class event_queue
: public std::priority_queue<event, std::vector<event>, std::greater<event> >
{
  rational pulse;
public:
  typedef std::priority_queue< value_type
                             , container_type
                             , std::greater<value_type>
                             > base_type;
  event_queue()
  : base_type()
  , pulse(1, 4)
  {}
  void push(value_type const& event)
  {
    pulse = gcd(pulse, gcd(event.begin(), event.duration()));
    base_type::push(event);
  }
  rational::int_type ppq() const
  {
    BOOST_ASSERT((rational(1, 4) / pulse).denominator() == 1);
    return boost::rational_cast<rational::int_type>(rational(1, 4) / pulse);
  }
};

}}

