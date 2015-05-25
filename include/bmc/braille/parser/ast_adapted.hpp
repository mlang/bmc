#if !defined(BMC_BRAILLE_PARSER_AST_ADAPTED_HPP)
#define BMC_BRAILLE_PARSER_AST_ADAPTED_HPP

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
  bmc::braille::parser::ast::time_signature,
  (int, numerator)
  (int, denominator)
)

BOOST_FUSION_ADAPT_STRUCT(
  bmc::braille::parser::ast::key_signature,
  (int, fifths)
)

#endif
