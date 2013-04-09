/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE
#define AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE

#include <cstddef>

#include <algorithm>

#include "soa/table.hpp"
#include "aosoa/table_iterator.hpp"
#include "aosoa/table_range.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task_group.h"

namespace aosoa {

  namespace {
	template<class C, typename Enable = void> class _parallel_indexed_for_each_range;

	template<class C>
	class _parallel_indexed_for_each_range<C, typename std::enable_if<soa::table_traits<C>::tabled>::type> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  template<typename F>
	  static inline void loop(C& container, const F& f) {
		const auto size = container.size();
		auto data = container.data();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		const tbb::blocked_range<size_t> range(0, sdb);

		auto const fun = [&f, data](const tbb::blocked_range<size_t>& r) {
		  for (size_t i=r.begin(); i<r.end(); ++i)
			f(data[i], 0, traits::table_size, i*traits::table_size);
		};

		if (smb) {
		  tbb::task_group g;
		  g.run([&f,data,sdb,smb]{f(data[sdb], 0, smb, sdb*traits::table_size);});
		  tbb::parallel_for(range, fun);
		  g.wait();
		} else tbb::parallel_for(range, fun);
	  }
	};

	template<class C>
	class _parallel_indexed_for_each_range<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(C& container, const F& f) {
		const auto root = container.begin();
		tbb::parallel_for
		  (tbb::blocked_range<typename C::iterator>
		   (container.begin(), container.end()),
		   [&f, root](const tbb::blocked_range<typename C::iterator>& r){
			f(r.begin(), 0, r.end()-r.begin(), r.begin()-root);
		  });
	  }
	};
  }

  template<class C, typename F>
  inline void parallel_indexed_for_each_range(C& container, const F& f)
  {_parallel_indexed_for_each_range<C>::loop(container, f);}

  namespace {
	template<typename T, typename Enable = void> class _parallel_indexed_for_each_range_it;

	template<typename T>
	class _parallel_indexed_for_each_range_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		const auto root = begin.table;
		tbb::parallel_for
		  (table_range<T>(begin, end),
		   [&f, root](const table_range<T>& r) {
			const auto table0 = r.begin().table;
			const auto index0 = r.begin().index;
			const auto tablen = r.end().table;
			const auto indexn = r.end().index;

			const auto delta = (table0-root)*traits::table_size-index0;

			if (table0 < tablen) {
			  f(table0[0], index0, traits::table_size, delta);
			  const auto range = tablen-table0;
			  for (ptrdiff_t i=1; i<range; ++i) f(table0[i], 0, traits::table_size, delta+i*traits::table_size);
			  f(tablen[0], 0, indexn, delta+range*traits::table_size);
			} else if (table0 == tablen) {
			  f(table0[0], index0, indexn, delta);
			}
		  });
	  }
	};

	template<typename T>
	class _parallel_indexed_for_each_range_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		const auto root = begin;
		tbb::parallel_for
		  (tbb::blocked_range<T>(begin, end),
		   [&f, root](const tbb::blocked_range<T>& r){
			f(r.begin(), 0, r.end()-r.begin(), r.begin()-root);
		  });
	  }
	};
  }

  template<typename T, typename F>
  inline void parallel_indexed_for_each_range(T begin, T end, const F& f)
  {_parallel_indexed_for_each_range_it<T>::loop(begin, end, f);}
}

#endif
