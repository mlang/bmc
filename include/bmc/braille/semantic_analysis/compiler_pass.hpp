// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_COMPILER_PASS_HPP
#define BMC_COMPILER_PASS_HPP

#include <functional>

namespace bmc { namespace braille {

class compiler_pass
{
protected:
  typedef std::function<void(int tag, std::wstring const& what)> report_error_type;
  report_error_type const report_error;

  compiler_pass(report_error_type const& report_error)
  : report_error(report_error)
  {}
};

}}

#endif
