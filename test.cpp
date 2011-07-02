#include "numbers.hpp"
#include <boost/spirit/include/qi.hpp>
#include "ttb.h"

#define BOOST_TEST_MODULE bmc_test
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE(time_signature_grammar_test) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"#ab(");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::time_signature_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute == music::time_signature(12, 8));
  BOOST_CHECK(attribute == music::rational(3, 2));
  BOOST_CHECK(attribute.numerator() == 12);
  destroyTextTable(textTable);
}

#include "brl.hpp"

BOOST_AUTO_TEST_CASE(brl_parser_test) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"#A");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  music::braille::brl_type brl;
  BOOST_CHECK(boost::spirit::qi::parse(begin, end, brl(3456) >> brl(1)));
  BOOST_CHECK(begin == end);
  destroyTextTable(textTable);
}

#include "measure.hpp"

class is_rest: public boost::static_visitor<bool> {
public:
  result_type operator()(music::braille::ambiguous::note const&) const {
    return false;
  }
  result_type operator()(music::braille::ambiguous::chord const&) const {
    return false;
  }
  result_type operator()(music::braille::ambiguous::rest const&) const {
    return true;
  }
  result_type operator()(music::braille::ambiguous::value_distinction const&) const {
    return false;
  }
  result_type operator()(music::braille::ambiguous::simile const&) const {
    return false;
  }
};

BOOST_AUTO_TEST_CASE(measure_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"vu.");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 2);
  BOOST_CHECK(apply_visitor(is_rest(), attribute[0][0][0][0]));
  BOOST_CHECK(apply_visitor(is_rest(), attribute[0][0][0][1]));
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(measure_test2) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"_r.2`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 2);
  BOOST_CHECK(attribute[0].size() == 1);
  destroyTextTable(textTable);
}

#include "value_proxy.hpp"

class value_proxy_collector : public boost::static_visitor<void> {
  std::vector<music::braille::value_proxy> proxies;
  music::braille::value_category category;
public:
  value_proxy_collector(music::braille::value_category category) : category(category) {}
  std::vector<music::braille::value_proxy> result() const {
    return proxies;
  }
  void operator()(music::braille::ambiguous::note& note) {
    proxies.push_back(music::braille::value_proxy(note, category));
  }
  void operator()(music::braille::ambiguous::rest& rest) {
    proxies.push_back(music::braille::value_proxy(rest, category));
  }
  void operator()(music::braille::ambiguous::chord& chord) {
    proxies.push_back(music::braille::value_proxy(chord, category));
  }
  void operator()(music::braille::ambiguous::value_distinction) {
  }
  void operator()(music::braille::ambiguous::simile&) {
  }
};

BOOST_AUTO_TEST_CASE(value_proxy_test) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"u46*");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 3);
  value_proxy_collector collector(music::braille::large);
  std::for_each(attribute[0][0][0].begin(), attribute[0][0][0].end(),
                boost::apply_visitor(collector));
  BOOST_CHECK(collector.result().size() == 3);
  BOOST_CHECK(collector.result()[0].as_rational() == music::rational(1, 2));
  BOOST_CHECK(collector.result()[1].as_rational() == music::rational(1, 4));
  BOOST_CHECK(collector.result()[2].as_rational() == music::rational(1, 4));
  music::braille::value_proxy_list candidates(attribute[0][0][0]);
  BOOST_CHECK(candidates.size() == attribute[0][0][0].size());
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(value_proxy_list_test) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L">2,u46*");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 4);
  music::braille::value_proxy_list candidates(attribute[0][0][0]);
  BOOST_CHECK(candidates.size() == attribute[0][0][0].size() - 1);
  BOOST_CHECK(candidates[0].size() == 1); 
  BOOST_CHECK(candidates[1].size() == 2);
  BOOST_CHECK(candidates[2].size() == 2);
  std::vector< std::vector<music::braille::value_proxy> >
  interpretations = music::braille::disambiguate(candidates.begin(), candidates.end(), music::rational(1));
  BOOST_CHECK(interpretations.size() == 4);
  music::braille::proxied_partial_measure
  partial_measure_interpretations(attribute[0][0], music::rational(1));
  BOOST_CHECK(partial_measure_interpretations.size() == 4);
  for (music::braille::proxied_partial_measure::const_iterator
       i = partial_measure_interpretations.begin();
       i != partial_measure_interpretations.end(); ++i)
  {
    std::cout << '[';
    for (std::vector< std::vector<music::braille::value_proxy> >::const_iterator
         j = i->begin(); j != i->end(); ++j)
    {
      std::cout << '[';
      for (std::vector<music::braille::value_proxy>::const_iterator
           k = j->begin(); k != j->end(); ++k)
      {
        std::cout << k->as_rational();
        if (k != j->end() - 1) std::cout << ' ';
      }      
      std::cout << ']';
    }
    std::cout << ']';
  }
  std::cout << std::endl;

  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(proxied_voice_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L">2,u46*");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 4);
  music::braille::proxied_voice
  voice_interpretations(attribute[0], music::rational(1));
  BOOST_CHECK(voice_interpretations.size() == 4);
  for (music::braille::proxied_voice::const_iterator
       i = voice_interpretations.begin(); i != voice_interpretations.end(); ++i)
  {
    std::cout << '[';
    for (std::vector< std::vector< std::vector<music::braille::value_proxy> > >::const_iterator
         j = i->begin(); j != i->end(); ++j)
    {
      std::cout << '[';
      for (std::vector< std::vector<music::braille::value_proxy> >::const_iterator
           k = j->begin(); k != j->end(); ++k)
      {
	std::cout << '[';
	for (std::vector<music::braille::value_proxy>::const_iterator
	     l = k->begin(); l != k->end(); ++l)
	{
	  std::cout << l->as_rational();
	  if (l != k->end() - 1) std::cout << ' ';
	}     
	std::cout << ']';
      }      
      std::cout << ']';
    }
    std::cout << ']';
  }
  std::cout << std::endl;
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(proxied_voice_test2) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 1);
  BOOST_CHECK(attribute[0].size() == 2);
  BOOST_CHECK(attribute[0][0].size() == 1);
  BOOST_CHECK(attribute[0][0][0].size() == 1);
  music::braille::proxied_voice
  voice_interpretations(attribute[0], music::rational(3, 4));
  BOOST_CHECK(voice_interpretations.size() == 4);
  for (music::braille::proxied_voice::const_iterator
       i = voice_interpretations.begin(); i != voice_interpretations.end(); ++i)
  {
    std::cout << '[';
    for (std::vector< std::vector< std::vector<music::braille::value_proxy> > >::const_iterator
         j = i->begin(); j != i->end(); ++j)
    {
      std::cout << '[';
      for (std::vector< std::vector<music::braille::value_proxy> >::const_iterator
           k = j->begin(); k != j->end(); ++k)
      {
	std::cout << '[';
	for (std::vector<music::braille::value_proxy>::const_iterator
	     l = k->begin(); l != k->end(); ++l)
	{
	  std::cout << l->as_rational();
	  if (l != k->end() - 1) std::cout << ' ';
	}     
	std::cout << ']';
      }      
      std::cout << ']';
    }
    std::cout << ']';
  }
  std::cout << std::endl;
  destroyTextTable(textTable);
}

BOOST_AUTO_TEST_CASE(proxied_measure_test1) {
  textTable = compileTextTable("Tables/de.ttb");
  std::wstring const input(L"_r72`v$k_t!,v!5");
  typedef std::wstring::const_iterator iterator_type;
  iterator_type begin(input.begin());
  iterator_type const end(input.end());
  typedef music::braille::measure_grammar<iterator_type> parser_type;
  parser_type parser;
  parser_type::start_type::attr_type attribute;
  BOOST_CHECK(parse(begin, end, parser, attribute));
  BOOST_CHECK(begin == end);
  BOOST_CHECK(attribute.size() == 2);
  BOOST_CHECK(attribute[0].size() == 1);
  BOOST_CHECK(attribute[1].size() == 2);
  BOOST_CHECK(attribute[1][0].size() == 1);
  BOOST_CHECK(attribute[1][0][0].size() == 1);
  music::braille::proxied_measure
  measure_interpretations(attribute, music::rational(3, 4));
  BOOST_CHECK(measure_interpretations.size() == 4);
  for (music::braille::proxied_measure::const_iterator
       i = measure_interpretations.begin(); i != measure_interpretations.end();
       ++i)
  {
    std::cout << '[';
    for (std::vector< std::vector< std::vector< std::vector<music::braille::value_proxy> > > >::const_iterator
         j = i->begin(); j != i->end(); ++j)
    {
      std::cout << '[';
      for (std::vector< std::vector< std::vector<music::braille::value_proxy> > >::const_iterator
           k = j->begin(); k != j->end(); ++k)
      {
	std::cout << '[';
	for (std::vector< std::vector<music::braille::value_proxy> >::const_iterator
	     l = k->begin(); l != k->end(); ++l)
	{
	  std::cout << '[';
	  for (std::vector<music::braille::value_proxy>::const_iterator
		 m = l->begin(); m != l->end(); ++m)
	    {
	      std::cout << m->as_rational();
	      if (m != l->end() - 1) std::cout << ' ';
	    }     
	  std::cout << ']';
	}     
	std::cout << ']';
      }
      std::cout << ']';
    }
    std::cout << ']' << std::endl;
  }

  destroyTextTable(textTable);
}
