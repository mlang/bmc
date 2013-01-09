// Copyright (C) 2012, 2013  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AST_AST_HPP
#define BMC_AST_AST_HPP

#include <cmath>
#include <type_traits>
#include <vector>

#include <boost/mpl/remove_if.hpp>
#include <boost/optional.hpp>
#include <boost/variant/variant.hpp>

#include "bmc/music.hpp"
#include "bmc/braille_music.hpp"

namespace music { namespace braille {

/**
 * \brief Data types required to store parse result.
 *
 * Some of the missing values are filled in by the compiler translation unit
 * right now.  However, it is likely that we will need two different ways of
 * storing our data, one form immediately after parsing, and another as a result
 * of the necessary transformations on top of that.
 */
namespace ast {

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
  ast::value ambiguous_value;
  unsigned dots;
  rational type; // filled in by value_disambiguation.hpp
  rhythmic_data(): ambiguous_value(unknown), dots(0), type() {}
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

struct slur : locatable
{
  enum type { single, cross_staff };
  type value;
};

struct tie : locatable
{
  enum type { single, chord, arpeggio };
  type value;
};

struct pitched
{
  boost::optional<accidental> acc;
  boost::optional<unsigned> octave_spec;
  unsigned octave;
  diatonic_step step;
  int alter;
  boost::optional<ast::tie> tie;
};

struct stem : rhythmic
{
  rational type;
  unsigned dots;
  boost::optional<ast::tie> tied;

  rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct note : locatable, rhythmic_data, rhythmic, pitched
{
  std::vector<articulation> articulations;
  std::vector<slur> slurs;
  fingering_list fingers;
  std::vector<stem> extra_stems;

  note(): locatable(), rhythmic_data(), pitched() {}
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct rest : locatable, rhythmic_data, rhythmic
{
  rest(): locatable(), rhythmic_data(), whole_measure(false) {}
  bool whole_measure; // filled in by disambiguate.hpp
  virtual rational as_rational() const
  { return type * 2 - type / pow(2, dots); }
};

struct interval : locatable, pitched
{
  music::interval steps;
  fingering_list fingers;
};

struct chord_tied {};

struct chord : locatable, rhythmic
{
  note base;
  std::vector<interval> intervals;
  boost::optional<chord_tied> all_tied;

  virtual rational as_rational() const
  { return base.as_rational(); }
};

/** The moving-note device, although infrequently employed,
 *  is chiefly useful for vocal music and keyboard settings of
 *  hymns.  Complications of fingering, phrasing and nuances render it
 *  unsuitable for instrumental music in general.
 */
struct moving_note : locatable, rhythmic
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

struct simile : locatable
{
  boost::optional<unsigned> octave_spec;
  unsigned count;

  rational duration;                    // Filled by value_disambiguation
};

enum barline { begin_repeat, end_repeat, end_part };

typedef boost::variant< note, rest, chord, moving_note
                      , value_distinction, tie
                      , hand_sign, simile, barline
                      >
        sign;

struct partial_voice : locatable, std::vector<sign> {};
struct partial_measure : locatable, std::vector<partial_voice> {};
struct voice : locatable, std::vector<partial_measure> {};

struct measure : locatable
{
  boost::optional<unsigned> ending;
  std::vector<voice> voices;
};

struct key_and_time_signature : locatable
{
  key_signature key;
  time_signature time;
};

typedef boost::variant<measure, key_and_time_signature> staff_element;
typedef std::vector<staff_element> staff;

typedef std::vector<staff> part;

typedef boost::variant<measure, key_and_time_signature> paragraph_element;
typedef std::vector<paragraph_element> paragraph;

struct section
{
  typedef unsigned number_type;
  typedef std::pair<number_type, number_type> range_type;
  boost::optional<number_type> number;
  boost::optional<range_type> range;
  std::vector<paragraph> paragraphs;
};

/** These data structures correspond to an unrolled representation which
 *  replaces partial measure simile with the musical material they represent.
 */
namespace unfolded {

typedef boost::mpl::or_< boost::is_same<boost::mpl::_, value_distinction>
                       , boost::is_same<boost::mpl::_, simile>
                       >
        is_simile_or_value_distinction;

typedef boost::make_variant_over
        < boost::mpl::remove_if< ast::sign::types
                               , is_simile_or_value_distinction
                               >::type
        >::type
        sign;

struct partial_voice : locatable, std::vector<sign>
{
  partial_voice()
  : std::vector<sign>()
  {}
  partial_voice(const_iterator begin, const_iterator const &end)
  : std::vector<sign>(begin, end)
  {}
};
struct partial_measure : locatable, std::vector<partial_voice> {};
struct voice : locatable, std::vector<partial_measure> {};

struct measure : locatable
{
  boost::optional<unsigned> ending;
  std::vector<voice> voices;
  unsigned count;

  measure()
  : count(1)
  {}
};

typedef boost::variant<measure, ast::key_and_time_signature> staff_element;
struct staff : std::vector<staff_element> {};
struct part : std::vector<staff> {};

}

struct score {
  key_signature key_sig;
  boost::optional<time_signature> time_sig;
  std::vector<part> parts;
  std::vector<unfolded::part> unfolded_part;
};

}}}

#endif
