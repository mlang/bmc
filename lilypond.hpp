// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "ambiguous.hpp"
#include "output_format.hpp"

namespace music { namespace lilypond {

class generator: public boost::static_visitor<void>
{
  std::ostream& os;
  bool const layout, midi;
public:
  generator(std::ostream& os, bool layout = true, bool midi = true);

  void operator() (braille::ambiguous::score const& score) const;

  void operator() ( braille::ambiguous::part const& part
                  , braille::ambiguous::score const& score
		  ) const;

  void operator() (braille::ambiguous::measure const& measure) const
  {
    if (measure.voices.size() == 1) {
      (*this)(measure.voices.front());
    } else {
      os << "<< ";
      for (size_t voice_index = 0; voice_index < measure.voices.size();
           ++voice_index)
      {
        os << "{"; (*this)(measure.voices[voice_index]); os << "}";
        if (voice_index != measure.voices.size() - 1) os << "\\\\";
      }
      os << " >>";
    }
  }
  void operator() (braille::ambiguous::voice const& voice) const
  {
    for (size_t partial_measure_index = 0; partial_measure_index < voice.size();
         ++partial_measure_index)
    {
      (*this)(voice[partial_measure_index]);
      if (partial_measure_index != voice.size() - 1) os << " ";
    }
  }
  void operator() (braille::ambiguous::partial_measure const& partial_measure) const
  {
    if (partial_measure.size() == 1) {
      (*this)(partial_measure.front());
    } else {
      os << "<< ";
      for (size_t voice_index = 0; voice_index < partial_measure.size();
           ++voice_index)
      {
        os << "{"; (*this)(partial_measure[voice_index]); os << "}";
        if (voice_index != partial_measure.size() - 1) os << "\\\\";
      }
      os << " >>";
    }
  }
  void operator() (braille::ambiguous::partial_voice const& partial_voice) const
  {
    for (size_t element_index = 0; element_index < partial_voice.size();
         ++element_index)
    {
      boost::apply_visitor(*this, partial_voice[element_index]);
      if (element_index != partial_voice.size() - 1) os << " ";
    }
  }
//[ ::music::braille::ambiguous::sign
  result_type operator() (braille::ambiguous::barline const&) const
  {
  }
  result_type operator() (braille::ambiguous::simile const&) const
  { BOOST_ASSERT(false); }
  result_type operator() (braille::ambiguous::value_distinction const&) const
  { }
  result_type operator() (braille::hand_sign const&) const
  {
  }
  result_type operator() (braille::ambiguous::rest const& rest) const
  {
    os << "r";
    if (rest.whole_measure) os << "1";
    else ly_rhythm(rest);
  }
  result_type operator() (braille::ambiguous::note const& note) const;
  result_type operator() (braille::ambiguous::chord const&) const;
//]

private: // utilities
  void ly_accidental(int) const;
  void ly_clef(std::string const&) const;
  void ly_finger(braille::fingering_list const&) const;
  void ly_key(key_signature const&) const;
  void ly_rhythm(braille::ambiguous::rhythmic_data const&) const;
  void ly_octave(int) const;
  void ly_partial(rational const&) const;
  void ly_pitch_step(diatonic_step) const;
};

}}

inline std::ostream&
operator <<
( std::ostream& stream
, music::braille::ambiguous::score const& score
) {
  switch (music::get_output_format(stream)) {
  case music::output_format::lilypond: {
    music::lilypond::generator generate(stream);
    generate(score);
    break;
  }
  default:
    BOOST_ASSERT(false);
  }
  return stream;
}
