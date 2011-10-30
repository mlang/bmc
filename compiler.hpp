// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_COMPILER_HPP
#define BMC_COMPILER_HPP

#include "ambiguous.hpp"
#include "disambiguate.hpp"
#include "octave_calculator.hpp"
#include <boost/function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

class compiler
{
  boost::function<void(int tag, std::wstring const& what)> report_error;
  measure_interpretations anacrusis;
  octave_calculator calculate_octaves;
public:
  music::time_signature global_time_signature;
  typedef bool result_type;

  template<typename ErrorHandler>
  compiler(ErrorHandler& error_handler)
  : report_error( boost::phoenix::function<ErrorHandler>(error_handler)
                  ( L"Error"
                  , boost::phoenix::arg_names::_2
                  , boost::phoenix::cref(error_handler.iters)
                    [boost::phoenix::arg_names::_1]
                  )
                )
  , global_time_signature(4, 4)
  , anacrusis()
  , calculate_octaves(report_error)
  {
  }

  result_type operator()(ambiguous::score& score);
  result_type operator()(ambiguous::measure& measure);

private:
  result_type disambiguate(ambiguous::measure& measure);
};

}}


#endif
