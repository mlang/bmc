// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/musicxml.hpp"
#include "xsdcxx-musicxml/document.hxx"

namespace music {

namespace {

// We are going to export score-partwise documents.
using score_type = ::musicxml::score_partwise;
using part_type = score_type::part_type;
using measure_type = part_type::measure_type;

// Determine the greatest common divisor of all rhythmic values.
// We need this for the MusicXML divisons element.
// We're basically determining the common denominator such that all
// rhythmic values can be expressed as an integer, since the MusicXML duration
// element is not a rational.

class duration_gcd_visitor : public boost::static_visitor<void> {
  rational value = 0;
public:
  void operator()(braille::ast::unfolded::measure const &measure) {
    for (auto &&voice: measure.voices) {
      for (auto &&partial_measure: voice) {
        for (auto &&partial_voice: partial_measure) {
          std::for_each(partial_voice.begin(), partial_voice.end(), apply_visitor(*this));
        }
      }
    }
  }
  void operator()(music::braille::ast::rhythmic const &r) {
    value = boost::math::gcd(value, r.as_rational());
  }
  void operator()(braille::ast::clef const &) {}
  void operator()(braille::ast::key_and_time_signature const &) {}
  void operator()(braille::ast::tie const &) {}
  void operator()(braille::ast::tuplet_start const &) {}

  rational get() const { return value; }
};

rational duration_gcd(braille::ast::score const &score) {
  duration_gcd_visitor accumulator { };

  for (auto &&part: score.unfolded_part) {
    for (auto &&staff: part) {
      std::for_each(staff.begin(), staff.end(), apply_visitor(accumulator));
    }
  }

  return accumulator.get();
}
 
// Conversion functions between our AST and MusicXML objects:

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

::musicxml::backup backup(rational const &duration, rational const &divisions) {
  return {
    boost::rational_cast<double>(duration / (rational{1, 4} / divisions))
  };
}

class make_measures_for_staff_visitor : public boost::static_visitor<void> {
  part_type::measure_sequence &measures;
  unsigned staff_number;
  rational divisions;
  unsigned measure_number = 1;
  measure_type *current_measure;
public:
  make_measures_for_staff_visitor(part_type::measure_sequence &measures,
                                  unsigned staff_number,
                                  rational const &divisions)
  : measures { measures }, staff_number { staff_number }, divisions { divisions }
  , current_measure { nullptr }
  {}

  void operator()(braille::ast::unfolded::measure const &measure) {
    if (measure_number > measures.size())
      measures.push_back({std::to_string(measure_number)});

    current_measure = &measures[measure_number - 1];

    if (staff_number > 1)
      ::musicxml::push_back(*current_measure, backup(duration(measure), divisions));
    for (auto vi = measure.voices.begin(), ve = measure.voices.end();
         vi != ve; ++vi) {
      for (auto &&partial_measure: *vi) {
        for (auto pvi = partial_measure.begin(), pve = partial_measure.end();
             pvi != pve; ++pvi) {
          std::for_each(pvi->begin(), pvi->end(), apply_visitor(*this));
          if (std::next(pvi) != pve)
            ::musicxml::push_back(*current_measure, backup(duration(*pvi), divisions));
        }
      }
      if (std::next(vi) != ve)
        ::musicxml::push_back(*current_measure, backup(duration(*vi), divisions));
    }

    measure_number += 1;
  }
  void operator()(braille::ast::key_and_time_signature const &) {
  }
  void operator()(braille::ast::note const &note) const {
    ::musicxml::note xml_note { };
    xml_note.duration(boost::rational_cast<double>(note.as_rational() / (rational{1, 4} / divisions)));
    ::musicxml::push_back(*current_measure, xml_note);
  }
  void operator()(braille::ast::rest const &rest) const {
    ::musicxml::note xml_note { };
    xml_note.rest(::musicxml::rest{});
    xml_note.duration(boost::rational_cast<double>(rest.as_rational() / (rational{1, 4} / divisions)));
    ::musicxml::push_back(*current_measure, xml_note);
  }
  void operator()(braille::ast::chord const &chord) const {
    (*this)(chord.base);
    for (auto &&interval: chord.intervals) {
      ::musicxml::note xml_note { };
      xml_note.chord(::musicxml::empty{});
      ::musicxml::push_back(*current_measure, xml_note);
    }
  }
  void operator()(braille::ast::moving_note const &moving_note) const {
    (*this)(moving_note.base);
    ::musicxml::push_back(*current_measure, backup(moving_note.base.as_rational(), divisions));
    for (auto &&interval: moving_note.intervals) {
      ::musicxml::note xml_note { };
      xml_note.chord(::musicxml::empty{});
      ::musicxml::push_back(*current_measure, xml_note);
    }
  }
  void operator()(braille::ast::barline const &) const {
  }
  void operator()(braille::ast::clef const &) const {
  }
  void operator()(braille::hand_sign const &) const {
  }
  void operator()(braille::ast::tie const &) const {
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
    measure_type initial_measure { "1" };
    ::musicxml::attributes attributes { global_attributes };
    attributes.staves(p.size());
    ::musicxml::push_back(initial_measure, attributes);

    part_type::measure_sequence measures;
    measures.push_back(initial_measure);

    unsigned staff_number { 1 };
    for (auto &&staff: p) {
      make_measures_for_staff_visitor visitor {
        measures, staff_number, divisions
      };
      std::for_each(staff.begin(), staff.end(), apply_visitor(visitor));

      staff_number += 1;
    }

    return measures;
  }

  score_type score_partwise() const { return xml_score; }
};

}

void musicxml(std::ostream &os, music::braille::ast::score const &score)
{
  ::musicxml::score_partwise_(os, musicxml_generator(score).score_partwise());
}

}

