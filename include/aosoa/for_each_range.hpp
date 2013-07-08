/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_FOR_EACH_RANGE
#define AOSOA_FOR_EACH_RANGE

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "soa/table_traits.hpp"
#include "aosoa/apply_tuple.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa {

  namespace {
	template<class C0, typename Enable = void> class _for_each_range;

	template<class C0>
	class _for_each_range<C0, typename std::enable_if<soa::table_traits<C0>::tabled>::type> {
	private:
	  typedef soa::table_traits<C0> traits;
	public:
	  template<typename F, class... C>
	  static inline void loop(const F& f, C0& container, C&... other_containers) {
		const auto size = container.size();
		//		auto data = std::make_tuple(container.data(), other_containers.data()...);
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;
		for (size_t i=0; i<sdb; ++i)
		  apply_tuple(f, std::tuple_cat(std::make_tuple(0, traits::table_size), apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(i), std::make_tuple(container.data(), other_containers.data()...))));
		//		for (size_t i=0; i<sdb; ++i) f(data[i], 0, traits::table_size);
		apply_tuple(f, std::tuple_cat(std::make_tuple(0, smb), apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(sdb), std::make_tuple(container.data(), other_containers.data()...))));
		//		if (smb) f(data[sdb], 0, smb);
	  }
	};

	template<class C>
	class _for_each_range<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(const F& f, C& container) {
		f(0, container.end()-container.begin(), container.begin());
	  }
	};
  }

  template<typename F, class C0, class... C>
  inline void for_each_range(const F& f, C0& container, C&... other_containers)
  {_for_each_range<C0>::loop(f, container, other_containers...);}

  namespace {
	template<typename T, typename Enable = void> class _for_each_range_it;

	template<typename T>
	class _for_each_range_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
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
		  f(table0[0], index0, traits::table_size);
		  const auto range = tablen-table0;
		  for (ptrdiff_t i=1; i<range; ++i) f(table0[i], 0, traits::table_size);
		  f(tablen[0], 0, indexn);
		} else if (table0 == tablen) {
		  f(table0[0], index0, indexn);
		}
	  }
	};

	template<typename T>
	class _for_each_range_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		f(begin, 0, end-begin);
	  }
	};
  }

  template<typename T, typename F>
  inline void for_each_range(T begin, T end, const F& f)
  {_for_each_range_it<T>::loop(begin, end, f);}
}

#endif
