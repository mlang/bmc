// Copyright (C) 2014  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include "bmc/music.hpp"
#define BOOST_PYTHON_PY_SIGNATURES_PROPER_INIT_SELF_TYPE
#include <boost/python.hpp>

BOOST_PYTHON_MODULE(bmc) {
  using namespace boost::python;

  scope().attr("__doc__") = str("braille music compiler");
  scope().attr("__compiler__") = str(BOOST_COMPILER);

  class_<music::rational>("rational",
    init<music::rational>())
    .def(init<>())
    .def(init<music::rational::int_type>())
    .def(init<music::rational::int_type, music::rational::int_type>(args("numerator", "denominator")))
    .def("numerator", &music::rational::numerator)
    .def("denominator", &music::rational::denominator)
    .def(self + int()).def(int() + self).def(self + self)
    .def(self - int()).def(int() - self).def(self - self)
    .def(self * int()).def(int() * self).def(self * self)
    .def(self / int()).def(int() / self).def(self / self)
    .def(self += int()).def(self += self)
    .def(self -= int()).def(self -= self)
    .def(self *= int()).def(self *= self)
    .def(self /= int()).def(self /= self)
    .def(self > int()).def(self > self).def(self >= int()).def(self >= self)
    .def(self <= int()).def(self <= self).def(self < int()).def(self < self)
    .def("__float__", &boost::rational_cast<double, music::rational::int_type>)
    .def(repr(self))
    ;
  implicitly_convertible<music::rational::int_type, music::rational>();
  def("gcd", &boost::math::gcd<music::rational>);
  def("lcm", &boost::math::lcm<music::rational>);

  class_<music::time_signature, bases<music::rational> >("time_signature",
    init<music::time_signature>())
    .def(init<>())
    .def(init<music::rational::int_type, music::rational::int_type>(args("numerator", "denominator")))
    .def("numerator", &music::time_signature::numerator)
    .def("denominator", &music::time_signature::denominator)
    .def(repr(self))
    ;
}
