#if !defined(BMC_BRAILLE_PARSER_ERROR_HANDLER_HPP)
#define BMC_BRAILLE_PARSER_ERROR_HANDLER_HPP

#include <boost/spirit/home/x3/auxiliary/guard.hpp>
#include <boost/spirit/home/x3/support/context.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace bmc { namespace braille { namespace parser {

template <typename Iterator>
using error_handler = boost::spirit::x3::error_handler<Iterator>;

using boost::spirit::x3::error_handler_result;
using boost::spirit::x3::error_handler_tag;
using boost::spirit::x3::get;

struct report_on_error
{
    template <typename Iterator, typename Exception, typename Context>
    error_handler_result on_error(
        Iterator& first, Iterator const& last
      , Exception const& x, Context const& context) const
    {
        std::string which = x.which();
        std::string message = "Error! Expecting: " + which + " here:";
        auto& error_handler = get<error_handler_tag>(context).get();
        error_handler(x.where(), message);
        return error_handler_result::fail;
    }
};

}}}

#endif

