// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <boost/spirit/home/support/info.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace boost { namespace spirit {

template <>
struct simple_printer < std::basic_ostream<wchar_t> >
{
  typedef utf8_string string;

  simple_printer(std::basic_ostream<wchar_t>& out) : out(out) {}

  void element(string const& tag, string const& value, int /*depth*/) const
  {
    if (value == "")
      out << wchar_t('<')
          << locale::conv::utf_to_utf<wchar_t>(tag)
          << wchar_t('>');
    else
      out << wchar_t('"')
          << locale::conv::utf_to_utf<wchar_t>(value)
          << wchar_t('"');
  }

  std::basic_ostream<wchar_t>& out;
};

}}
