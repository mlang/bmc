// Copyright (C) 2012, 2013  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AST_DURATION_HPP
#define BMC_AST_DURATION_HPP

#include "bmc/braille/ast/ast.hpp"
#include <boost/variant/apply_visitor.hpp>

namespace bmc {
  namespace braille {
    namespace ast {
      struct get_duration: boost::static_visitor<rational>
      {
        result_type operator() (rhythmic const& note) const
        { return note.as_rational(); }
        result_type operator() (barline const&) const { return result_type(); }
        result_type operator() (hand_sign const&) const { return result_type(); }
        result_type operator() (clef const&) const { return result_type(); }
        result_type operator() (simile const&) const { return result_type(); }
        result_type operator() (value_prefix const&) const { return result_type(); }
        result_type operator() (hyphen const&) const { return result_type(); }
        result_type operator() (tie const &) const { return result_type(); }
        result_type operator() (tuplet_start const &) const { return result_type(); }

        result_type operator() (measure const&) const;
        result_type operator() (unfolded::measure const&) const;
        result_type operator() (key_and_time_signature const&) const { return result_type(); }
      };

      inline
      rational
      duration(partial_voice::const_reference sign)
      {
        return apply_visitor(get_duration(), sign);
      }
      inline
      rational
      duration(unfolded::partial_voice::const_reference sign)
      {
        return apply_visitor(get_duration(), sign);
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::partial_voice::const_reference sign
            )
  {
    return r + duration(sign);
  }
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::unfolded::partial_voice::const_reference sign
            )
  {
    return r + duration(sign);
  }
}

#include <boost/range/numeric.hpp>

namespace bmc {
  namespace braille {
    namespace ast {
      inline
      rational
      duration(partial_voice const& partial_voice)
      {
        // BOOST_ASSERT(!partial_voice.empty());
        return boost::accumulate(partial_voice, rational());
      }
      inline
      rational
      duration(unfolded::partial_voice const& partial_voice)
      {
        // BOOST_ASSERT(!partial_voice.empty());
        return boost::accumulate(partial_voice, rational());
      }

      inline
      rational
      duration(partial_measure const& partial_measure)
      {
        BOOST_ASSERT(!partial_measure.empty());
        return duration(partial_measure.front());
      }
      inline
      rational
      duration(unfolded::partial_measure const& partial_measure)
      {
        BOOST_ASSERT(!partial_measure.empty());
        return duration(partial_measure.front());
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::voice::const_reference partial_measure
            )
  {
    return r + duration(partial_measure);
  }
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::unfolded::voice::const_reference partial_measure
            )
  {
    return r + duration(partial_measure);
  }
}

namespace bmc {
  namespace braille {
    namespace ast {
      inline
      rational
      duration(voice const& voice)
      {
        return boost::accumulate(voice, rational());
      }
      inline
      rational
      duration(unfolded::voice const& voice)
      {
        return boost::accumulate(voice, rational());
      }

      inline
      rational
      duration(measure const& measure)
      {
        BOOST_ASSERT(!measure.voices.empty());
        return duration(measure.voices.front());
      }
      inline
      rational
      duration(unfolded::measure const& measure)
      {
        BOOST_ASSERT(!measure.voices.empty());
        return duration(measure.voices.front());
      }

      inline
      get_duration::result_type
      get_duration::operator() (measure const& measure) const
      {
        return duration(measure);
      }
      inline
      get_duration::result_type
      get_duration::operator() (unfolded::measure const& measure) const
      {
        return duration(measure);
      }

      inline
      rational
      duration(paragraph::const_reference paragraph_element)
      {
        return boost::apply_visitor(get_duration(), paragraph_element);
      }
      inline
      rational
      duration(unfolded::staff::const_reference staff_element)
      {
        return boost::apply_visitor(get_duration(), staff_element);
      }
    }
  }
}

namespace boost {
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::paragraph::const_reference paragraph_element
            )
  {
    return r + duration(paragraph_element);
  }
  template <typename IntType>
  inline
  rational<IntType>
  operator+ ( rational<IntType> const& r
            , ::bmc::braille::ast::unfolded::staff::const_reference staff_element
            )
  {
    return r + duration(staff_element);
  }
}

namespace bmc {
  namespace braille {
    namespace ast {
      inline
      rational
      duration(paragraph const& paragraph)
      {
        return boost::accumulate(paragraph, rational());
      }
      inline
      rational
      duration(unfolded::staff const& staff)
      {
        return boost::accumulate(staff, rational());
      }
    }
  }
}

#endif
