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
    template <typename>
    struct result { typedef void type; };

    error_handler(Iterator first, Iterator last)
    : first(first), last(last)
    {
    }

    template <typename Message, typename What>
    void operator()( Message const& message
		   , What const& what
		   , Iterator err_pos
		   ) const
    {
      int line;
      Iterator line_start = get_pos(err_pos, line);
      if (err_pos != last) {
        std::cout << "<INPUT>:"
                  << line << ':' << std::distance(line_start, err_pos) + 1
                  << ": "
                  << message << ": " << what << std::endl;
	std::cout << get_line(line_start) << std::endl;
	for (; line_start != err_pos; ++line_start)
	  std::cout << ' ';
	std::cout << '^' << std::endl;
      } else {
	std::cout << "Unexpected end of file. ";
	std::cout << message << what << " line " << line << std::endl;
      }
    }

    Iterator get_pos(Iterator err_pos, int& line) const
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

    std::string get_line(Iterator err_pos) const
    {
      Iterator i = err_pos;
      // position i to the next EOL
      while (i != last && (*i != '\r' && *i != '\n')) ++i;
      return std::string(err_pos, i);
    }

    Iterator first;
    Iterator last;
    std::vector<Iterator> iters;
  };

}}

#endif

