// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <cassert>
#include <ios>

namespace music {

enum output_format
{
  verbatim_source = 0,
  lilypond = 1
};

enum ios_base_iword_mask
{
  output_format_mask = 1
};

namespace detail {

  template<bool>
  struct xalloc_key_holder
  {
    static int value;
    static bool initialized;
  };

  template<bool b>  int xalloc_key_holder<b>::value = 0;
  template<bool b> bool xalloc_key_holder<b>::initialized = false;

  struct xalloc_key_initializer_t
  {
    xalloc_key_initializer_t()
    {
      if (!xalloc_key_holder<true>::initialized) {
        xalloc_key_holder<true>::value = std::ios_base::xalloc();
        xalloc_key_holder<true>::initialized = true;
      }
    }
  };

  namespace { xalloc_key_initializer_t xalloc_key_initializer; }

} // namespace detail

/// returns flags controlling output.
inline long get_flags(std::ios_base& ios, ios_base_iword_mask mask)
{ return ios.iword(detail::xalloc_key_holder<true>::value) & mask; }

/// Set new flags controlling output format.
inline void set_flags(std::ios_base& ios, long new_flags, ios_base_iword_mask mask)
{
  assert((~mask & new_flags) == 0);
  long& flags = ios.iword(detail::xalloc_key_holder<true>::value);
  flags = (flags & ~mask) | new_flags;
}

/// Returns output format.
inline output_format get_output_format(std::ios_base& ios)
{ return static_cast<output_format>(get_flags(ios, output_format_mask)); }

/// Set new flags controlling output format.
inline void set_output_format(std::ios_base& ios, output_format new_format)
{ set_flags(ios, new_format, output_format_mask); }

/// Set new flags for verbatim source output format.
inline std::ios_base& verbatim_source_output_format(std::ios_base& ios)
{
  set_output_format(ios, verbatim_source);
  return ios;
}

/// Set new flag for lilypond output format.
inline std::ios_base& lilypond_output_format(std::ios_base& ios)
{
  set_output_format(ios, lilypond);
  return ios;
}

}
