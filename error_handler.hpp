// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BMC_ERROR_HANDLER_HPP)
#define BMC_ERROR_HANDLER_HPP

#include <iostream>
#include <string>
#include <vector>

namespace music { namespace braille {

  ///////////////////////////////////////////////////////////////////////////////
  //  The error handler
  ///////////////////////////////////////////////////////////////////////////////
  template <typename Iterator>
  struct error_handler
  {
    typedef Iterator iterator_type;

    template <typename>
    struct result { typedef void type; };

    error_handler(iterator_type first, iterator_type last)
    : first(first), last(last)
    {
    }

    template <typename Message, typename What>
    void operator()( Message const& message
		   , What const& what
		   , iterator_type err_pos
		   ) const
    {
      int line;
      iterator_type line_start = get_pos(err_pos, line);
      if (err_pos != last) {
        std::wcerr << L"<INPUT>:"
                  << line << L':' << std::distance(line_start, err_pos) + 1
                  << L": "
                  << message << L": " << what
                  << std::endl;
	std::wcerr << get_line(line_start) << std::endl;
	for (; line_start != err_pos; ++line_start) std::wcerr << L' ';
	std::wcerr << '^' << std::endl;
      } else {
	std::wcerr << "Unexpected end of file. ";
	std::wcerr << message << what << L" line " << line << std::endl;
      }
    }

    iterator_type get_pos(Iterator err_pos, int& line) const
    {
      line = 1;
      Iterator i = first;
      Iterator line_start = first;
      while (i != err_pos) {
	bool eol = false;
	if (i != err_pos && *i == '\r') {
	  eol = true;
	  line_start = ++i;
	}
	if (i != err_pos && *i == '\n') {
	  eol = true;
	  line_start = ++i;
	}
	if (eol) ++line; else ++i;
      }
      return line_start;
    }

    std::wstring get_line(iterator_type err_pos) const
    {
      iterator_type i = err_pos;
      // position i to the next EOL
      while (i != last && (*i != '\r' && *i != '\n')) ++i;
      return std::wstring(err_pos, i);
    }

    iterator_type first, last;
    std::vector<iterator_type> iters;
  };

}}

#endif

