// Copyright (C) 2015  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_BRAILLE_LINEBREAKING_HPP_INCLUDED
#define BMC_BRAILLE_LINEBREAKING_HPP_INCLUDED

#include <deque>
#include <limits>
#include <memory>
#include <vector>

namespace bmc { namespace braille { namespace linebreaking {

struct object {
public:
  virtual ~object() {}
};

struct box: public object {
  virtual int width() const = 0;
};

struct glue: public object {
  virtual int width() const = 0;
};

int constexpr infinity = std::numeric_limits<int>::max() - 1;

struct penalty: public object {
  virtual int width() const = 0;
  virtual int value() const = 0;
};

using objects = std::vector<std::unique_ptr<object>>;

std::deque<objects::const_iterator>
breakpoints(objects const &objs, std::vector<unsigned> const &line_lengths);

}}}

#endif
