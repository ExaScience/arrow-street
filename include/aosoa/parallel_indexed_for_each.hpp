/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH
#define AOSOA_PARALLEL_INDEXED_FOR_EACH

#include <cstddef>
#include <tuple>

#include "soa/table_traits.hpp"

#include "aosoa/apply_tuple.hpp"
#include "aosoa/table_iterator.hpp"

#include "aosoa/parallel_indexed_for_each_range.hpp"

namespace aosoa {

#ifndef NOTBB
#define def_parallel_indexed_for_each(name, ...)						\
  template<typename F, class... CN>										\
  static inline void name(const F& f, C& first, CN&... rest) {			\
	parallel_indexed_for_each_range										\
	  ([&f](size_t start, size_t end, size_t offset,					\
			typename soa::table_traits<C>::table_reference first,		\
			typename soa::table_traits<CN>::table_reference... rest){	\
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, first, rest...);												\
  }
#endif

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each(name, ...)					\
  template<typename F, class... CN>										\
  static inline void name(const F& f, C& first, CN&... rest) {			\
	cilk_parallel_indexed_for_each_range								\
	  ([&f](size_t start, size_t end, size_t offset,					\
			typename soa::table_traits<C>::table_reference first,		\
			typename soa::table_traits<CN>::table_reference... rest){	\
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, first, rest...);												\
  }
#endif

  namespace {
	template<class C> class _parallel_indexed_for_each {
	public:
#ifndef NOTBB
	  def_parallel_indexed_for_each(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each(novector_loop, _Pragma("novector"));
#endif
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};
  }

#ifndef NOTBB
  template<typename F, class C, class... CN>
  inline void parallel_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each<C>::loop(f, first, rest...);
  }

#ifdef __ICC
  template<typename F, class C, class... CN>
  inline void parallel_vector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::vector_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void parallel_ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void parallel_vector_ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::vector_ivdep_loop(f, first, rest);
  }

  template<typename F, class C, class... CN>
  inline void parallel_simd_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::simd_loop(f, first, rest);
  }

  template<typename F, class C, class... CN>
  inline void parallel_novector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::novector_loop(f, first, rest...);
  }
#endif
#endif

#ifdef __cilk
  template<typename F, class C, class... CN>
  inline void cilk_parallel_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_loop(f, first, rest...);
  }

#ifdef __ICC
  template<typename F, class C, class... CN>
  inline void cilk_parallel_vector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_vector_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void cilk_parallel_ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void cilk_parallel_vector_ivdep_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_vector_ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void cilk_parallel_simd_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_simd_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void cilk_parallel_novector_indexed_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each<C>::cilk_novector_loop(f, first, rest...);
  }
#endif
#endif


#ifndef NOTBB
#define def_parallel_indexed_for_each_it(name, ...)						\
  template<typename F, typename... TN>									\
  static inline void name(T begin, T end, const F& f, TN... others) {	\
	parallel_indexed_for_each_range										\
	  (begin, end,														\
	   [&f](size_t start, size_t end, size_t offset,					\
			typename table_iterator_traits<T>::table_reference first,	\
			typename table_iterator_traits<TN>::table_reference... rest){ \
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, others...);													\
  }
#endif

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each_it(name, ...)				\
  template<typename F, typename... TN>									\
  static inline void name(T begin, T end, const F& f, TN... others) {	\
	cilk_parallel_indexed_for_each_range								\
	  (begin, end,														\
	   [&f](size_t start, size_t end, size_t offset,					\
			typename table_iterator_traits<T>::table_reference first,	\
			typename table_iterator_traits<TN>::table_reference... rest){ \
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(offset+i, first[i], rest[i]...)); \
	  }, others...);													\
  }
#endif

  namespace {
	template<typename T> class _parallel_indexed_for_each_it {
	public:
#ifndef NOTBB
	  def_parallel_indexed_for_each_it(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each_it(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each_it(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each_it(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each_it(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each_it(novector_loop, _Pragma("novector"));
#endif
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each_it(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each_it(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each_it(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each_it(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};
  }

  template<typename T, typename F, typename... TN>
  inline void parallel_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_it<T>::loop(begin, end, f, others...);
  }

#ifdef __ICC
  template<typename T, typename F, typename... TN>
  inline void parallel_vector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::vector_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void parallel_ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void parallel_vector_ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::vector_ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void parallel_simd_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::simd_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void parallel_novector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::novector_loop(begin, end, f, others...);
  }
#endif

#ifdef __cilk
  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_indexed_for_each_it<T>::cilk_loop(begin, end, f, others...);
  }

#ifdef __ICC
  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_vector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::cilk_vector_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::cilk_ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_vector_ivdep_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::cilk_vector_ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_simd_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::cilk_simd_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_novector_indexed_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_parallel_indexed_for_each_it<T>::cilk_novector_loop(begin, end, f, others...);
  }
#endif
#endif

}

#endif
