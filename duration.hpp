#ifndef BMC_DURATION_HPP
#define BMC_DURATION_HPP

#include <boost/range/numeric.hpp>

namespace music {

struct duration_visitor : boost::static_visitor<rational>
{
  result_type operator()(braille::ambiguous::note const& r) const
  { return r.as_rational(); }
  result_type operator()(braille::ambiguous::rest const& r) const
  { return r.as_rational(); }
  result_type operator()(braille::ambiguous::chord const& r) const
  { return r.as_rational(); }
  template<typename T> result_type operator()(T const&) const
  { return zero; }
};

inline rational
duration(braille::ambiguous::sign const& sign)
{ return apply_visitor(music::duration_visitor(), sign); }

}

namespace boost {
  template<typename IntType>
  inline rational<IntType>
  operator+( rational<IntType> const& r
           , music::braille::ambiguous::sign const& sign)
  { return r + music::duration(sign); }
}

namespace music {

inline rational
duration(braille::ambiguous::partial_voice const& partial_voice)
{ return boost::accumulate(partial_voice, zero); }

inline rational
duration(braille::ambiguous::partial_measure const& partial_measure)
{ return duration(partial_measure.front()); }

}

namespace boost {
  template<typename IntType>
  inline rational<IntType>
  operator+( rational<IntType> const& r
           , music::braille::ambiguous::voice::const_reference p)
  { return r + music::duration(p); }
}

namespace music {

inline rational
duration(braille::ambiguous::voice const& voice)
{ return boost::accumulate(voice, zero); }

inline rational
duration(braille::ambiguous::measure const& measure)
{ return duration(measure.voices.front()); }

}

#endif
