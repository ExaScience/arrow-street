/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_TABLE_RANGE
#define AOSOA_TABLE_RANGE

#include "tbb/tbb_stddef.h"

namespace aosoa {

  template<class Iterator>
  class table_range {
  private:
	Iterator lower, upper;

	typedef table_iterator_traits<Iterator> traits;

  public:
	table_range () {}

	table_range (Iterator lower, Iterator upper) : lower(lower), upper(upper) {}

	bool empty() const {return lower == upper;}

	bool is_divisible() const {return lower.table < upper.table;}

	table_range (table_range& that, tbb::split) {
	  auto mid = (that.upper.table - that.lower.table) / 2;
	  lower.table = that.lower.table + mid + 1;
	  lower.index = 0;
	  upper = that.upper;
	  that.upper.table = that.lower.table + mid;
	  that.upper.index = traits::table_size;
	}

	Iterator begin() {return lower;}
	const Iterator begin() const {return lower;}
	Iterator end() {return upper;}
	const Iterator end() const {return upper;}
  };
}

#endif
