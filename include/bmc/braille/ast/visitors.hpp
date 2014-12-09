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

      struct get_augmentation_dots : boost::static_visitor<unsigned>
      {
        result_type operator()(rhythmic const& rhythm) const { return rhythm.get_dots(); }
        result_type operator()(barline const&) const { return 0; }
        result_type operator()(hand_sign const&) const { return 0; }
        result_type operator()(hyphen const&) const { return 0; }
        result_type operator()(clef const&) const { return 0; }
        result_type operator()(simile const&) const { return 0; }
        result_type operator()(tie const&) const { return 0; }
        result_type operator()(tuplet_start const&) const { return 0; }
        result_type operator()(value_distinction const&) const { return 0; }
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

      inline bool is_grace(note const &n)
      {
        return std::find_if(n.articulations.begin(), n.articulations.end(),
                            [](articulation a)
               {
                 return a == appoggiatura || a ==  short_appoggiatura;
               }) != n.articulations.end();
      }
      inline bool is_grace(ast::rest const &) { return false; }
      inline bool is_grace(ast::chord const &chord) { return is_grace(chord.base); }
      inline bool is_grace(ast::moving_note &chord) { return is_grace(chord.base); }

      struct is_simile : boost::static_visitor<bool>
      {
        template <typename T>
        result_type operator()(T const&) const
        { return std::is_same<simile, T>::value; }
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

      class is_tuplet_start : public boost::static_visitor<bool>
      {
        unsigned &number;
        bool &doubled;
        bool &simple_triplet;
      public:
        is_tuplet_start(unsigned &number, bool &doubled, bool &simple_triplet)
        : number{number}
        , doubled{doubled}
        , simple_triplet{simple_triplet}
        {}

        result_type operator() (tuplet_start const& tuplet) const
        {
          number = tuplet.number();
          doubled = tuplet.doubled();
          simple_triplet = tuplet.simple_triplet();
          return true;
        }

        template <class T>
        result_type operator()(T const &) const
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
