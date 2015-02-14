// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/musicxml.hpp"
#include <xsdcxx-musicxml/musicxml.hpp>

namespace bmc {

namespace {

// We are going to export score-partwise documents.
using score_type = ::musicxml::score_partwise;
using part_type = score_type::part_type;
using measure_type = part_type::measure_type;

// Determine the greatest common divisor of all rhythmic values in a braille score.
// We need this for the MusicXML divisons element.
// We're basically determining the common denominator such that all
// rhythmic values can be expressed as an integer, since the MusicXML duration
// element is not a rational.

class duration_gcd_visitor : public boost::static_visitor<void> {
  rational value = 0;

public:
  void operator()(braille::ast::unfolded::measure const &measure) {
    for (auto &&voice: measure.voices)
      for (auto &&partial_measure: voice)
        for (auto &&partial_voice: partial_measure)
          std::for_each(partial_voice.begin(), partial_voice.end(),
                        apply_visitor(*this));
  }
  void operator()(::bmc::braille::ast::rhythmic const &r) {
    value = boost::math::gcd(value, r.as_rational());
  }
  void operator()(braille::ast::clef const &) {}
  void operator()(braille::ast::key_and_time_signature const &) {}
  void operator()(braille::ast::tie const &) {}
  void operator()(braille::ast::tuplet_start const &) {}

  rational const &get() const { return value; }
};

rational duration_gcd(braille::ast::score const &score) {
  duration_gcd_visitor accumulator { };

  for (auto &&part: score.unfolded_part)
    for (auto &&staff: part)
      std::for_each(staff.begin(), staff.end(), apply_visitor(accumulator));

  return accumulator.get();
}
 
// Conversion functions between braille AST and MusicXML objects:

::musicxml::key xml(key_signature const &key) {
  ::musicxml::key xml_key{};
  xml_key.fifths(key);

  return xml_key;
}

::musicxml::time xml(time_signature const &time) {
  ::musicxml::time xml_time{};
  xml_time.beats().push_back(std::to_string(time.numerator()));
  xml_time.beat_type().push_back(std::to_string(time.denominator()));

  return xml_time;
}

::musicxml::positive_divisions duration(rational const &dur, rational const &divisions) {
  return {
    boost::rational_cast<double>(dur / (rational{1, 4} / divisions))
  };
}

::musicxml::backup backup(rational const &dur, rational const &divisions) {
  return { duration(dur, divisions) };
}

std::string to_string(rational const & r) {
  return std::to_string(r.numerator()) + "/" + std::to_string(r.denominator());
}

::musicxml::note_type note_type(rational const &r) {
  BOOST_ASSERT(r.numerator() == 1);
  switch (r.denominator()) {
  case 1: return ::musicxml::note_type_value::whole;
  case 2: return ::musicxml::note_type_value::half;
  case 4: return ::musicxml::note_type_value::quarter;
  case 8: return ::musicxml::note_type_value::eighth;
  case 16: return ::musicxml::note_type_value::cxx_16th;
  case 32: return ::musicxml::note_type_value::cxx_32nd;
  case 64: return ::musicxml::note_type_value::cxx_64th;
  case 128: return ::musicxml::note_type_value::cxx_128th;
  default: throw std::runtime_error("Unknown note type: " + to_string(r));
  }
}

::musicxml::note::dot_sequence dots(braille::ast::rhythmic const &rhythmic) {
  ::musicxml::note::dot_sequence xml_dots;
  std::fill_n(std::back_inserter(xml_dots), rhythmic.get_dots(),
              ::musicxml::empty_placement{});
  return xml_dots;
}

::musicxml::pitch pitch(braille::ast::pitched const &p) {
  ::musicxml::step::value step;
  switch (p.step) {
  case A: step = ::musicxml::step::A; break;
  case B: step = ::musicxml::step::B; break;
  case C: step = ::musicxml::step::C; break;
  case D: step = ::musicxml::step::D; break;
  case E: step = ::musicxml::step::E; break;
  case F: step = ::musicxml::step::F; break;
  case G: step = ::musicxml::step::G; break;
  default: BOOST_ASSERT(false);
  }
  ::musicxml::pitch xml_pitch { step, p.octave - 1 };

  if (p.alter) xml_pitch.alter(p.alter);

  return xml_pitch;
}

::musicxml::accidental accidental(::bmc::accidental const &a) {
  switch (a) {
  case ::bmc::natural: return { ::musicxml::accidental_value::natural };
  case ::bmc::flat:    return { ::musicxml::accidental_value::flat };
  case ::bmc::sharp:   return { ::musicxml::accidental_value::sharp };
  case ::bmc::double_flat: return { ::musicxml::accidental_value::flat_flat };
  case ::bmc::double_sharp: return { ::musicxml::accidental_value::sharp_sharp };
  default: throw std::runtime_error("Invalid accidental: " + std::to_string(a));
  }
}

class fingering_visitor : public boost::static_visitor<void> {
  ::musicxml::technical::fingering_sequence &xml_fingers;

public:
  fingering_visitor(::musicxml::technical::fingering_sequence &xml_fingers)
  : xml_fingers { xml_fingers }
  {}

  void operator()(unsigned f) {
    xml_fingers.push_back({ std::to_string(f) });
  }
  void operator()(braille::finger_change const &fc) {
    xml_fingers.push_back({ std::to_string(fc.first) + " " + std::to_string(fc.second) });
  }
};

::musicxml::technical::fingering_sequence
fingering(braille::fingering_list const &fingers) {
  ::musicxml::technical::fingering_sequence xml_fingers;

  if (not fingers.empty()) {
    fingering_visitor visitor { xml_fingers };
    apply_visitor(visitor, fingers.front());
  }

  return xml_fingers;
}

bool is_anacrusis(braille::ast::unfolded::measure const &measure,
                  braille::ast::score const &score) {
  return
  (not score.time_sigs.empty() and (duration(measure) != score.time_sigs.front())) or
  (score.time_sigs.empty() and duration(measure) != 1);
}

class starts_with_anacrusis_visitor : public boost::static_visitor<bool> {
  braille::ast::score const &brl_score;
  bool active = true;

public:
  starts_with_anacrusis_visitor(braille::ast::score const &brl_score)
  : brl_score { brl_score }
  {}

  result_type operator()(braille::ast::unfolded::measure const &measure) {
    result_type result { active and is_anacrusis(measure, brl_score) };

    active = false;

    return result;
  }

  result_type operator()(braille::ast::key_and_time_signature const &) const {
    return false;
  }
};

bool starts_with_anacrusis(braille::ast::unfolded::part const &part,
                           braille::ast::score const &score) {
  starts_with_anacrusis_visitor visitor { score };
  BOOST_ASSERT(not part.empty());
  auto staff = part.front();

  return std::any_of(staff.begin(), staff.end(), boost::apply_visitor(visitor));
}

class make_measures_for_staff_visitor : public boost::static_visitor<void> {
  braille::ast::score const &brl_score;
  part_type::measure_sequence &measures;
  unsigned staff_number;
  rational const &divisions;
  unsigned measure_number = 1;
  measure_type *current_measure;
  bool implicit = false;

public:
  make_measures_for_staff_visitor(braille::ast::score const &brl_score,
                                  part_type::measure_sequence &measures,
                                  unsigned staff_number,
                                  rational const &divisions)
  : brl_score { brl_score }
  , measures { measures }
  , staff_number { staff_number }
  , divisions { divisions }
  , current_measure { nullptr }
  {}

  void operator()(braille::ast::unfolded::measure const &measure) {
    if (measure_number == 1) {
      if (is_anacrusis(measure, brl_score)) implicit = true;
    }

    if (measure_number > measures.size())
      measures.push_back({std::to_string(implicit? measure_number - 1
                                                 : measure_number)});

    current_measure = &measures[measure_number - 1];

    if (implicit and measure_number == 1) {
      current_measure->implicit(::musicxml::yes_no::yes);
    }

    // If this is not the first staff, insert backup element as appropriate.
    if (staff_number > 1)
      current_measure->music_data().push_back(
        backup(duration(measure), divisions)
      );

    for (auto vi = measure.voices.begin(), ve = measure.voices.end();
         vi != ve; ++vi) {
      for (auto &&partial_measure: *vi) {
        for (auto pvi = partial_measure.begin(), pve = partial_measure.end();
             pvi != pve; ++pvi) {
          std::for_each(pvi->begin(), pvi->end(), apply_visitor(*this));
          if (std::next(pvi) != pve)
            current_measure->music_data().push_back(backup(duration(*pvi), divisions));
        }
      }
      if (std::next(vi) != ve)
        current_measure->music_data().push_back(backup(duration(*vi), divisions));
    }

    measure_number += 1;
  }
  void operator()(braille::ast::key_and_time_signature const &) {
  }
  void operator()(braille::ast::note const &note) const {
    current_measure->music_data().push_back(xml(note));
  }
  void operator()(braille::ast::rest const &rest) const {
    current_measure->music_data().push_back(xml(rest));
  }
  void operator()(braille::ast::chord const &chord) const {
    auto music_data = xml(chord);

    current_measure->music_data().insert(current_measure->music_data().end(),
                                         music_data.begin(), music_data.end());
  }
  void operator()(braille::ast::moving_note const &moving_note) const {
    auto music_data = xml(moving_note);

    current_measure->music_data().insert(current_measure->music_data().end(),
                                         music_data.begin(), music_data.end());
  }
  void operator()(braille::ast::barline const &) const {
  }
  void operator()(braille::ast::clef const &) const {
  }
  void operator()(braille::hand_sign const &) const {
  }
  void operator()(braille::ast::tie const &) const {
  }

private:
  ::musicxml::note xml(braille::ast::note const &note) const {
    ::musicxml::note xml_note {
      pitch(note), duration(note.as_rational(), divisions)
    };

    if (is_grace(note)) {
      xml_note.grace(::musicxml::grace{});
    }
    xml_note.type(note_type(note.get_type()));
    xml_note.dot(dots(note));
    if (note.acc) xml_note.accidental(accidental(*note.acc));
    xml_note.staff(staff_number);

    auto xml_fingers = fingering(note.fingers);
    if (not xml_fingers.empty()) {
      ::musicxml::technical xml_technical { };
      xml_technical.fingering(xml_fingers);
      ::musicxml::notations xml_notations { };
      xml_notations.technical().push_back(xml_technical);
      xml_note.notations().push_back(xml_notations);
    }

    for (auto &&articulation: note.articulations) {
      switch(articulation) {
      default: throw std::runtime_error("Unknown articulation: " + std::to_string(articulation));
      case appoggiatura: // Handled by is_grace().
      case arpeggio_up: // Handled by arpeggio() method of ast::chord
      case arpeggio_down:
        break;
      case extended_mordent: {
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        if (xml_note.notations().back().ornaments().empty())
          xml_note.notations().back().ornaments().push_back(::musicxml::ornaments{});
        auto mordent = ::musicxml::mordent{};
        mordent.long_(::musicxml::yes_no::yes);
        xml_note.notations().back().ornaments().back().mordent().push_back(mordent);
        break;
      }
      case mordent:
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        if (xml_note.notations().back().ornaments().empty())
          xml_note.notations().back().ornaments().push_back(::musicxml::ornaments{});
        xml_note.notations().back().ornaments().back().mordent().push_back(::musicxml::mordent{});
        break;
      case extended_short_trill:
      case short_trill:
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        if (xml_note.notations().back().ornaments().empty())
          xml_note.notations().back().ornaments().push_back(::musicxml::ornaments{});
        xml_note.notations().back().ornaments().back().trill_mark().push_back(::musicxml::empty_trill_sound{});
        break;
      case staccato:
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        if (xml_note.notations().back().articulations().empty())
          xml_note.notations().back().articulations().push_back(::musicxml::articulations{});
        xml_note.notations().back().articulations().back().staccato().push_back(::musicxml::empty_placement{});
        break;
      case turn_between_notes:
      case turn_above_or_below_note:
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        if (xml_note.notations().back().ornaments().empty())
          xml_note.notations().back().ornaments().push_back(::musicxml::ornaments{});
        xml_note.notations().back().ornaments().back().turn().push_back(::musicxml::horizontal_turn{});
        break;
      }
    }

    return xml_note;
  }

  ::musicxml::note xml(braille::ast::rest const &rest) const {
    ::musicxml::note xml_note {
      ::musicxml::rest{}, duration(rest.as_rational(), divisions)
    };
    xml_note.type(note_type(rest.get_type()));
    xml_note.dot(dots(rest));
    xml_note.staff(staff_number);

    return xml_note;
  }

  measure_type::music_data_sequence xml(braille::ast::chord const &chord) const
  {
    measure_type::music_data_sequence music_data;
    auto arpeggio = chord.arpeggio();
    music_data.push_back(xml(chord.base));
    if (arpeggio) {
      ::musicxml::note &note = boost::get<::musicxml::note>(music_data.back());
      if (note.notations().empty())
        note.notations().push_back(::musicxml::notations{});
      ::musicxml::arpeggiate arp { };
      switch (*arpeggio) {
      default: throw std::runtime_error("Unknown arpeggio_type.");
      case braille::ast::chord::arpeggio_type::up:
        arp.direction(::musicxml::up_down::up);
        break;
      case braille::ast::chord::arpeggio_type::down:
        arp.direction(::musicxml::up_down::down);
        break;
      }
      note.notations().back().arpeggiate().push_back(arp);
    }
    for (auto &&interval: chord.intervals) {
      ::musicxml::note xml_note {
        pitch(interval), duration(chord.base.as_rational(), divisions)
      };

      xml_note.chord(::musicxml::empty{});
      xml_note.type(note_type(chord.base.get_type()));
      xml_note.dot(dots(chord.base));
      if (interval.acc) xml_note.accidental(accidental(*interval.acc));
      xml_note.staff(staff_number);

      auto xml_fingers = fingering(interval.fingers);
      if (not xml_fingers.empty()) {
        ::musicxml::technical xml_technical { };
        xml_technical.fingering(xml_fingers);
        ::musicxml::notations xml_notations { };
        xml_notations.technical().push_back(xml_technical);
        xml_note.notations().push_back(xml_notations);
      }

      if (arpeggio) {
        if (xml_note.notations().empty())
          xml_note.notations().push_back(::musicxml::notations{});
        ::musicxml::arpeggiate arp { };
        switch (*arpeggio) {
        default: throw std::runtime_error("Unknown arpeggio_type.");
        case braille::ast::chord::arpeggio_type::up:
          arp.direction(::musicxml::up_down::up);
          break;
        case braille::ast::chord::arpeggio_type::down:
          arp.direction(::musicxml::up_down::down);
          break;
        }
        xml_note.notations().back().arpeggiate().push_back(arp);
      }

      music_data.push_back(xml_note);
    }

    return music_data;
  }

  measure_type::music_data_sequence
  xml(braille::ast::moving_note const &moving_note) const
  {
    measure_type::music_data_sequence music_data;

    music_data.push_back(xml(moving_note.base));
    music_data.push_back(backup(moving_note.base.as_rational(), divisions));

    for (auto &&interval: moving_note.intervals) {
      ::musicxml::note xml_note {
        pitch(interval),
        duration(moving_note.base.as_rational() / moving_note.intervals.size(),
                 divisions)
      };

      xml_note.type(note_type(moving_note.base.get_type() / moving_note.intervals.size()));
      xml_note.dot(dots(moving_note.base));
      if (interval.acc) xml_note.accidental(accidental(*interval.acc));
      xml_note.staff(staff_number);

      auto xml_fingers = fingering(interval.fingers);
      if (not xml_fingers.empty()) {
        ::musicxml::technical xml_technical { };
        xml_technical.fingering(xml_fingers);
        ::musicxml::notations xml_notations { };
        xml_notations.technical().push_back(xml_technical);
        xml_note.notations().push_back(xml_notations);
      }

      music_data.push_back(xml_note);
    }

    return music_data;
  }
};

class musicxml_generator {
  braille::ast::score const &brl_score;
  score_type xml_score;
  ::musicxml::attributes global_attributes;
  rational divisions;

public:
  musicxml_generator(braille::ast::score const &score)
  : brl_score { score }, xml_score { ::musicxml::part_list {} }
  , global_attributes { }
    // The divisons element expresses the number of "ticks" per quarter note.
  , divisions {
      rational{1, 4} / boost::math::gcd(duration_gcd(brl_score), rational{1, 4})
    }
  {
    BOOST_ASSERT(divisions.denominator() == 1);

    xml_score.version("3.0");

    ::musicxml::encoding encoding { };
    encoding.software().push_back("Braille Music Compiler " BMC_VERSION);
    encoding.supports().push_back({::musicxml::yes_no::yes, "accidental"});
    encoding.supports().push_back({::musicxml::yes_no::no, "beam"});
    encoding.supports().push_back({::musicxml::yes_no::no, "print"});
    encoding.supports().push_back({::musicxml::yes_no::no, "stem"});
    encoding.supports().push_back({::musicxml::yes_no::no, "transpose"});
    ::musicxml::identification identification { };
    identification.encoding(encoding);
    xml_score.identification(identification);

    global_attributes.divisions(boost::rational_cast<double>(divisions));
    global_attributes.key().push_back(xml(brl_score.key_sig));
    if (not brl_score.time_sigs.empty())
      global_attributes.time().push_back(xml(brl_score.time_sigs.front()));

    unsigned c { 1 };
    for (auto &&p: brl_score.unfolded_part) {
      part_type part { "P" + std::to_string(c) };

      part.measure(get_measures(p));

      xml_score.part_list().score_part().push_back({
        "Part-" + std::to_string(c++), part.id()
      });
      xml_score.part().push_back(part);
    }
  }

  part_type::measure_sequence get_measures(braille::ast::unfolded::part const &p)
  {
    measure_type initial_measure { starts_with_anacrusis(p, brl_score)? "0": "1" };
    ::musicxml::attributes attributes { global_attributes };
    attributes.staves(p.size());
    initial_measure.music_data().push_back(attributes);

    part_type::measure_sequence measures;
    measures.push_back(initial_measure);

    unsigned staff_number { 1 };
    for (auto &&staff: p) {
      make_measures_for_staff_visitor visitor {
        brl_score, measures, staff_number, divisions
      };
      std::for_each(staff.begin(), staff.end(), apply_visitor(visitor));

      staff_number += 1;
    }

    return measures;
  }

  score_type const &score_partwise() const { return xml_score; }
};

}

void musicxml(::std::ostream &os, braille::ast::score const &score)
{
  ::musicxml::serialize(os, musicxml_generator(score).score_partwise());
}

}
