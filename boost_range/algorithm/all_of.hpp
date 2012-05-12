//  Copyright Mario Lang 2012.
//  Use, modification and distribution is subject to
//  the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef BOOST_RANGE_ALGORITHM_ALL_OF_HPP_INCLUDED
#define BOOST_RANGE_ALGORITHM_ALL_OF_HPP_INCLUDED

#include <boost/concept_check.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/concepts.hpp>
#include <algorithm>

namespace boost
{
    namespace range
    {
        /// \brief template function all_of
        ///
        /// range-based version of the all_of std algorithm
        ///
        /// \pre SinglePassRange is a model of the SinglePassRangeConcept
        /// \pre UnaryPredicate is a model of the UnaryPredicateConcept
        template< class SinglePassRange, class UnaryPredicate >
        inline bool
        all_of( SinglePassRange& rng, UnaryPredicate pred )
        {
            BOOST_RANGE_CONCEPT_ASSERT((SinglePassRangeConcept<SinglePassRange>));
            return std::all_of(boost::begin(rng), boost::end(rng), pred);
        }
    } // namespace range
    using range::all_of;
} // namespace boost

#endif // include guard
