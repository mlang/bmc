// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_CONFIG_HPP
#define BMC_CONFIG_HPP

#include <ciso646>

#define BOOST_CONTROL_RATIONAL_HAS_GCD 0
#define BOOST_SPIRIT_ACTIONS_ALLOW_ATTR_COMPAT
#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS

#if defined(_MSC_VER) && _MSC_VER <= 1900
#define BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES
#endif


#endif
