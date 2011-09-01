// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <boost/spirit/home/support/info.hpp>
#include <boost/locale/encoding_utf.hpp>

// See https://svn.boost.org/trac/boost/ticket/5716

namespace boost { namespace spirit {

template<>
struct simple_printer<std::wostream>
{
  typedef utf8_string string;

  simple_printer(std::wostream& out) : out(out) {}

  void element(string const& tag, string const& value, int /*depth*/) const
  {
    typedef std::wostream::char_type char_type;
    if (value == "")
      out << char_type('<')
          << locale::conv::utf_to_utf<char_type>(tag)
          << char_type('>');
    else
      out << char_type('"')
          << locale::conv::utf_to_utf<char_type>(value)
          << char_type('"');
  }

  std::wostream& out;
};

}}
