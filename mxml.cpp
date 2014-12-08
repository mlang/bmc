// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/musicxml.hpp"
#include "xsdcxx-musicxml/document.hxx"

namespace music {

namespace {

using score_type = ::musicxml::score_partwise;
using part_type = score_type::part_type;
using measure_type = part_type::measure_type;

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
public:
  make_measures_for_staff_visitor(part_type::measure_sequence &measures,
                                  unsigned staff_number,
                                  rational const &divisions)
  : measures { measures }, staff_number { staff_number }, divisions { divisions } {}
  void operator()(braille::ast::unfolded::measure const &measure) {
    if (measure_number > measures.size())
      measures.push_back({std::to_string(measure_number)});

    measure_type &xml_measure = measures[measure_number - 1];

    if (staff_number > 1)
      ::musicxml::push_back(xml_measure, backup(duration(measure), divisions));
    for (auto vi = measure.voices.begin(), ve = measure.voices.end();
         vi != ve; ++vi) {
      for (auto &&partial_measure: *vi) {
        for (auto &&partial_voice: partial_measure) {
        }
      }
      if (std::next(vi) != ve)
        ::musicxml::push_back(xml_measure, backup(duration(*vi), divisions));
    }

    measure_number += 1;
  }
  void operator()(braille::ast::key_and_time_signature const &) {
  }
};

part_type::measure_sequence get_measures(braille::ast::unfolded::part const &p, rational const &divisions) {
  measure_type initial_measure { "1" };
  ::musicxml::attributes attributes { };
  attributes.divisions(boost::rational_cast<double>(divisions));
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

}

void musicxml(std::ostream &os, music::braille::ast::score const &score)
{
  rational const divisions {
    rational{1, 4} / boost::math::gcd(duration_gcd(score), rational{1, 4})
  };
  BOOST_ASSERT(divisions.denominator() == 1);

  ::musicxml::attributes global_attributes {};
  global_attributes.key().push_back(xml(score.key_sig));
  std::clog << global_attributes;
  score_type xml_score { ::musicxml::part_list {} };
  unsigned c { 1 };
  for (auto &&p: score.unfolded_part) {
    part_type part { "P" + std::to_string(c) };

    part.measure(get_measures(p, divisions));

    xml_score.part_list().score_part().push_back({
      "Part-" + std::to_string(c++), part.id()
    });
    xml_score.part().push_back(part);
  }
  ::musicxml::score_partwise_(os, xml_score);
}

}

