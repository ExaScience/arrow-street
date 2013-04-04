/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH
#define AOSOA_PARALLEL_INDEXED_FOR_EACH

#include <cstddef>

#include <algorithm>

#include "aosoa/apply.hpp"
#include "aosoa/table_iterator.hpp"
#include "aosoa/table_range.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"

namespace aosoa {

#define def_parallel_indexed_for_each(name, ...)					\
  template<class C, typename F>										\
  inline void name(C& container, const F& f) {						\
	const auto size = container.size();								\
	auto data = container.data();									\
																	\
	typedef soa::table_traits<C> traits;							\
																	\
	if (traits::tabled) {											\
	  const auto sdb = size/traits::table_size;						\
	  const auto smb = size%traits::table_size;						\
																	\
	  tbb::parallel_for												\
		(tbb::blocked_range<size_t>(0,sdb+(smb?1:0)),				\
		 [&f,data,sdb,smb](const tbb::blocked_range<size_t>& r){	\
		  for (size_t i=r.begin(); i<std::min(r.end(),sdb); ++i) {	\
			auto&& t = traits::get_table(data, i);					\
			const auto offset = i*traits::table_size;				\
			__VA_ARGS__												\
			  for (size_t j=0; j<traits::table_size; ++j)			\
				apply(f, t, offset+j, j);							\
		  }															\
		  if (r.end() == sdb) {										\
			auto&& t = traits::get_table(data, sdb);				\
			const auto offset = sdb*traits::table_size;				\
			__VA_ARGS__												\
			  for (size_t j=0; j<smb; ++j)							\
				apply(f, t, offset+j, j);							\
		  }															\
		});															\
	} else {														\
	  tbb::parallel_for												\
		(tbb::blocked_range<size_t>(0,size),						\
		 [&f,data](const tbb::blocked_range<size_t> r){				\
		  auto&& t = traits::get_table(data, 0);					\
		  __VA_ARGS__												\
			for (size_t i=r.begin(); i<r.end(); ++i)				\
			  apply(f, t, i, i);									\
		});															\
	}																\
  }

  def_parallel_indexed_for_each(parallel_indexed_for_each);

#ifdef __ICC
  def_parallel_indexed_for_each(parallel_vector_indexed_for_each, _Pragma("vector always"));
  def_parallel_indexed_for_each(parallel_ivdep_indexed_for_each, _Pragma("ivdep"));
  def_parallel_indexed_for_each(parallel_vector_ivdep_indexed_for_each, _Pragma("ivdep") _Pragma("vector always"));
  def_parallel_indexed_for_each(parallel_simd_indexed_for_each, _Pragma("simd"));
  def_parallel_indexed_for_each(parallel_novector_indexed_for_each, _Pragma("novector"));
#endif

#define def_parallel_indexed_for_each_iterator(name, ...)		\
  template<class C, size_t B, typename F>						\
  inline void name(const table_iterator<C,B>& begin,			\
				   const table_iterator<C,B>& end,				\
				   const F& f) {								\
	const auto root = begin.table;								\
	tbb::parallel_for											\
	  (table_range<table_iterator<C,B>>(begin, end),			\
	   [root, &f](const table_range<table_iterator<C,B>>& r) {	\
		const auto table0 = r.begin().table;					\
		const auto index0 = r.begin().index;					\
		const auto tablen = r.end().table;						\
		const auto indexn = r.end().index;						\
																\
		const auto delta = (table0-root)*B-index0;				\
																\
		if (table0 < tablen) {									\
		  __VA_ARGS__											\
			for (size_t j=index0; j<B; ++j) {					\
			  auto obj = table0[0][j]; f(delta+j, obj);			\
			}													\
		  const auto range = tablen-table0;						\
		  for (ptrdiff_t i=1; i<range; ++i) {					\
			const auto offset = delta+i*B;						\
			__VA_ARGS__											\
			  for (size_t j=0; j<B; ++j) {						\
				auto obj = table0[i][j]; f(offset+j, obj);		\
			  }													\
		  }														\
		  const auto offset = delta+range*B;					\
		  __VA_ARGS__											\
			for (size_t j=0; j<indexn; ++j) {					\
			  auto obj = tablen[0][j]; f(offset+j, obj);		\
			}													\
		} else if (table0 == tablen) {							\
		  __VA_ARGS__											\
			for (size_t j=index0; j<indexn; ++j) {				\
			  auto obj = table0[0][j]; f(delta+j, obj);			\
			}													\
		}														\
	  });														\
  }

  def_parallel_indexed_for_each_iterator(parallel_indexed_for_each);

#ifdef __ICC
  def_parallel_indexed_for_each_iterator(parallel_vector_indexed_for_each, _Pragma("vector always"));
  def_parallel_indexed_for_each_iterator(parallel_ivdep_indexed_for_each, _Pragma("ivdep"));
  def_parallel_indexed_for_each_iterator(parallel_vector_ivdep_indexed_for_each, _Pragma("ivdep") _Pragma("vector always"));
  def_parallel_indexed_for_each_iterator(parallel_simd_indexed_for_each, _Pragma("simd"));
  def_parallel_indexed_for_each_iterator(parallel_novector_indexed_for_each, _Pragma("novector"));
#endif

}

#endif
