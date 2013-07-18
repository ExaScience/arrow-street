/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_INDEXED_FOR_EACH
#define AOSOA_INDEXED_FOR_EACH

#include <cstddef>
#include <tuple>

#include "aosoa/apply_tuple.hpp"
#include "soa/table_traits.hpp"
#include "aosoa/table_iterator.hpp"

#include "aosoa/indexed_for_each_range.hpp"

namespace aosoa {

#define def_indexed_for_each(name, ...)									\
  template<typename F, class... CN>										\
  static inline void name(const F& f, C& first, CN&... rest){			\
	indexed_for_each_range												\
	  ([&f](size_t start, size_t end, size_t offset,					\
			typename soa::table_traits<C>::table_reference first,		\
			typename soa::table_traits<CN>::table_reference... rest){	\
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, first, rest...);												\
  }

  namespace {
	template<class C> class _indexed_for_each {
	public:
	  def_indexed_for_each(loop);
#ifdef __ICC
	  def_indexed_for_each(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each(simd_loop, _Pragma("simd"));
	  def_indexed_for_each(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename F, class C, class... CN>
  inline void indexed_for_each(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_indexed_for_each<C>::loop(f, first, rest...);
  }

#ifdef __ICC
  template<typename F, class C, class... CN>
  inline void vector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_indexed_for_each<C>::vector_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_indexed_for_each<C>::ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void vector_ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_indexed_for_each<C>::vector_ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void simd_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_indexed_for_each<C>::simd_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void novector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_indexed_for_each<C>::novector_loop(f, first, rest...);
  }
#endif


#define def_indexed_for_each_it(name, ...)								\
  template<typename F, typename... TN>									\
  static inline void name(T begin, T end, const F& f, TN... others){	\
	indexed_for_each_range												\
	  (begin, end,														\
	   [&f](size_t start, size_t end, size_t offset,					\
			typename table_iterator_traits<T>::table_reference first,	\
			typename table_iterator_traits<TN>::table_reference... rest){ \
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, others...);													\
  }

  namespace {
	template<typename T> class _indexed_for_each_it {
	public:
	  def_indexed_for_each_it(loop);
#ifdef __ICC
	  def_indexed_for_each_it(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each_it(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each_it(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each_it(simd_loop, _Pragma("simd"));
	  def_indexed_for_each_it(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename T, typename F, typename... TN>
  inline void indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_indexed_for_each_it<T>::loop(begin, end, f, others...);
  }

#ifdef __ICC
  template<typename T, typename F, typename... TN>
  inline void vector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_indexed_for_each_it<T>::vector_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_indexed_for_each_it<T>::ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void vector_ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_indexed_for_each_it<T>::vector_ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void simd_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_indexed_for_each_it<T>::simd_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void novector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_indexed_for_each_it<T>::novector_loop(begin, end, f, others...);
  }
#endif

}

#endif
