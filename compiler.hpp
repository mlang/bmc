#ifndef BMC_COMPILER_HPP
#define BMC_COMPILER_HPP

#include "ambiguous.hpp"
#include <boost/function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace music { namespace braille {

class compiler
{
  boost::function<void(int tag, std::wstring const& what)> report_error;
public:
  music::time_signature global_time_signature;
  typedef bool result_type;
  template<typename ErrorHandler>
  compiler(ErrorHandler& error_handler)
  : global_time_signature(4, 4)
  {
    using boost::phoenix::arg_names::_1;
    using boost::phoenix::arg_names::_2;
    report_error =
      boost::phoenix::function<ErrorHandler>(error_handler)
      (L"Error", _2, boost::phoenix::cref(error_handler.iters)[_1]);
  }

  result_type operator()(ambiguous::measure& measure) const;
  result_type operator()(ambiguous::score& score) const;
};

}}


#endif
