// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BMC_ANNOTATIONS_HPP)
#define BMC_ANNOTATIONS_HPP

#include <map>
#include <boost/variant/apply_visitor.hpp>
#include <boost/mpl/bool.hpp>
#include "bmc/ast.hpp"

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

  void operator()( ast::locatable& ast
                 , Iterator begin, Iterator /*end*/
                 ) const
  {
    std::size_t id = iters.size();
    iters.push_back(begin);
    ast.id = id;
  }
};

}}

#endif

