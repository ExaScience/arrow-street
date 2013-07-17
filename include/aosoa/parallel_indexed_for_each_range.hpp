/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE
#define AOSOA_PARALLEL_INDEXED_FOR_EACH_RANGE

#include <cstddef>
#include <type_traits>

#include "soa/table_traits.hpp"

#include "aosoa/table_iterator.hpp"

#ifndef NOTBB
#include "aosoa/table_range.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task_group.h"
#endif

#ifdef __cilk
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

namespace aosoa {

  namespace {
	template<class C, typename Enable = void, class... CN> class _parallel_indexed_for_each_range;

	template<class C, class... CN>
	class _parallel_indexed_for_each_range
	<C, typename std::enable_if<soa::is_compatibly_tabled<C, CN...>::value>::type, CN...> {
	public:
#ifndef NOTBB
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		const tbb::blocked_range<size_t> range(0, sdb);

		auto const fun = [&f, &first, &rest...](const tbb::blocked_range<size_t>& r) {
		  for (size_t i=r.begin(); i<r.end(); ++i)
			f(0, traits::table_size, i*traits::table_size, first.data()[i], rest.data()[i]...);
		};

		if (smb) {
		  tbb::task_group g;
		  g.run([&f,&first,&rest...,sdb,smb]
				{f(0, smb, sdb*traits::table_size, first.data()[sdb], rest.data()[sdb]...);});
		  tbb::parallel_for(range, fun);
		  g.wait();
		} else tbb::parallel_for(range, fun);
	  }
#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		if (smb)
		  cilk_spawn f(0, smb, sdb*traits::table_size, first.data()[sdb], rest.data()[sdb]...);
		cilk_for (size_t i=0; i<sdb; ++i)
		  f(0, traits::table_size, i*traits::table_size, first.data()[i], rest.data()[i]...);
	  }
#endif

	};

	template<class C, class... CN>
	class _parallel_indexed_for_each_range
	<C, typename std::enable_if<!soa::is_compatibly_tabled<C, CN...>::value>::type, CN...> {
	public:
#ifndef NOTBB
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		tbb::parallel_for
		  (tbb::blocked_range<typename C::iterator>
		   (first.begin(), first.end()),
		   [&f, &first, &rest...](const tbb::blocked_range<typename C::iterator>& r){
			auto offset = r.begin() - first.begin();
			f(0, r.end()-r.begin(), offset, r.begin(), (rest.begin()+offset)...);
		  });
	  }
#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(const F& f, C& first, CN&... rest) {
		auto begin = first.begin();
		auto end = first.end();
		auto span = end-begin;
		auto grainsize = std::min(2048l, span / (8 * __cilkrts_get_nworkers()));
		if (grainsize == 0) grainsize = 1;
		cilk_for (auto it=begin; it<end; it+=grainsize) {
		  auto offset = it - first.begin();
		  f(0, std::min(grainsize, end-it), offset, it, (rest.begin()+offset)...);
		}
	  }
#endif
	};
  }

#ifndef NOTBB
  template<typename F, class C, class... CN>
  inline void parallel_indexed_for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_range<C, CN...>::loop(f, first, rest...);}
#endif

#ifdef __cilk
  template<typename F, class C, class... CN>
  inline void cilk_parallel_indexed_for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_range<C, CN...>::cilk_loop(f, first, rest...);}
#endif

  namespace {
	template<typename T, typename Enable = void, typename... TN> class _parallel_indexed_for_each_range_it;

	template<typename T, typename... TN>
	class _parallel_indexed_for_each_range_it
	<T, typename std::enable_if<is_compatibly_tabled_iterator<T, TN...>::value>::type, TN...> {
	public:
#ifndef NOTBB
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		tbb::parallel_for
		  (table_range<T>(begin, end),
		   [&f, begin, others...](const table_range<T>& r) {
			const auto table0 = r.begin().table;
			const auto index0 = r.begin().index;
			const auto tablen = r.end().table;
			const auto indexn = r.end().index;

			typedef table_iterator_traits<T> traits;
			const auto delta = (table0-begin.table)*traits::table_size-index0;

			if (table0 < tablen) {
			  f(index0, traits::table_size, delta, table0[0], others.table[0]...);
			  const auto range = tablen-table0;
			  for (ptrdiff_t i=1; i<range; ++i)
				f(0, traits::table_size, delta+i*traits::table_size, table0[i], others.table[i]...);
			  f(0, indexn, delta+range*traits::table_size, table0[range], others.table[range]...);
			} else if (table0 == tablen) {
			  f(index0, indexn, delta, table0[0], others.table[0]...);
			}
		  });
	  }
#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(T begin, T end, const F& f, TN... others) {
		typedef table_iterator_traits<T> traits;
		const auto table0 = begin.table;
		const auto index0 = begin.index;
		const auto tablen = end.table;
		const auto indexn = end.index;

		if (table0 < tablen) {
		  cilk_spawn f(index0, traits::table_size, -index0, table0[0], others.table[0]...);
		  const auto range = tablen-table0;
		  cilk_spawn f(0, indexn, range*traits::table_size-index0, table0[range], others.table[range]...);
		  cilk_for (ptrdiff_t i=1; i<range; ++i)
			f(0, traits::table_size, i*traits::table_size-index0, table0[i], others.table[i]...);
		} else if (table0 == tablen) {
		  f(index0, indexn, -index0, table0[0], others.table[0]...);
		}
	  }
#endif
	};

	template<typename T, typename... TN>
	class _parallel_indexed_for_each_range_it
	<T, typename std::enable_if<!is_compatibly_tabled_iterator<T, TN...>::value>::type, TN...> {
	public:
#ifndef NOTBB
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		tbb::parallel_for
		  (tbb::blocked_range<T>(begin, end),
		   [&f, begin, others...](const tbb::blocked_range<T>& r){
			auto offset = r.begin() - begin;
			f(0, r.end()-r.begin(), offset, r.begin(), (others+offset)...);
		  });
	  }
#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(T begin, T end, const F& f, TN... others) {
		auto span = end-begin;
		auto grainsize = std::min(2048l, span / (8 * __cilkrts_get_nworkers()));
		if (grainsize == 0) grainsize = 1;
		cilk_for (auto it=begin; it<end; it+=grainsize) {
		  auto offset = it - begin;
		  f(0, std::min(grainsize, end-it), offset, it, (others+offset)...);
		}
	  }
#endif
	};
  }

#ifndef NOTBB
  template<typename T, typename F, typename... TN>
  inline void parallel_indexed_for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_range_it<T, TN...>::loop(begin, end, f, others...);
  }
#endif

#ifdef __cilk
  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_indexed_for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_range_it<T, TN...>::cilk_loop(begin, end, f, others...);
  }
#endif
}

#endif
