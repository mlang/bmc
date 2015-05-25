#if !defined(BMC_BRAILLE_PARSER_ERROR_HANDLER_HPP)
#define BMC_BRAILLE_PARSER_ERROR_HANDLER_HPP

#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace bmc { namespace braille { namespace parser {

template <typename Iterator>
using error_handler = boost::spirit::x3::error_handler<Iterator>;

using boost::spirit::x3::error_handler_tag;

}}}

#endif

