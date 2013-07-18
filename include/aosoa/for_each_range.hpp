/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_FOR_EACH_RANGE
#define AOSOA_FOR_EACH_RANGE

#include <cstddef>
#include <type_traits>

#include "soa/table_traits.hpp"

#include "aosoa/table_iterator.hpp"

namespace aosoa {

  namespace {
	template<bool is_compatibly_tabled, class C, class... CN> class _for_each_range;

	template<class C, class... CN>
	class _for_each_range<true, C, CN...> {
	public:
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;
		for (size_t i=0; i<sdb; ++i)
		  f(0, traits::table_size, first.data()[i], rest.data()[i]...);
		if (smb)
		  f(0, smb, first.data()[sdb], rest.data()[sdb]...);
	  }
	};

	template<class C, class... CN>
	class _for_each_range<false, C, CN...> {
	public:
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		f(0, first.end()-first.begin(), first.begin(), rest.begin()...);
	  }
	};
  }

  template<typename F, class C, class... CN>
  inline void for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_for_each_range<soa::is_compatibly_tabled<C, CN...>::value, C, CN...>::
	  loop(f, first, rest...);
  }

  namespace {
	template<bool is_compatibly_tabled, typename T, typename... TN> class _for_each_range_it;

	template<typename T, typename... TN>
	class _for_each_range_it<true, T, TN...> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		typedef table_iterator_traits<T> traits;
		const auto table0 = begin.table;
		const auto index0 = begin.index;
		const auto tablen = end.table;
		const auto indexn = end.index;

		if (table0 < tablen) {
		  f(index0, traits::table_size, table0[0], others.table[0]...);
		  const auto range = tablen-table0;
		  for (ptrdiff_t i=1; i<range; ++i)
			f(0, traits::table_size, table0[i], others.table[i]...);
		  f(0, indexn, table0[range], others.table[range]...);
		} else if (table0 == tablen) {
		  f(index0, indexn, table0[0], others.table[0]...);
		}
	  }
	};

	template<typename T, typename... TN>
	class _for_each_range_it<false, T, TN...> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		f(0, end-begin, begin, others...);
	  }
	};
  }

  template<typename T, typename F, typename... TN>
  inline void for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_for_each_range_it<is_compatibly_tabled_iterator<T, TN...>::value, T, TN...>::
	  loop(begin, end, f, others...);
  }
}

#endif
