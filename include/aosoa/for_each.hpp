/*
Copyright (c) 2013, Intel Corporation All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AOSOA_FOR_EACH
#define AOSOA_FOR_EACH

#include <cstddef>
#include <tuple>

#include "soa/table_traits.hpp"

#include "aosoa/apply_tuple.hpp"
#include "aosoa/table_iterator.hpp"

#include "aosoa/for_each_range.hpp"

namespace aosoa {

#define def_for_each(name, ...)											\
  template<typename F, class... CN>										\
  static inline void name(const F& f, C& first, CN&... rest){			\
	for_each_range														\
	  ([&f](size_t start, size_t end,									\
			typename soa::table_traits<C>::table_reference first,		\
			typename soa::table_traits<CN>::table_reference... rest){	\
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(first[i], rest[i]...)); \
	  }, first, rest...);												\
  }

  namespace {
	template<class C> class _for_each {
	public:
	  def_for_each(loop);
#ifdef __ICC
	  def_for_each(vector_loop, _Pragma("vector always"));
	  def_for_each(ivdep_loop, _Pragma("ivdep"));
	  def_for_each(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each(simd_loop, _Pragma("simd"));
	  def_for_each(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename F, class C, class... CN>
  inline void for_each(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_for_each<C>::loop(f, first, rest...);
  }

#ifdef __ICC
  template<typename F, class C, class... CN>
  inline void vector_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_for_each<C>::vector_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void ivdep_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_for_each<C>::ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void vector_ivdep_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_for_each<C>::vector_ivdep_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void simd_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_for_each<C>::simd_loop(f, first, rest...);
  }

  template<typename F, class C, class... CN>
  inline void novector_for_each(const F& f, C& first, CN&... rest)
  {
#pragma forceinline recursive
	_for_each<C>::novector_loop(f, first, rest...);
  }
#endif


#define def_for_each_it(name, ...)										\
  template<typename F, typename... TN>									\
  static inline void name(T begin, T end, const F& f, TN... others){	\
	for_each_range														\
	  (begin, end,														\
	   [&f](size_t start, size_t end,									\
			typename table_iterator_traits<T>::table_reference first,	\
			typename table_iterator_traits<TN>::table_reference... rest){ \
		__VA_ARGS__														\
		  for (size_t i=start; i<end; ++i)								\
			apply_tuple(f, std::forward_as_tuple(first[i], rest[i]...)); \
	  }, others...);													\
  }

  namespace {
	template<typename T> class _for_each_it {
	public:
	  def_for_each_it(loop);
#ifdef __ICC
	  def_for_each_it(vector_loop, _Pragma("vector always"));
	  def_for_each_it(ivdep_loop, _Pragma("ivdep"));
	  def_for_each_it(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each_it(simd_loop, _Pragma("simd"));
	  def_for_each_it(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename T, typename F, typename... TN>
  inline void for_each(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_for_each_it<T>::loop(begin, end, f, others...);
  }

#ifdef __ICC
  template<typename T, typename F, typename... TN>
  inline void vector_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_for_each_it<T>::vector_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void ivdep_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_for_each_it<T>::ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void vector_ivdep_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_for_each_it<T>::vector_ivdep_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void simd_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_for_each_it<T>::simd_loop(begin, end, f, others...);
  }

  template<typename T, typename F, typename... TN>
  inline void novector_for_each(T begin, T end, const F& f, TN... others)
  {
#pragma forceinline recursive
	_for_each_it<T>::novector_loop(begin, end, f, others...);
  }
#endif

}

#endif
