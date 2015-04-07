// Copyright (C) 2011  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#if !defined(BRL_HPP)
#define BRL_HPP

#include <boost/spirit/include/qi_parse.hpp>
#include <bmc/braille/text2braille.hpp>

// definition the place holder
namespace bmc { namespace braille { BOOST_SPIRIT_TERMINAL_EX(brl) }}

// implementation the enabler
namespace boost { namespace spirit
{
  template <typename A0>
  struct use_terminal< qi::domain
                     , terminal_ex< ::bmc::braille::tag::brl
                                  , fusion::vector1<A0>
                                  >
                     > : mpl::or_<is_integral<A0>, is_enum<A0> > {};

  template <>
  struct use_lazy_terminal<qi::domain, ::bmc::braille::tag::brl, 1>
    : mpl::true_ {};
}}

// implementation of the parser
namespace bmc { namespace braille {
  template <typename Int>
  struct brl_parser : boost::spirit::qi::primitive_parser< brl_parser<Int> > {
    template <typename Context, typename Iterator>
    struct attribute { typedef boost::spirit::unused_type type; };

    brl_parser(Int dots) : dots(from_decimal(dots)) {}

    template< typename Iterator
            , typename Context, typename Skipper, typename Attribute
            >
    bool parse( Iterator& first, Iterator const& last
              , Context&, Skipper& skipper, Attribute&
              ) const
    {
      boost::spirit::qi::skip_over(first, last, skipper);
      if (first == last) return false;
      if (*first < 0X20) return false;
      unsigned char
      d = get_dots_for_character(*first) & 0X3F;
      if (d == dots) {
        ++first;
        return true;
      }
      return false;
    }

    template<typename Context>
    boost::spirit::info what(Context&) const
    { return boost::spirit::info("brl"); }

  private:
    Int const dots;

    static Int from_decimal(Int dots, Int bits = 0)
    {
      return dots > 0?
             from_decimal(dots / 10, bits | Int(1) << ((dots % 10) - 1)):
             bits;
    }
  };
}}

// instantiation of the parser
namespace boost { namespace spirit { namespace qi
{
  template<typename Modifiers, typename A0>
  struct make_primitive< terminal_ex< ::bmc::braille::tag::brl
                                    , fusion::vector1<A0>
                                    >
                       , Modifiers
                       >
  {
    typedef ::bmc::braille::brl_parser<A0> result_type;

    template<typename Terminal>
    result_type operator()(Terminal const& term, unused_type) const {
      return result_type(fusion::at_c<0>(term.args));
    }
  };
}}}

#endif
