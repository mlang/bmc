// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_MATH_HPP
#define BMC_MATH_HPP

#include <boost/integer/common_factor_rt.hpp>
#include <boost/rational.hpp>

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  fmod(rational<IntType> const& lhs, rational<IntType> const& rhs)
  {
    return lhs - rhs * rational_cast<IntType>(lhs / rhs);
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

  namespace integer {
    template <typename IntType>
    struct gcd_evaluator< rational<IntType> >
    {
      typedef rational<IntType> result_type,
                                first_argument_type, second_argument_type;
      result_type operator() (  first_argument_type const &a
                             , second_argument_type const &b
                             ) const
      {
        return result_type(gcd(a.numerator(), b.numerator()),
                           lcm(a.denominator(), b.denominator()));
      }
    };

    template <typename IntType>
    struct lcm_evaluator< rational<IntType> >
    {
      typedef rational<IntType> result_type,
                                first_argument_type, second_argument_type;
      result_type operator() (  first_argument_type const &a
                             , second_argument_type const &b
                             ) const
      {
        return result_type(lcm(a.numerator(), b.numerator()),
                           gcd(a.denominator(), b.denominator()));
      }
    };
  }

  // http://en.wikipedia.org/wiki/Dyadic_rational
  template <typename IntType>
  inline bool is_dyadic(rational<IntType> const &r)
  { return (r.denominator() & -r.denominator()) == r.denominator(); }
}

#endif
