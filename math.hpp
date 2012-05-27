// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MATH_HPP
#define BMC_MATH_HPP

#include <boost/rational.hpp>

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  fmod(rational<IntType> const& lhs, rational<IntType> const& rhs)
  {
    return lhs - rhs * rational_cast<IntType>(lhs / rhs);
  }

  template <typename IntType>
  inline
  rational<IntType>
  fmod(rational<IntType> const& lhs, IntType rhs)
  {
    return lhs - rhs * rational_cast<IntType>(lhs / rhs);
  }

  template <typename IntType>
  inline
  rational<IntType>
  fmod(IntType lhs, rational<IntType> const& rhs)
  {
    return lhs - rhs * rational_cast<IntType>(lhs / rhs);
  }

  template <typename IntType>
  inline
  rational<IntType>
  gcd(rational<IntType> const& a, rational<IntType> const& b)
  {
    return !b? a: gcd(b, fmod(a, b));
  }

  template <typename IntType>
  inline
  rational<IntType>
  gcd(IntType a, rational<IntType> const& b)
  {
    return !b? rational<IntType>(a): gcd(b, fmod(a, b));
  }

  template <typename IntType>
  inline
  rational<IntType>
  gcd(rational<IntType> const& a, IntType b)
  {
    return !b? a: gcd(b, fmod(a, b));
  }

  /** \return true if lhs / rhs results in a remainder equal to zero
   */
  template <typename IntType>
  inline
  bool
  no_remainder(rational<IntType> const& lhs, rational<IntType> const& rhs)
  {
    IntType const a = lhs.numerator() * rhs.denominator(),
                  b = lhs.denominator() * rhs.numerator();
    return (a - b*(a/b)) == 0;
  }

  template <typename IntType>
  inline
  rational<IntType>
  reciprocal(rational<IntType> const& r)
  {
    return rational<IntType>(r.denominator(), r.numerator());
  }

  /** \return r to the i-th power
   */
  template <typename IntType>
  inline
  rational<IntType>
  pow(rational<IntType> const& r, IntType i)
  {
    if (!r) return r;
    rational<IntType> result(i % 2? r: rational<IntType>(1)), x(r);
    bool const positive = i >= 0;
    if (!positive) i = -i;
    while (i /= 2) {
      x *= x;
      if (i % 2) result *= x;
    }
    return positive? result: reciprocal(result);
  }
}

#endif
