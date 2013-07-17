/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_INDEXED_FOR_EACH_RANGE
#define AOSOA_INDEXED_FOR_EACH_RANGE

#include <cstddef>
#include <type_traits>

#include "soa/table_traits.hpp"

#include "aosoa/table_iterator.hpp"

namespace aosoa {

  namespace {
	template<class C, typename Enable = void, class... CN> class _indexed_for_each_range;

	template<class C, class... CN>
	class _indexed_for_each_range
	<C, typename std::enable_if<soa::is_compatibly_tabled<C, CN...>::value>::type, CN...> {
	public:
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;
		for (size_t i=0; i<sdb; ++i)
		  f(0, traits::table_size, i*traits::table_size, first.data()[i], rest.data()[i]...);
		if (smb)
		  f(0, smb, sdb*traits::table_size, first.data()[sdb], rest.data()[sdb]...);
	  }
	};

	template<class C, class... CN>
	class _indexed_for_each_range
	<C, typename std::enable_if<!soa::is_compatibly_tabled<C, CN...>::value>::type, CN...> {
	public:
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		f(0, first.end()-first.begin(), 0, first.begin(), rest.begin()...);
	  }
	};
  }

  template<typename F, class C, class... CN>
  inline void indexed_for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_indexed_for_each_range<C, CN...>::loop(f, first, rest...);}

  namespace {
	template<typename T, typename Enable = void, typename... TN> class _indexed_for_each_range_it;

	template<typename T, typename... TN>
	class _indexed_for_each_range_it
	<T, typename std::enable_if<is_compatibly_tabled_iterator<T, TN...>::value>::type, TN...> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		typedef table_iterator_traits<T> traits;
		const auto table0 = begin.table;
		const auto index0 = begin.index;
		const auto tablen = end.table;
		const auto indexn = end.index;

		if (table0 < tablen) {
		  f(index0, traits::table_size, -index0, table0[0], others.table[0]...);
		  const auto range = tablen-table0;
		  for (ptrdiff_t i=1; i<range; ++i)
			f(0, traits::table_size, i*traits::table_size-index0, table0[i], others.table[i]...);
		  f(0, indexn, range*traits::table_size-index0, table0[range], others.table[range]...);
		} else if (table0 == tablen) {
		  f(index0, indexn, -index0, table0[0], others.table[0]...);
		}
	  }
	};

	template<typename T, typename... TN>
	class _indexed_for_each_range_it
	<T, typename std::enable_if<!is_compatibly_tabled_iterator<T, TN...>::value>::type, TN...> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		f(0, end-begin, 0, begin, others...);
	  }
	};
  }

  template<typename F, typename T, typename... TN>
  inline void indexed_for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_indexed_for_each_range_it<T, TN...>::loop(begin, end, f, others...);}

}

#endif
