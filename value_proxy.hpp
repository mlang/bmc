namespace music { namespace braille {

enum value_category
{
  large, small
};

class value_proxy
{
  ambiguous::value value_type;
  value_category category;
  unsigned dots;
  rational* final_type;
public:
  value_proxy(ambiguous::note& note, value_category const& category)
  : value_type(note.ambiguous_value), category(category), dots(note.dots)
  , final_type(&note.type)
  {
    BOOST_ASSERT(*final_type == music::rational(0));
  }

  value_proxy(ambiguous::rest& rest, value_category const& category)
  : value_type(rest.ambiguous_value), category(category), dots(rest.dots)
  , final_type(&rest.type)
  {
    BOOST_ASSERT(*final_type == music::rational(0));
  }

  value_proxy(ambiguous::chord& chord, value_category const& category)
  : value_type(chord.base.ambiguous_value), category(category), dots(chord.base.dots)
  , final_type(&chord.base.type)
  {
    BOOST_ASSERT(*final_type == music::rational(0));
  }

  rational as_rational() const {
    rational base;
    switch (value_type) {
    case ambiguous::whole_or_16th:
      base = rational(1, category==large? 1: 16);
      break;
    case ambiguous::half_or_32th:
      base = rational(1, category==large? 2: 32);
      break;
    case ambiguous::quarter_or_64th:
      base = rational(1, category==large? 4: 64);
      break;
    case ambiguous::eighth_or_128th:
      base = rational(1, category==large? 8: 128);
      break;
    default:
      BOOST_ASSERT(false);
    }
    return base;
  }
};

}}

