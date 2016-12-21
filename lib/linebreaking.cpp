#include <bmc/braille/linebreaking.hpp>

#include <algorithm>
#include <iostream>
#include <list>

// A fixed-width linebreaking implementation based on the box/glue/penalty
// algebra from Donald E. Knuth and Michael F. Plass.
//
// This implementation deliberately skips the shrink and stretch factors,
// because we are dealing with fixed-width text.
// It uses the same underlying dynamic programming algorithm detailed in the
// original Knuth and Plass paper, calculating demerits in a different way.
// We are trying to minimize slack and use penalties to score certain newline
// oppotunities in different ways.  For instance, a newline after a voice or
// partial measure separator is prefered above a typical music hyphen.
// Currently, breaking inside a notegroup is supressed completely.
// A future implementation could revert to normal note/rest values inside a
// broken notegroup, while penalizing a break inside a notegroup failry highly.

namespace bmc { namespace braille {

namespace {

struct breakpoint {
  linebreaking::objects::const_iterator position;
  int demerits;
  unsigned line;
  unsigned total_width;
  std::shared_ptr<breakpoint> previous;

  breakpoint(linebreaking::objects::const_iterator position,
	     int demerits, unsigned line, unsigned total_width,
	     std::shared_ptr<breakpoint> previous = std::shared_ptr<breakpoint>{})
  : position{position}, demerits{demerits}, line{line}, total_width{total_width}
  , previous{std::move(previous)} {}
};

int compute_slack(linebreaking::objects::const_iterator begin,
                  linebreaking::objects::const_iterator end,
                  breakpoint const &active, unsigned current_line,
                  unsigned const &sum_width,
                  std::vector<unsigned> const &line_lengths)
{
  int width = sum_width - active.total_width;

  if (auto p = dynamic_cast<linebreaking::penalty const *>(end->get())) {
    width += p->width();
    if (p->width()) {
      if (auto b = dynamic_cast<linebreaking::box const *>(std::prev(end)->get())) {
        if (b->is_guide()) width -= b->width();
      }
    }
  }

  int const line_length = line_lengths[std::min(std::size_t(current_line),
						line_lengths.size()) - 1];

  return line_length - width;
}

bool is_forced_break(linebreaking::objects::const_iterator const &i)
{
  if (auto p = dynamic_cast<linebreaking::penalty const *>(i->get())) {
    return p->value() == -linebreaking::infinity;
  }

  return false;
}

unsigned
compute_sum(linebreaking::objects::const_iterator current,
            linebreaking::objects::const_iterator end)
{
  unsigned width = 0;

  for (auto i = current; i != end; ++i) {
    if (auto g = dynamic_cast<linebreaking::glue const *>(i->get())) {
      width += g->width();
    } else if (dynamic_cast<linebreaking::box const *>(i->get()) ||
	       (i > current && is_forced_break(i))) {
      break;
    }
  }

  return width;
}

void main_loop(linebreaking::objects::const_iterator index,
		 linebreaking::objects const &objs,
		 std::list<std::shared_ptr<breakpoint>> &active_nodes,
		 unsigned const &sum_width,
		 std::vector<unsigned> const &line_lengths)
{
  auto i = active_nodes.begin(), e = active_nodes.end();
  while (i != e) {
    std::shared_ptr<breakpoint> candidate;
    int candidate_demerits = linebreaking::infinity;
    while (i != e) {
      unsigned current_line = (*i)->line + 1;
      auto slack = compute_slack((*i)->position, index, **i, current_line,
                                  sum_width, line_lengths);

      if (slack < 0) {
	active_nodes.erase(i++);
      } else {
	int demerits = slack * slack;
	if (auto p = dynamic_cast<linebreaking::penalty const *>(index->get())) {
	  if (p->value() != -linebreaking::infinity)
            demerits += p->value() * p->value();
	  if (auto pp = dynamic_cast<linebreaking::penalty const *>((*i)->position->get())) {
	    if (p->width() && pp->width())
	      demerits += 10;
	  }
	}

	demerits += (*i)->demerits;
	
	if (demerits < candidate_demerits) {
	  candidate = *i;
          candidate_demerits = demerits;
        }

	++i;

	if (i != e && (*i)->line >= current_line) break;
      }
    }

    if (candidate) {
      active_nodes.insert(i, std::make_shared<breakpoint>
			     (index, candidate_demerits, candidate->line + 1,
			      sum_width + compute_sum(index, objs.end()),
			      candidate
			     )
                         );
    }
  }
}

}

std::deque<linebreaking::objects::const_iterator>
linebreaking::breakpoints(objects const &objs, std::vector<unsigned> const &line_lengths)
{
  unsigned sum_width = 0;
  std::list<std::shared_ptr<breakpoint>> active_nodes {
    std::make_shared<breakpoint>(objs.begin(), 0, 0, 0, nullptr)
  };

  for (auto begin = objs.begin(), i = begin, e = objs.end(); i != e; ++i) {
    if (auto b = dynamic_cast<box const *>(i->get())) {
      sum_width += b->width();
    } else if (auto g = dynamic_cast<glue const *>(i->get())) {
      if (i != begin && dynamic_cast<box const *>(std::prev(i)->get())) {
	main_loop(i, objs, active_nodes, sum_width, line_lengths);
      }
      sum_width += g->width();
    } else if (auto p = dynamic_cast<penalty const *>(i->get())) {
      if (p->value() != infinity) {
	main_loop(i, objs, active_nodes, sum_width, line_lengths);
      }
    }
  }

  std::deque<objects::const_iterator> breaks;
  if (!active_nodes.empty()) {
    for (auto ptr = *std::min_element(active_nodes.begin(), active_nodes.end(),
				      [](std::shared_ptr<breakpoint> const &lhs,
					 std::shared_ptr<breakpoint> const &rhs)
				      {
					return lhs->demerits < rhs->demerits;
				      }); ptr; ptr = ptr->previous)
      breaks.push_front(ptr->position);
  }
  return breaks;
}

}}
