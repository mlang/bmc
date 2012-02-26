// Copyright (C) 2012  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#include <cassert>
#include <ios>

namespace music {

enum class output_format: long
{
  identity = 0,
  lilypond = 1
};

enum class ios_base_iword_mask: long
{
  output_format = 1
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

/// Get flags controlling output.
inline long get_flags(std::ios_base& ios, ios_base_iword_mask mask)
{ return ios.iword(detail::xalloc_key_holder<true>::value) & long(mask); }

/// Set new flags controlling output.
inline void set_flags(std::ios_base& ios, long new_flags, ios_base_iword_mask mask)
{
  assert((~long(mask) & new_flags) == 0);
  long& flags = ios.iword(detail::xalloc_key_holder<true>::value);
  flags = (flags & ~long(mask)) | new_flags;
}

/// Returns output format.
inline output_format get_output_format(std::ios_base& ios)
{ return output_format(get_flags(ios, ios_base_iword_mask::output_format)); }

/// Set new output format.
inline void set_output_format(std::ios_base& ios, output_format format)
{ set_flags(ios, long(format), ios_base_iword_mask::output_format); }

/// Set verbatim source output format.
inline std::ios_base& identity_output_format(std::ios_base& ios)
{
  set_output_format(ios, output_format::identity);
  return ios;
}

/// Set lilypond output format.
inline std::ios_base& lilypond_output_format(std::ios_base& ios)
{
  set_output_format(ios, output_format::lilypond);
  return ios;
}

}
