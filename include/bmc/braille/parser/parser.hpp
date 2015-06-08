#if !defined(BMC_BRAILLE_PARSER_PARSER_HPP)
#define BMC_BRAILLE_PARSER_PARSER_HPP

#include "ast.hpp"
#include <bmc/braille/text2braille.hpp>
#include <boost/optional.hpp>
#include "error_handler.hpp"
#include <tuple>

namespace bmc { namespace braille { namespace parser {

using iterator_type = std::u32string::const_iterator;

template <typename T, typename Iterator = iterator_type>
using result_t = std::tuple<boost::optional<T>, error_handler<Iterator>>;

}

auto parse_time_signature(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::time_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

auto parse_key_signature(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::key_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

auto parse_note(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::note,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

auto parse_key_and_time_signature(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::key_and_time_signature,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

auto parse_measure(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::measure,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

auto parse_score(std::u32string const& input,
  std::ostream &out, std::string filename = "<INPUT>", bool full_match = true
) -> parser::result_t<parser::ast::score,
                      std::remove_reference<decltype(input)>::type::const_iterator>;

}}

#endif

