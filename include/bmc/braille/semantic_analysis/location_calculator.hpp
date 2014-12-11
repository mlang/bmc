// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_LOCATION_CALCULATOR_HPP
#define BMC_LOCATION_CALCULATOR_HPP

#include <map>
#include <boost/variant/static_visitor.hpp>
#include "bmc/braille/ast.hpp"
#include "compiler_pass.hpp"
#include "bmc/braille/parsing/error_handler.hpp"

namespace bmc { namespace braille {

/**
 * \brief Set line and column numbers for all locatable objects.
 *
 * \ingroup compilation
 */
template <typename ErrorHandler>
class location_calculator
: public boost::static_visitor<void>
, public compiler_pass
{
  ErrorHandler const& handler;
public:
  location_calculator( report_error_type const& report_error
                     , ErrorHandler const& handler
                     )
  : compiler_pass(report_error)
  , handler(handler)
  {}

  result_type operator()(ast::measure& measure) const
  {
    for (ast::voice& voice: measure.voices) {
      for (ast::partial_measure& part: voice) {
        for (ast::partial_voice& partial_voice: part) {
          std::for_each(partial_voice.begin(), partial_voice.end(),
                        boost::apply_visitor(*this));
          (*this)(partial_voice);
        }
        (*this)(part);
      }
      (*this)(voice);
    }
    (*this)(static_cast<ast::locatable&>(measure));
  }

  result_type operator()(ast::locatable& lexeme) const
  {
    typedef typename ErrorHandler::iterator_type iterator_type;
    if (lexeme.id >= 0) {
      BOOST_ASSERT(lexeme.id < handler.iters.size());
      iterator_type const error_position(handler.iters[lexeme.id]);
      iterator_type const line_start(handler.get_pos(error_position, lexeme.line));
      lexeme.column = std::distance(line_start, error_position) + 1;
    }
  }

  result_type operator()(ast::barline&) const { }
  result_type operator()(hand_sign&) const { }
};

}}

#endif
