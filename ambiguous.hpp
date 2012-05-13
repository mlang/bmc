// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AMBIGUOUS_HPP
#define BMC_AMBIGUOUS_HPP

#include <cmath>
#include <list>
#include <type_traits>
#include <vector>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include "music.hpp"
#include "braille_music.hpp"

namespace music { namespace braille {

/**
 * \brief Data types required to store parse result.
 *
 * Some of the missing values are filled in by the compiler translation unit
 * right now.  However, it is likely that we will need two different ways of
 * storing our data, one form immediately after parsing, and another as a result
 * of the necessary transformations on top of that.
 *
 * The namespace is called 'ambiguous' because it resembles the pure,
 * unprocessed parse result which is by the nature of braille music code
 * ambiguous (exact note and rest values need to be calculated).
 */
namespace ambiguous {

enum value
{
  whole_or_16th, half_or_32th, quarter_or_64th, eighth_or_128th, unknown
};

struct locatable
{
  std::size_t id;
  int line, column;
};

/** \brief Storage for rhythmic values.
 *
 * A rhythmic value consists of an ambiguous value type and the number of
 * augmentation dots.  The unambiguous value type is filled in after the parse
 * step by the music::braille::compiler.
 */
struct rhythmic_data
{
  ambiguous::value ambiguous_value;
  unsigned dots;
  rational type; // filled in by value_disambiguation.hpp
  rhythmic_data(): ambiguous_value(unknown), dots(0) {}
};

/** \brief Base class for everything that implies a rhythmic value.
 */
class rhythmic
{
protected:
  rhythmic() {}
  virtual ~rhythmic() {}
public:
  virtual rational as_rational() const = 0;
};

struct slur {};

struct tie : locatable {};

struct note : locatable, rhythmic_data, rhythmic
{
  std::vector<articulation> articulations;
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave_spec;
  unsigned octave; // filled in by octave_calculator.hpp
  diatonic_step step;
  int alter;       // filled in by alteration_calculator.hpp
  std::vector<slur> slurs;
  fingering_list fingers;
  boost::optional<ambiguous::tie> tie;

  note(): locatable(), rhythmic_data(), octave(0), alter(0) {}
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct rest : locatable, rhythmic_data, rhythmic
{
  rest() : whole_measure(false) {}
  bool whole_measure; // filled in by disambiguate.hpp
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct interval : locatable
{
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave_spec;
  unsigned octave; music::diatonic_step step;
  int alter;
  music::interval steps;
  fingering_list fingers;
  boost::optional<ambiguous::tie> tie;
};

struct chord : locatable, rhythmic
{
  note base;
  std::vector<interval> intervals;
  virtual rational as_rational() const
  { return base.as_rational(); }
};

struct value_distinction : locatable
{
  enum type { distinct, large_follows, small_follows };
  type value;
};

struct simile : locatable {
  boost::optional<unsigned> octave_spec;
};

enum barline { begin_repeat, end_repeat };

typedef boost::variant< note, rest, chord
                      , value_distinction, hand_sign, simile, barline> sign;

struct partial_voice : locatable, std::vector<sign> {};
struct partial_measure : locatable, std::vector<partial_voice> {};
struct voice : locatable, std::vector<partial_measure> {};

struct measure : locatable
{
  boost::optional<unsigned> ending;
  std::vector<voice> voices;
};

typedef std::vector< boost::variant<measure> > staff;

typedef std::vector<staff> part;

struct score {
  key_signature key_sig;
  boost::optional<time_signature> time_sig;
  std::vector<part> parts;
};

}}}

namespace music {
  namespace braille {
    namespace ambiguous {

      /// Visitors

      struct get_line : boost::static_visitor<int>
      {
        result_type operator()(locatable const& lexeme) const { return lexeme.line; }
        result_type operator()(barline const&) const { return 0; }
        result_type operator()(hand_sign const&) const { return 0; }
      };

      struct get_column : boost::static_visitor<int>
      {
        result_type operator()(locatable const& lexeme) const { return lexeme.column; }
        result_type operator()(barline const&) const { return 0; }
        result_type operator()(hand_sign const&) const { return 0; }
      };

      struct get_ambiguous_value : boost::static_visitor<value>
      {
        result_type operator()(note const& note) const
        { return note.ambiguous_value; }
        result_type operator()(rest const& rest) const
        { return rest.ambiguous_value; }
        result_type operator()(chord const& chord) const
        { return (*this)(chord.base); }
        template<typename T>
        result_type operator()(T const&) const
        { return unknown; }
      };

      struct is_rest : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_same<rest, T>::value; }
      };

      struct is_rhythmic : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_base_of<rhythmic, T>::value; }
      };

      class is_value_distinction : public boost::static_visitor<bool>
      {
        bool check;
        value_distinction::type expected;
      public:
        is_value_distinction()
        : check(false) {}

        is_value_distinction(value_distinction::type distinction)
        : check(true)
        , expected(distinction) {}

        result_type operator() (value_distinction const& distinction) const
        { return not check? true: distinction.value == expected; }

        template <class Sign>
        result_type operator()(Sign const&) const
        { return false; }
      };
    }
  }
}

namespace music {
  namespace braille {
    namespace ambiguous {
      struct get_duration: boost::static_visitor<rational>
      {
        result_type operator() (rhythmic const& note) const
        { return note.as_rational(); }
        result_type operator() (barline const&) const { return result_type(); }
        result_type operator() (hand_sign const&) const { return result_type(); }
        result_type operator() (simile const&) const { return result_type(); }
        result_type operator() (value_distinction const&) const { return result_type(); }

        result_type operator() (measure const&) const;
      };

      inline
      rational
      duration(partial_voice::const_reference sign)
      {
        return apply_visitor(get_duration(), sign);
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , music::braille::ambiguous::partial_voice::const_reference sign
            )
  {
    return r + duration(sign);
  }
}

#include <boost/range/numeric.hpp>

namespace music {
  namespace braille {
    namespace ambiguous {
      inline
      rational
      duration(partial_voice const& partial_voice)
      {
        // BOOST_ASSERT(not partial_voice.empty());
        return boost::accumulate(partial_voice, rational());
      }

      inline
      rational
      duration(partial_measure const& partial_measure)
      {
        BOOST_ASSERT(not partial_measure.empty());
        return duration(partial_measure.front());
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , music::braille::ambiguous::voice::const_reference partial_measure
            )
  {
    return r + duration(partial_measure);
  }
}

namespace music {
  namespace braille {
    namespace ambiguous {
      inline
      rational
      duration(voice const& voice)
      {
        return boost::accumulate(voice, rational());
      }

      inline
      rational
      duration(measure const& measure)
      {
        BOOST_ASSERT(not measure.voices.empty());
        return duration(measure.voices.front());
      }

      inline
      get_duration::result_type
      get_duration::operator() (measure const& measure) const
      {
        return duration(measure);
      }

      inline
      rational
      duration(staff::const_reference staff_element)
      {
        return boost::apply_visitor(get_duration(), staff_element);
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , music::braille::ambiguous::staff::const_reference staff_element
            )
  {
    return r + duration(staff_element);
  }
}

namespace music {
  namespace braille {
    namespace ambiguous {
      inline
      rational
      duration(staff const& staff)
      {
        return boost::accumulate(staff, rational());
      }
    }
  }
}

#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::rest,
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::note,
  (std::vector<music::articulation>, articulations)
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::diatonic_step, step)
  (music::braille::ambiguous::value, ambiguous_value)
  (unsigned, dots)
  (std::vector<music::braille::ambiguous::slur>, slurs)
  (music::braille::fingering_list, fingers)
  (boost::optional<music::braille::ambiguous::tie>, tie)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::interval,
  (boost::optional<music::accidental>, acc)
  (boost::optional<unsigned>, octave_spec)
  (music::interval, steps)
  (music::braille::fingering_list, fingers)
  (boost::optional<music::braille::ambiguous::tie>, tie)
)
BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::chord,
  (music::braille::ambiguous::note, base)
  (std::vector<music::braille::ambiguous::interval>, intervals)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::value_distinction,
  (music::braille::ambiguous::value_distinction::type, value)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::simile,
  (boost::optional<unsigned>, octave_spec)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::measure,
  (boost::optional<unsigned>, ending)
  (std::vector<music::braille::ambiguous::voice>, voices)
)

BOOST_FUSION_ADAPT_STRUCT(
  music::braille::ambiguous::score,
  (music::key_signature, key_sig)
  (boost::optional<music::time_signature>, time_sig)
  (std::vector<music::braille::ambiguous::part>, parts)
)

#endif
