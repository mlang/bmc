#include "config.hpp"
#include "music.hpp"
#include <boost/spirit/include/qi_grammar.hpp>

namespace music { namespace braille {

template <typename Iterator>
struct upper_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  upper_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

template <typename Iterator>
struct lower_number_grammar : boost::spirit::qi::grammar<Iterator, unsigned()>
{
  lower_number_grammar();

  boost::spirit::qi::rule<Iterator, unsigned()> start;
};

template <typename Iterator>
struct time_signature_grammar : boost::spirit::qi::grammar<Iterator, time_signature(), boost::spirit::qi::locals<unsigned, unsigned> >
{
  time_signature_grammar();

  boost::spirit::qi::rule< Iterator
                         , time_signature()
                         , boost::spirit::qi::locals<unsigned, unsigned>
                         > start;
  upper_number_grammar<Iterator> upper_number;
  lower_number_grammar<Iterator> lower_number;
};

}}
