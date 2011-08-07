#if !defined(BMC_ANNOTATIONS_HPP)
#define BMC_ANNOTATIONS_HPP

#include <map>
#include <boost/variant/apply_visitor.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/mpl/bool.hpp>
#include "ambiguous.hpp"

namespace music { namespace braille {

template <typename Iterator>
struct annotation
{
  template <typename>
  struct result { typedef void type; };

  std::vector<Iterator>& iters;
  annotation(std::vector<Iterator>& iters)
  : iters(iters)
  {
  }

  void operator()(ambiguous::locatable& ast, Iterator pos) const
  {
    std::size_t id = iters.size();
    iters.push_back(pos);
    ast.id = id;
  }
};

}}

#endif

