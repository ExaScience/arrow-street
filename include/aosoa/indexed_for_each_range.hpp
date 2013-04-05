/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_INDEXED_FOR_EACH_RANGE
#define AOSOA_INDEXED_FOR_EACH_RANGE

#include <cstddef>

#include "soa/table.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa {

  namespace {
	template<class C, typename Enable = void> class _indexed_for_each_range;

	template<class C>
	class _indexed_for_each_range<C, typename std::enable_if<soa::table_traits<C>::tabled>::type> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  template<typename F>
	  static inline void loop(C& container, const F& f) {
		const auto size = container.size();
		auto data = container.data();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;
		for (size_t i=0; i<sdb; ++i) f(data[i], 0, traits::table_size, i*traits::table_size);
		if (smb) f(data[sdb], 0, smb, sdb*traits::table_size);
	  }
	};

	template<class C>
	class _indexed_for_each_range<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(C& container, const F& f) {
		f(container.begin(), 0, container.end()-container.begin(), 0);
	  }
	};
  }

  template<class C, typename F>
  inline void indexed_for_each_range(C& container, const F& f)
  {_indexed_for_each_range<C>::loop(container, f);}

  namespace {
	template<typename T, typename Enable = void> class _indexed_for_each_range_it;

	template<typename T>
	class _indexed_for_each_range_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		const auto table0 = begin.table;
		const auto index0 = begin.index;
		const auto tablen = end.table;
		const auto indexn = end.index;

		if (table0 < tablen) {
		  f(table0[0], index0, traits::table_size, -index0);
		  const auto range = tablen-table0;
		  for (ptrdiff_t i=1; i<range; ++i) f(table0[i], 0, traits::table_size, i*traits::table_size-index0);
		  f(tablen[0], 0, indexn, range*traits::table_size-index0);
		} else if (table0 == tablen) {
		  f(table0[0], index0, indexn, -index0);
		}
	  }
	};

	template<typename T>
	class _indexed_for_each_range_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		f(begin, 0, end-begin, 0);
	  }
	};
  }

  template<typename T, typename F>
  inline void indexed_for_each_range(T begin, T end, const F& f)
  {_indexed_for_each_range_it<T>::loop(begin, end, f);}

}

#endif
