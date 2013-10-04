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

#ifndef AOSOA_PARALLEL_FOR_EACH_RANGE
#define AOSOA_PARALLEL_FOR_EACH_RANGE

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
	template<bool is_compatibly_tabled, class C, class... CN> class _parallel_for_each_range;

	template<class C, class... CN>
	class _parallel_for_each_range<true, C, CN...> {
	public:
#ifndef NOTBB

#if defined(__ICC) || (GCC_VERSION >= 40900)
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		const tbb::blocked_range<size_t> range(0, sdb);

		auto const fun = [&f, &first, &rest...](const tbb::blocked_range<size_t>& r) {
		  for (size_t i=r.begin(); i<r.end(); ++i)
			f(0, traits::table_size, first.data()[i], rest.data()[i]...);
		};

		if (smb) {
		  tbb::task_group g;
		  g.run([&f,&first,&rest...,sdb,smb]
				{f(0, smb, first.data()[sdb], rest.data()[sdb]...);});
		  tbb::parallel_for(range, fun);
		  g.wait();
		} else tbb::parallel_for(range, fun);
	  }
#else
	  // capturing parameter packs is not supported in GCC 4.8.x.
	  template<typename F>
	  static inline void loop(const F& f, C& first) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		const tbb::blocked_range<size_t> range(0, sdb);

		auto const fun = [&f, &first](const tbb::blocked_range<size_t>& r) {
		  for (size_t i=r.begin(); i<r.end(); ++i)
			f(0, traits::table_size, first.data()[i]);
		};

		if (smb) {
		  tbb::task_group g;
		  g.run([&f,&first,sdb,smb]
				{f(0, smb, first.data()[sdb]);});
		  tbb::parallel_for(range, fun);
		  g.wait();
		} else tbb::parallel_for(range, fun);
	  }
#endif

#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(const F& f, C& first, CN&... rest) {
		typedef soa::table_traits<C> traits;
		const auto size = first.size();
		const auto sdb = size/traits::table_size;
		const auto smb = size%traits::table_size;

		if (smb)
		  cilk_spawn f(0, smb, first.data()[sdb], rest.data()[sdb]...);
		/*
		cilk_for (size_t i=0; i<sdb; ++i)
		  f(0, traits::table_size, first.data()[i], rest.data()[i]...);
		*/
		cilk_iterate((decltype(sdb))0,sdb,[&f, &first, &rest...](decltype(sdb) i){
			f(0, traits::table_size, first.data()[i], rest.data()[i]...);
		  });
	  }
#endif
	};

	template<class C, class... CN>
	class _parallel_for_each_range<false, C, CN...> {
	public:
#ifndef NOTBB

#if defined(__ICC) || (GCC_VERSION >= 40900)
	  template<typename F>
	  static inline void loop(const F& f, C& first, CN&... rest) {
		tbb::parallel_for
		  (tbb::blocked_range<typename C::iterator>
		   (first.begin(), first.end()),
		   [&f,&first,&rest...](const tbb::blocked_range<typename C::iterator>& r){
			auto offset = r.begin() - first.begin();
			f(0, r.end()-r.begin(), r.begin(), (rest.begin()+offset)...);
		  });
	  }
#else
	  // capturing parameter packs is not supported in GCC 4.8.x.
	  template<typename F>
	  static inline void loop(const F& f, C& first) {
		tbb::parallel_for
		  (tbb::blocked_range<typename C::iterator>
		   (first.begin(), first.end()),
		   [&f,&first](const tbb::blocked_range<typename C::iterator>& r){
			auto offset = r.begin() - first.begin();
			f(0, r.end()-r.begin(), r.begin());
		  });
	  }
#endif

#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(const F& f, C& first, CN&... rest) {
		auto begin = first.begin();
		auto end = first.end();
		auto span = end-begin;
		auto grainsize = std::min(2048l, span / (8 * __cilkrts_get_nworkers()));
		if (grainsize == 0) grainsize = 1;
		/*
		cilk_for (auto it=begin; it<end; it+=grainsize) {
		  auto offset = it - first.begin();
		  f(0, std::min(grainsize, end-it), it, (rest.begin()+offset)...);
		}
		*/
		cilk_iterate
		  (begin, end, grainsize,
		   [&f, &first, &rest...](decltype(begin) lbegin, decltype(end) lend){
			auto offset = lbegin - first.begin();
			f(0, lend-lbegin, lbegin, (rest.begin()+offset)...);
		  });
	  }
#endif
	};
  }

#ifndef NOTBB
  template<typename F, class C, class... CN>
  inline void parallel_for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_for_each_range<soa::is_compatibly_tabled<C, CN...>::value, C, CN...>::
	  loop(f, first, rest...);
  }
#endif

#ifdef __cilk
  template<typename F, class C, class... CN>
  inline void cilk_parallel_for_each_range(const F& f, C& first, CN&... rest)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_for_each_range<soa::is_compatibly_tabled<C, CN...>::value, C, CN...>::
	  cilk_loop(f, first, rest...);
  }
#endif

  namespace {
	template<bool is_compatibly_tabled, typename T, typename... TN> class _parallel_for_each_range_it;

	template<typename T, typename... TN>
	class _parallel_for_each_range_it<true, T, TN...> {
	public:
#ifndef NOTBB

#if defined(__ICC) || (GCC_VERSION >= 40900)
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		typedef table_iterator_traits<T> traits;
		tbb::parallel_for
		  (table_range<T>(begin, end),
		   [&f, begin, others...](const table_range<T>& r) {
			const auto table0 = r.begin().table;
			const auto index0 = r.begin().index;
			const auto tablen = r.end().table;
			const auto indexn = r.end().index;

			auto offset = table0 - begin.table;

			if (table0 < tablen) {
			  f(index0, traits::table_size, table0[0], others.table[offset]...);
			  const auto range = tablen-table0;
			  for (ptrdiff_t i=1; i<range; ++i)
				f(0, traits::table_size, table0[i], others.table[offset+i]...);
			  f(0, indexn, table0[range], others.table[offset+range]...);
			} else if (table0 == tablen) {
			  f(index0, indexn, table0[0], others.table[offset]...);
			}
		  });
	  }
#else
	  // capturing parameter packs is not supported in GCC 4.8.x.
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		typedef table_iterator_traits<T> traits;
		tbb::parallel_for
		  (table_range<T>(begin, end),
		   [&f, begin](const table_range<T>& r) {
			const auto table0 = r.begin().table;
			const auto index0 = r.begin().index;
			const auto tablen = r.end().table;
			const auto indexn = r.end().index;

			auto offset = table0 - begin.table;

			if (table0 < tablen) {
			  f(index0, traits::table_size, table0[0]);
			  const auto range = tablen-table0;
			  for (ptrdiff_t i=1; i<range; ++i)
				f(0, traits::table_size, table0[i]);
			  f(0, indexn, table0[range]);
			} else if (table0 == tablen) {
			  f(index0, indexn, table0[0]);
			}
		  });
	  }
#endif

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
		  cilk_spawn f(index0, traits::table_size, table0[0], others.table[0]...);
		  const auto range = tablen-table0;
		  cilk_spawn f(0, indexn, table[range], others.table[range]...);
		  cilk_for (ptrdiff_t i=1; i<range; ++i)
			f(0, traits::table_size, table0[i], others.table[i]...);
		} else if (table0 == tablen) {
		  f(index0, indexn, table0[0], others.table[0]...);
		}
	  }
#endif
	};

	template<typename T, typename... TN>
	class _parallel_for_each_range_it<false, T, TN...> {
	public:
#ifndef NOTBB

#if defined(__ICC) || (GCC_VERSION >= 40900)
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f, TN... others) {
		tbb::parallel_for
		  (tbb::blocked_range<T>(begin, end),
		   [&f, begin, others...](const tbb::blocked_range<T>& r){
			auto offset = r.begin() - begin;
			f(0, r.end()-r.begin(), r.begin(), (others+offset)...);
		  });
	  }
#else
	  // capturing parameter packs is not supported in GCC 4.8.x.
	  template<typename F>
	  static inline void loop(T begin, T end, const F& f) {
		tbb::parallel_for
		  (tbb::blocked_range<T>(begin, end),
		   [&f, begin](const tbb::blocked_range<T>& r){
			auto offset = r.begin() - begin;
			f(0, r.end()-r.begin(), r.begin());
		  });
	  }
#endif

#endif

#ifdef __cilk
	  template<typename F>
	  static inline void cilk_loop(T begin, T end, const F& f, TN... others) {
		auto span = end-begin;
		size_t grainsize = std::min(2048, span / (8 * __cilkrts_get_nworkers()));
		if (grainsize == 0) grainsize = 1;
		cilk_for (auto it=begin; it<end; it+=grainsize) {
		  auto offset = it - begin;
		  f(0, std::min(grainsize, end-it), it, (others+offset)...);
		}
	  }
#endif
	};
  }

#ifndef NOTBB
  template<typename T, typename F, typename... TN>
  inline void parallel_for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_for_each_range_it<is_compatibly_tabled_iterator<T, TN...>::value, T, TN...>::
	  loop(begin, end, f, others...);
  }
#endif

#ifdef __cilk
  template<typename T, typename F, typename... TN>
  inline void cilk_parallel_for_each_range(T begin, T end, const F& f, TN... others)
  {
#ifdef __ICC
#pragma forceinline recursive
#endif
	_parallel_for_each_range_it<is_compatibly_tabled_iterator<T, TN...>::value, T, TN...>::
	  loop(begin, end, f, others...);
  }
#endif
}

#endif
