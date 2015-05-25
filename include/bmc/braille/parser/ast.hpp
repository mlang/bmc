#if !defined(BMC_BRAILLE_PARSER_AST_HPP)
#define BMC_BRAILLE_PARSER_AST_HPP

#include <bmc/music.hpp>
#include <bmc/braille_music.hpp>
#include <boost/optional.hpp>
#include <boost/rational.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

namespace bmc { namespace braille { namespace parser { namespace ast {

using boost::spirit::x3::position_tagged;
using boost::spirit::x3::variant;

struct time_signature : position_tagged
{
  int numerator;
  int denominator;

  operator boost::rational<int>() const { return {numerator, denominator}; }
};

struct key_signature : position_tagged
{
  int fifths;
};

enum ambiguous_value
{
  whole_or_16th, half_or_32th, quarter_or_64th, eighth_or_128th, unknown_ambiguous_value
};

enum class notegroup_member_type: uint8_t {
  none, begin, middle, end
};

enum class slur_member_type: uint8_t {
  none, begin, middle, end
};

struct rhythmic_storage
{
  ast::ambiguous_value ambiguous_value = unknown_ambiguous_value;
  unsigned dots = 0;
  rational type = rational(); // filled in by value_disambiguation.hpp
  rational factor = rational(1);
  notegroup_member_type notegroup_member = notegroup_member_type::none; // filled in by value_disambiguation.hpp
  slur_member_type slur_member = slur_member_type::none;
  std::vector<rational> tuplet_begin;
  unsigned tuplet_end = 0;
};

class rhythmic
{
protected:
  virtual ~rhythmic();
public:
  virtual rational as_rational() const = 0;
  virtual unsigned get_dots() const = 0;
  virtual rational get_type() const = 0;
  virtual rational get_factor() const = 0;
};

struct slur final : position_tagged
{
  enum type { single, cross_staff };
  type value;
};

struct tie final : position_tagged
{
  enum type { single, chord, arpeggio };
  type value;
};

struct hand_sign final : position_tagged
{
  enum type { right_hand, left_hand };
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

struct stem final : rhythmic
{
  rational type;
  unsigned dots;
  boost::optional<ast::tie> tied;

  rational as_rational() const override
  { return type * augmentation_dots_factor(dots); }
  unsigned get_dots() const override
  { return dots; }
  rational get_factor() const override { return 1; }
  rational get_type() const override { return type; }
};

struct note final : position_tagged, rhythmic_storage, rhythmic, pitched
{
  std::vector<articulation> articulations;
  std::vector<slur> slurs;
  fingering_list fingers;
  std::vector<stem> extra_stems;

  note(): position_tagged(), rhythmic_storage(), pitched() {}
  rational as_rational() const override
  { return type * augmentation_dots_factor(dots) * factor; }
  unsigned get_dots() const override { return dots; }
  rational get_factor() const override { return factor; }
  rational get_type() const override { return type; }
};

struct rest final : position_tagged, rhythmic_storage, rhythmic
{
  bool by_transcriber;

  rest(): position_tagged(), rhythmic_storage(), whole_measure(false) {}
  virtual ~rest();

  bool whole_measure; // filled in by disambiguate.hpp

  rational as_rational() const override
  { return type * augmentation_dots_factor(dots) * factor; }
  unsigned get_dots() const override
  { return dots; }
  rational get_factor() const override { return factor; }
  rational get_type() const override
  { return whole_measure? rational{1}: type; }
};

struct interval : position_tagged, pitched
{
  ::bmc::interval steps;
  fingering_list fingers;
};

struct chord final : position_tagged, rhythmic
{
  note base;
  std::vector<interval> intervals;
  bool all_tied = false;

  rational as_rational() const override { return base.as_rational(); }
  unsigned get_dots() const override { return base.get_dots(); }
  rational get_factor() const override { return base.get_factor(); }
  rational get_type() const override { return base.get_type(); }

  enum class arpeggio_type { up, down };
  boost::optional<arpeggio_type> arpeggio() const {
    if (std::find(base.articulations.begin(), base.articulations.end(),
                  ::bmc::arpeggio_up) != base.articulations.end())
      return arpeggio_type::up;
    if (std::find(base.articulations.begin(), base.articulations.end(),
                  ::bmc::arpeggio_down) != base.articulations.end())
      return arpeggio_type::down;
    return {};
  }
};

struct moving_note final : position_tagged, rhythmic
{
  note base;
  std::vector<interval> intervals;

  rational as_rational() const override { return base.as_rational(); }
  unsigned get_dots() const override { return base.get_dots(); }
  rational get_factor() const override { return base.get_factor(); }
  rational get_type() const override { return base.get_type(); }
};

struct value_distinction : position_tagged
{
  enum type { distinct, large_follows, small_follows };
  type value;
};

struct hyphen : position_tagged {};

struct simile : position_tagged
{
  boost::optional<unsigned> octave_spec;
  unsigned count;

  rational duration;                    // Filled by value_disambiguation
};

struct barline : position_tagged
{
  enum type { begin_repeat, end_repeat, end_part };
  type value;
};

class tuplet_start : public position_tagged
{
  bool simple_triplet_;
  bool doubled_;
  unsigned number_;
public:
  tuplet_start(bool doubled = false)
  : simple_triplet_{true}, doubled_{doubled} {}
  tuplet_start(unsigned number, bool doubled)
  : simple_triplet_{false}, doubled_{doubled}, number_{number} {}

  unsigned number() const { return simple_triplet_? 3: number_; }
  bool simple_triplet() const { return simple_triplet_; }
  bool doubled() const { return doubled_; }
};

class clef : public position_tagged
{
public:
  enum class type { G, C, F };
private:
  type sign_;
  boost::optional<unsigned> line_;
  boost::optional<unsigned> staff_;
public:
  clef(): sign_{type::G} {}
  clef(type sign): sign_{sign} {}
  clef(type sign, unsigned line): sign_{sign}, line_{line} {}

  type sign() const { return sign_; }
  unsigned line() const {
    if (line_) return *line_;
    switch(sign_) {
    case type::G: return 2;
    case type::F: return 4;
    case type::C: return 3;
    }
  }
  clef &staff(boost::optional<unsigned> const &n) {
    staff_ = n;
    return *this;
  }
  boost::optional<unsigned> const &staff() const { return staff_; }
};

struct sign : variant<
  note, rest, chord, moving_note
, value_distinction, hyphen, tie, tuplet_start
, hand_sign, clef, simile, barline
>
{};

struct partial_voice : position_tagged, std::vector<sign> {};
struct partial_measure : position_tagged, std::vector<partial_voice> {};
struct voice : position_tagged, std::vector<partial_measure> {};

struct measure : position_tagged
{
  boost::optional<unsigned> ending;
  std::vector<voice> voices;
};

struct key_and_time_signature : position_tagged
{
  key_signature key;
  time_signature time;
};

struct paragraph_element : variant<
  measure, key_and_time_signature
>
{};

struct paragraph : position_tagged, std::vector<paragraph_element> {};

struct measure_specification : position_tagged
{
  typedef unsigned number_type;
  number_type number;
  boost::optional<number_type> alternative;
};

struct measure_range : position_tagged
{
  measure_specification first;
  measure_specification last;
};

struct section : position_tagged
{
  typedef unsigned number_type;
  boost::optional<key_and_time_signature> key_and_time_sig;
  boost::optional<number_type> number;
  boost::optional<measure_range> range;
  std::vector<paragraph> paragraphs;
};

struct part : position_tagged, std::vector<section> {};

struct score {
  key_signature key_sig;
  std::vector<time_signature> time_sigs;
  std::vector<part> parts;
};

}}}}

#endif

