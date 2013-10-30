// Copyright (C) 2012, 2013  Mario Lang <mlang@delysid.org>
//
// Distributed under the terms of the GNU General Public License version 3.
// (see accompanying file LICENSE.txt or copy at
//  http://www.gnu.org/licenses/gpl-3.0-standalone.html)

#ifndef BMC_AST_VISITORS_HPP
#define BMC_AST_VISITORS_HPP

#include "bmc/braille/ast/ast.hpp"
#include <boost/variant/static_visitor.hpp>

namespace music {
  namespace braille {
    namespace ast {

      /// Visitors

      struct get_line : boost::static_visitor<int>
      {
        result_type operator()(locatable const& lexeme) const { return lexeme.line; }
        result_type operator()(barline const&) const { return 0; }
        result_type operator()(hand_sign const&) const { return 0; }
      };

      struct get_column : boost::static_visitor<int>
      {
        result_type operator()(locatable const& lexeme) const { return lexeme.column; }
        result_type operator()(barline const&) const { return 0; }
        result_type operator()(hand_sign const&) const { return 0; }
      };

      struct get_ambiguous_value : boost::static_visitor<value>
      {
        result_type operator()(note const& note) const
        { return note.ambiguous_value; }
        result_type operator()(rest const& rest) const
        { return rest.ambiguous_value; }
        result_type operator()(chord const& chord) const
        { return (*this)(chord.base); }
        result_type operator()(moving_note const& chord) const
        { return (*this)(chord.base); }
        template<typename T>
        result_type operator()(T const&) const
        { return unknown; }
      };

      struct is_rest : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_same<rest, T>::value; }
      };

      struct is_rhythmic : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_base_of<rhythmic, T>::value; }
      };

      class is_value_distinction : public boost::static_visitor<bool>
      {
        bool check;
        value_distinction::type expected;
      public:
        is_value_distinction()
        : check(false) {}

        is_value_distinction(value_distinction::type distinction)
        : check(true)
        , expected(distinction) {}

        result_type operator() (value_distinction const& distinction) const
        { return not check? true: distinction.value == expected; }

        template <class Sign>
        result_type operator()(Sign const &) const
        { return false; }
      };
      struct is_hyphen : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_same<hyphen, T>::value; }
      };
    }
  }
}

#endif
