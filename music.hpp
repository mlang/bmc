#ifndef MUSIC_HPP
#define MUSIC_HPP

#include <boost/rational.hpp>

namespace music {

typedef boost::rational<int> rational;

inline rational::int_type floor(rational const& r)
{ return boost::rational_cast<rational::int_type>(r); }

rational gcd(rational const& a, rational const& b)
{ return b == rational(0)?  a: gcd(b, a - b * floor(a / b)); }

typedef rational time_modification;

// arithmetic with time_signatures is useful, so derive from rational
class time_signature: public rational {
  // boost::rational simplifies during construction, so we need to
  // preserve the original numerator and denominator
  int_type d_num, d_den;
  rational d_anacrusis;
public:
  time_signature(int_type n = 4, int_type d = 4)
  : rational(n, d), d_num(n), d_den(d) {}
  using rational::operator==; bool operator==(time_signature const&) const;
  int_type numerator() const { return d_num; }
  int_type denominator() const { return d_den; }
  // anacrusis
  rational const& anacrusis() const { return d_anacrusis; }
  void anacrusis(rational const& anacrusis) { d_anacrusis = anacrusis; }
};

enum diatonic_step { C = 0, D, E, F, G, A, B };
enum accidental {
  natural, flat, double_flat, triple_flat,
           sharp, double_sharp, triple_sharp
};

typedef signed short key_signature;

enum ornament { appoggiatura };

enum interval { unison = 0, second = 1, third = 2, fourth = 3, fifth = 4, sixth = 5, seventh = 6, octave = 7 };

}

#endif
