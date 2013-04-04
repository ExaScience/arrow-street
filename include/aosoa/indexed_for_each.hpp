/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_INDEXED_FOR_EACH
#define AOSOA_INDEXED_FOR_EACH

#include <cstddef>

#include <iostream>

#include "aosoa/apply.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa {

#define def_indexed_for_each(name, ...)					\
  template<class C, typename F>							\
  inline void name(C& container, const F& f) {			\
	const auto size = container.size();					\
	auto data = container.data();						\
														\
	typedef soa::table_traits<C> traits;				\
														\
	if (traits::tabled) {								\
	  const auto sdb = size/traits::table_size;			\
	  const auto smb = size%traits::table_size;			\
														\
	  for (size_t i=0; i<sdb; ++i) {					\
		auto&& t = traits::get_table(data, i);			\
		const auto offset = i*traits::table_size;		\
		__VA_ARGS__										\
		  for (size_t j=0; j<traits::table_size; ++j)	\
			apply(f, t, offset+j, j);					\
	  }													\
	  if (smb) {										\
		auto&& t = traits::get_table(data, sdb);		\
		const auto offset = sdb*traits::table_size;		\
		__VA_ARGS__										\
		  for (size_t j=0; j<smb; ++j)					\
			apply(f, t, offset+j, j);					\
	  }													\
	} else {											\
	  auto&& t = traits::get_table(data, 0);			\
	  __VA_ARGS__										\
		for (size_t i=0; i<size; ++i)					\
		  apply(f, t, i, i);							\
	}													\
  }

  def_indexed_for_each(indexed_for_each);

#ifdef __ICC
  def_indexed_for_each(vector_indexed_for_each, _Pragma("vector always"));
  def_indexed_for_each(ivdep_indexed_for_each, _Pragma("ivdep"));
  def_indexed_for_each(vector_ivdep_indexed_for_each, _Pragma("ivdep") _Pragma("vector always"));
  def_indexed_for_each(simd_indexed_for_each, _Pragma("simd"));
  def_indexed_for_each(novector_indexed_for_each, _Pragma("novector"));
#endif

#define def_indexed_for_each_iterator(name, ...)		\
  template<class C, size_t B, typename F>				\
  inline void name(const table_iterator<C,B>& begin,	\
				   const table_iterator<C,B>& end,		\
				   const F& f) {						\
	const auto table0 = begin.table;					\
	const auto index0 = begin.index;					\
	const auto tablen = end.table;						\
	const auto indexn = end.index;						\
														\
	if (table0 < tablen) {								\
	  __VA_ARGS__										\
		for (size_t j=index0; j<B; ++j) {				\
		  auto obj = table0[0][j]; f(j-index0, obj);	\
		}												\
	  const auto range = tablen-table0;					\
	  for (ptrdiff_t i=1; i<range; ++i) {				\
		const auto offset = i*B-index0;					\
		__VA_ARGS__										\
		  for (size_t j=0; j<B; ++j) {					\
			auto obj = table0[i][j]; f(offset+j, obj);	\
		  }												\
	  }													\
	  const auto offset = range*B-index0;				\
	  __VA_ARGS__										\
		for (size_t j=0; j<indexn; ++j) {				\
		  auto obj = tablen[0][j]; f(offset+j, obj);	\
		}												\
	} else if (table0 == tablen) {						\
	  __VA_ARGS__										\
		for (size_t j=index0; j<indexn; ++j) {			\
		  auto obj = table0[0][j]; f(j-index0, obj);	\
		}												\
	}													\
  }

  def_indexed_for_each_iterator(indexed_for_each);

#ifdef __ICC
  def_indexed_for_each_iterator(vector_indexed_for_each, _Pragma("vector always"));
  def_indexed_for_each_iterator(ivdep_indexed_for_each, _Pragma("ivdep"));
  def_indexed_for_each_iterator(vector_ivdep_indexed_for_each, _Pragma("ivdep") _Pragma("vector always"));
  def_indexed_for_each_iterator(simd_indexed_for_each, _Pragma("simd"));
  def_indexed_for_each_iterator(novector_indexed_for_each, _Pragma("novector"));
#endif

}

#endif
