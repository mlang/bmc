// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/music.hpp"
#include <boost/variant.hpp>
#include <queue>

namespace music { namespace midi {

struct channel_event
{
  rational begin;
  int channel;
  channel_event(rational const& begin, int channel)
  : begin(begin), channel(channel) {}
};

struct note_event : channel_event
{
  int note;
  note_event(rational const& begin, int channel, int note)
  : channel_event(begin, channel), note(note) {}
};

struct note_on : note_event
{
  int velocity;
  rational duration;
  note_on( rational const& begin, int channel, int note
         , int velocity, rational const& duration)
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
  struct begin_ : boost::static_visitor<rational const&>
  {
    result_type operator()(channel_event const& event) const
    { return event.begin; }
  };
  struct duration_ : public boost::static_visitor<rational const&>
  {
    result_type operator()(note_on const& note) const { return note.duration; }
    result_type operator()(channel_event const&) const { return zero; }
  };
  template<typename EventType>
  struct is_event : public boost::static_visitor<bool>
  {
    result_type operator()(EventType const&) const { return true; }
    result_type operator()(channel_event const&) const { return false; }
  };
public:
  template<typename T> event(T const& t) : event_base(t) {}
  begin_::result_type begin() const
  { return boost::apply_visitor(begin_(), *this); }
  duration_::result_type duration() const
  { return boost::apply_visitor(duration_(), *this); }
  bool operator>(event const& rhs) const
  {
    if (begin() == rhs.begin()) {
      // assure that note_off events are placed before all others to avoid
      // accidentally killing of a note that just started at the same time.
      is_event<note_off> is_note_off;     
      if (boost::apply_visitor(is_note_off, *this)) return false;
      if (boost::apply_visitor(is_note_off, rhs)) return true;
    }
    return begin() > rhs.begin();
  }
};

/**
 * \breif A queue of MIDI events.
 *
 * This class handles ordering of MIDI events according to their begin time.
 * Call the <code>push</code> member function to add events.
 */
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
    pulse = boost::math::gcd(pulse, boost::math::gcd(event.begin(), event.duration()));
    base_type::push(event);
  }
  /**
   * \brief Returns the number of pulses per quarter note implied by the current
   *        content of this queue.
   */
  rational::int_type ppq() const
  {
    rational const p(rational(1, 4) / pulse);
    BOOST_ASSERT(p.denominator() == 1);
    return boost::rational_cast<rational::int_type>(p);
  }
};

}}

