// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_VALUE_DISAMBIGUATOR_HPP
#define BMC_VALUE_DISAMBIGUATOR_HPP

#include "bmc/braille/ast.hpp"
#include "compiler_pass.hpp"

namespace music { namespace braille {

namespace value_disambiguation {
  class measure_interpretations;
}

/**
 * \brief Calcualte the duration (value) of all notes and rests in a measure.
 *
 * When braille music was invented, 6 dot braille was dominant.  The number of
 * possible signs was therefore limited to 63 combinations (minus the empty
 * cell).  Since steps and values are encoded into a single cell, this led to
 * the necessity to overload braille patterns because it would have been
 * impossible to represent whole to 128th notes without exceeding the possible
 * combinations (7*8 + 8 is already 64).  A trick had to be invented to allow
 * the representation of all these values.  Therefore, every value in braille
 * music can have two possible meanings.  The sign for a whole note C either
 * represents a whole note C or a 16th note C.  The sign for a half note D
 * either represents a half note D or a 32th note D.  The reader of braille music
 * is assumed to disambiguate the meanings of each sign from the context.
 * Context is mostly the time signature and a bit of musical understanding.
 *
 * This compiler pass (the most complicated of all passes we have) tries to
 * find the one and only possiblle interpretation of all notes and rests in
 * a single measure given the current time signature.
 *
 * As a special case, it handles music with upbeat (anacrusis) as well.
 * If a measure appears short relative to the current time signature, its
 * interpretation is delayed until another short measure appears.  Once a second
 * measure with duration less then the current time signature is found the
 * two measures are combined, and if they add up to a duration which equals  to
 * the current time signature, both measures are disambiguated correctly.
 *
 * \ingroup compilation
 */
class value_disambiguator: public compiler_pass
{
  music::time_signature time_signature;
  value_disambiguation::measure_interpretations *anacrusis;

public:
  typedef bool result_type;
  value_disambiguator(report_error_type const& report_error);
  ~value_disambiguator();

  void set(music::time_signature const& time_sig)
  { time_signature = time_sig; }

  result_type operator()(ast::measure& measure);

  bool end_of_staff() const;
};

}}

#endif

