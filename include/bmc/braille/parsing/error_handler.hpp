// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BMC_ERROR_HANDLER_HPP)
#define BMC_ERROR_HANDLER_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace bmc { namespace braille {

  ///////////////////////////////////////////////////////////////////////////////
  //  The error handler
  ///////////////////////////////////////////////////////////////////////////////
  template <typename Iterator>
  struct error_handler
  {
    typedef Iterator iterator_type;
    typedef std::basic_string<typename iterator_type::value_type> string_type;

    template <typename>
    struct result { typedef void type; };

    error_handler(iterator_type f, iterator_type l)
    : first(f), last(l)
    , messages{std::make_shared<std::vector<string_type>>()}
    {}

    template <typename Message, typename What>
    void operator()( Message const& message
                   , What const& what
                   , iterator_type err_pos
                   ) const
    {
      int line;
      iterator_type line_start = get_pos(err_pos, line);
      if (err_pos != last) {
        std::wstringstream ss;
        ss << L"<INPUT>:"
           << line << L':' << std::distance(line_start, err_pos) + 1
           << L": "
           << message << L": " << what;
        messages->push_back(ss.str());
        ss.str(string_type());

        messages->push_back(get_line(line_start));
        for (; line_start != err_pos; ++line_start) ss << L' ';
        ss << '^';
        messages->push_back(ss.str());
      } else {
        std::wstringstream ss;
        ss << "Unexpected end of file. ";
        ss << message << what << L" line " << line;
        messages->push_back(ss.str());
      }
    }

    iterator_type get_pos(iterator_type err_pos, int& line) const
    {
      line = 1;
      iterator_type i = first;
      iterator_type line_start = first;
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

    string_type get_line(iterator_type line_start) const
    {
      iterator_type line_end = line_start;
      for (; line_end != last && (*line_end != '\r' && *line_end != '\n');
           ++line_end);
      return string_type(line_start, line_end);
    }

    friend std::wostream &operator<<(std::wostream &os, error_handler const &eh)
    {
      for (auto const &line: *eh.messages) os << line << std::endl;
      return os;
    }

    iterator_type first, last;
    std::vector<iterator_type> iters;
    std::shared_ptr<std::vector<string_type>> messages;
  };

}}

#endif

