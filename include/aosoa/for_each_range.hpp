/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_FOR_EACH_RANGE
#define AOSOA_FOR_EACH_RANGE

#include <cstddef>

#include "soa/table.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa {

  template<class C, typename F>
  inline void for_each_range(C& container, const F& f) {
	const auto size = container.size();
	auto data = container.data();

	typedef soa::table_traits<C> traits;

	if (traits::tabled) {
	  const auto sdb = size/traits::table_size;
	  const auto smb = size%traits::table_size;
	  for (size_t i=0; i<sdb; ++i) f(traits::get_table(data, i), 0, traits::table_size);
	  if (smb) f(traits::get_table(data, sdb), 0, smb);
	} else f(traits::get_table(data, 0), 0, size);
  }

  template<class C, size_t B, typename F>
  inline void for_each_range(const table_iterator<C,B>& begin,
							 const table_iterator<C,B>& end,
							 const F& f) {
	const auto table0 = begin.table;
	const auto index0 = begin.index;
	const auto tablen = end.table;
	const auto indexn = end.index;

	if (table0 < tablen) {
	  f(table0[0], index0, B);
	  const auto range = tablen-table0;
	  for (ptrdiff_t i=1; i<range; ++i) f(table0[i], 0, B);
	  f(tablen[0], 0, indexn);
	} else if (table0 == tablen) {
	  f(table0[0], index0, indexn);
	}
  }

}

#endif
