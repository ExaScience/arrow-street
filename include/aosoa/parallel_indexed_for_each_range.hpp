/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE
#define AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE

#include <cstddef>

#include <algorithm>

#include "soa/table.hpp"
#include "aosoa/table_iterator.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task_group.h"

namespace aosoa {

  template<class C, typename F>
  inline void parallel_for_each_range(C& container, const F& f) {
	const auto size = container.size();
	auto data = container.data();

	typedef soa::table_traits<C> traits;

	if (traits::tabled) {

	  const auto sdb = size/traits::table_size;
	  const auto smb = size%traits::table_size;

	  const tbb::blocked_range<size_t>(0, sdb);

	  auto const fun = [&f, data](const tbb::blocked_range<size_t>& r) {
		for (size_t i=r.begin(); i<r.end(); ++i)
		  f(traits::get_table(data, i), 0, traits::table_size, i*traits::table_size);
	  };

	  if (smb) {
		tbb::task_group g;
		g.run([&f,data,sdb,smb]{f(traits::get_table(data, sdb), 0, smb, sdb*traits::table_size);});
		tbb::parallel_for(range, fun);
		g.wait();
	  } else tbb::parallel_for(range, fun);

	} else tbb::parallel_for
			 (tbb::blocked_range<size_t>(0,size),
			  [&f, data](const tbb::blocked_range<size_t>& r){
			   f(traits::get_table(data, 0), r.begin(), r.end(), 0);
			 });
  }

  template<class C, size_t B, typename F>
  inline void parallel_for_each_range(const table_iterator<C,B>& begin,
									  const table_iterator<C,B>& end,
									  const F& f) {
	const auto root = begin.table;
	tbb::parallel_for
	  (table_range<table_iterator<C,B>>(begin, end),
	   [root, &f](const table_range<table_iterator<C,B>>& r) {
		const auto table0 = r.begin().table;
		const auto index0 = r.begin().index;
		const auto tablen = r.end().table;
		const auto indexn = r.end().index;

		const auto delta = (table0-root)*B-index0;

		if (table0 < tablen) {
		  f(table0[0], index0, B, delta);
		  const auto range = tablen-table0;
		  for (ptrdiff_t i=1; i<range; ++i) f(table0[i], 0, B, delta+i*B);
		  f(tablen[0], 0, indexn, delta+range*B);
		} else if (table0 == tablen) {
		  f(table0[0], index0, indexn, delta);
		}
	  });
  }

}

#endif
