/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_INDEXED_FOR_EACH
#define AOSOA_PARALLEL_INDEXED_FOR_EACH

#include <cstddef>

#include <algorithm>

#include "aosoa/table_iterator.hpp"
#include "aosoa/table_range.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task_group.h"

#ifdef __cilk
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#endif

namespace aosoa {

#define def_parallel_indexed_for_each_tabled(name, ...)					\
  template<typename F>													\
  static inline void name(C& container, const F& f) {					\
	const auto size = container.size();									\
	auto data = container.data();										\
	const auto sdb = size/traits::table_size;							\
	const auto smb = size%traits::table_size;							\
																		\
	const tbb::blocked_range<size_t> range(0, sdb);						\
																		\
	auto const fun = [&f,data](const tbb::blocked_range<size_t>& r) {	\
	  for (size_t i=r.begin(); i<r.end(); ++i) {						\
		auto& t = data[i];												\
		const auto offset = i*traits::table_size;						\
		__VA_ARGS__														\
		for (size_t j=0; j<traits::table_size; ++j)	{					\
		  auto obj = t[j]; f(offset+j, obj);							\
		}																\
	  }																	\
	};																	\
	  																	\
	if (smb) {															\
	  tbb::task_group g;												\
	  g.run([&f,data,sdb,smb]{											\
		  auto& t = data[sdb];											\
		  const auto offset = sdb*traits::table_size;					\
		  __VA_ARGS__													\
			for (size_t j=0; j<smb; ++j) {								\
			  auto obj = t[j]; f(offset+j, obj);						\
			}															\
		});																\
	  tbb::parallel_for(range, fun);									\
	  g.wait();															\
	} else tbb::parallel_for(range, fun);								\
  }

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each_tabled(name, ...)			\
  template<typename F>													\
  static inline void name(C& container, const F& f) {					\
	const auto size = container.size();									\
	auto data = container.data();										\
	const auto sdb = size/traits::table_size;							\
	const auto smb = size%traits::table_size;							\
																		\
	if (smb) {															\
	  cilk_spawn [&f,data,sdb,smb]{										\
		auto& t = data[sdb];											\
		const auto offset = sdb*traits::table_size;						\
		__VA_ARGS__														\
		  for (size_t j=0; j<smb; ++j) {								\
			auto obj = t[j]; f(offset+j, obj);							\
		  }																\
	  }();																\
	}																	\
	cilk_for (size_t i=0; i<sdb; ++i) {									\
	  auto& t = data[i];												\
	  const auto offset = i*traits::table_size;							\
	  __VA_ARGS__														\
		for (size_t j=0; j<traits::table_size; ++j) {					\
		  auto obj = t[j]; f(offset+j, obj);							\
		}																\
	}																	\
  }
#endif

#define def_parallel_indexed_for_each_non_tabled(name, ...)				\
  template<typename F>													\
  static inline void name(C& container, const F& f) {					\
	const auto root = container.begin();								\
	tbb::parallel_for													\
	  (tbb::blocked_range<typename C::iterator>							\
	   (container.begin(), container.end()),							\
	   [&f, root](const tbb::blocked_range<typename C::iterator>& r){	\
		auto begin = r.begin();											\
		auto end = r.end();												\
		__VA_ARGS__														\
		  for (auto it=begin; it != end; ++it)							\
			f(it-root, *it);											\
	  });																\
  }

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each_non_tabled(name, ...)		\
  template<typename F>													\
  static inline void name(C& container, const F& f) {					\
	auto begin = container.begin();										\
	auto end = container.end();											\
	auto span = end-begin;												\
	auto grainsize = std::min(2048l, span / (8 * __cilkrts_get_nworkers())); \
	if (grainsize == 0) grainsize = 1;									\
	cilk_for (auto it=begin; it<end; it+=grainsize)	{					\
	  const auto n = std::min(grainsize, end-it);						\
	  const auto offset = it-begin;										\
	  __VA_ARGS__														\
		for (size_t j=0; j<n; ++j) f(offset+j, it[j]);					\
	}																	\
  }
#endif

  namespace {
	template<class C, typename Enable = void> class _parallel_indexed_for_each;

	template<class C>
	class _parallel_indexed_for_each<C, typename std::enable_if<soa::table_traits<C>::tabled>::type> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  def_parallel_indexed_for_each_tabled(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each_tabled(novector_loop, _Pragma("novector"));
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each_tabled(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each_tabled(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_tabled(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each_tabled(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_tabled(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each_tabled(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};

	template<typename C>
	class _parallel_indexed_for_each<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  def_parallel_indexed_for_each_non_tabled(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each_non_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each_non_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each_non_tabled(novector_loop, _Pragma("novector"));
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each_non_tabled(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};
  }

  template<class C, typename F>
  inline void parallel_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::loop(container, f);}

#ifdef __ICC
  template<class C, typename F>
  inline void parallel_vector_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::vector_loop(container, f);}

  template<class C, typename F>
  inline void parallel_ivdep_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::ivdep_loop(container, f);}

  template<class C, typename F>
  inline void parallel_vector_ivdep_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::vector_ivdep_loop(container, f);}

  template<class C, typename F>
  inline void parallel_simd_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::simd_loop(container, f);}

  template<class C, typename F>
  inline void parallel_novector_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::novector_loop(container, f);}
#endif


#ifdef __cilk
  template<class C, typename F>
  inline void cilk_parallel_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_loop(container, f);}

#ifdef __ICC
  template<class C, typename F>
  inline void cilk_parallel_vector_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_vector_loop(container, f);}

  template<class C, typename F>
  inline void cilk_parallel_ivdep_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_ivdep_loop(container, f);}

  template<class C, typename F>
  inline void cilk_parallel_vector_ivdep_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_vector_ivdep_loop(container, f);}

  template<class C, typename F>
  inline void cilk_parallel_simd_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_simd_loop(container, f);}

  template<class C, typename F>
  inline void cilk_parallel_novector_indexed_for_each(C& container, const F& f)
  {_parallel_indexed_for_each<C>::cilk_novector_loop(container, f);}
#endif
#endif


#define def_parallel_indexed_for_each_it_tabled(name, ...)			\
  template<typename F>												\
  static inline void name(T begin, T end, const F& f) {				\
	const auto root = begin.table;									\
	tbb::parallel_for												\
	  (table_range<T>(begin, end),									\
	   [&f, root](const table_range<T>& r) {						\
		const auto table0 = r.begin().table;						\
		const auto index0 = r.begin().index;						\
		const auto tablen = r.end().table;							\
		const auto indexn = r.end().index;							\
																	\
		const auto delta = (table0-root)*traits::table_size-index0;	\
																	\
		if (table0 < tablen) {										\
		  __VA_ARGS__												\
			for (size_t j=index0; j<traits::table_size; ++j) {		\
			  auto obj = table0[0][j]; f(delta+j, obj);				\
			}														\
		  const auto range = tablen-table0;							\
		  for (ptrdiff_t i=1; i<range; ++i) {						\
			const auto offset = delta+i*traits::table_size;			\
			__VA_ARGS__												\
			  for (size_t j=0; j<traits::table_size; ++j) {			\
				auto obj = table0[i][j]; f(offset+j, obj);			\
			  }														\
		  }															\
		  const auto offset = delta+range*traits::table_size;		\
		  __VA_ARGS__												\
			for (size_t j=0; j<indexn; ++j) {						\
			  auto obj = tablen[0][j]; f(offset+j, obj);			\
			}														\
		} else if (table0 == tablen) {								\
		  __VA_ARGS__												\
			for (size_t j=index0; j<indexn; ++j) {					\
			  auto obj = table0[0][j]; f(delta+j, obj);				\
			}														\
		}															\
	  });															\
  }

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each_it_tabled(name, ...)		\
  template<typename F>												\
  static inline void name(T begin, T end, const F& f) {				\
	const auto table0 = begin.table;								\
	const auto index0 = begin.index;								\
	const auto tablen = end.table;									\
	const auto indexn = end.index;									\
																	\
	if (table0 < tablen) {											\
	  cilk_spawn [=]{												\
		__VA_ARGS__													\
		  for (size_t j=index0; j<traits::table_size; ++j) {		\
			auto obj = table0[0][j]; f(j-index0, obj);				\
		  }															\
	  }();															\
	  const auto range = tablen-table0;								\
	  cilk_spawn [=]{												\
		const auto offset = range*traits::table_size-index0;		\
		__VA_ARGS__													\
		  for (size_t j=0; j<indexn; ++j) {							\
			auto obj = tablen[0][j]; f(offset+j, obj);				\
		  }															\
	  }();															\
	  cilk_for (ptrdiff_t i=1; i<range; ++i) {						\
		const auto offset = i*traits::table_size-index0;			\
		__VA_ARGS__													\
		  for (size_t j=0; j<traits::table_size; ++j) {				\
			auto obj = table0[i][j]; f(offset+j, obj);				\
		  }															\
	  }																\
	} else if (table0 == tablen) {									\
	  __VA_ARGS__													\
		for (size_t j=index0; j<indexn; ++j) {						\
		  auto obj = table0[0][j]; f(j-index0, obj);				\
		}															\
	}																\
  }
#endif

#define def_parallel_indexed_for_each_it_non_tabled(name, ...)			\
  template<typename F>													\
  static inline void name(T begin, T end, const F& f) {					\
	const auto root = begin;											\
	tbb::parallel_for													\
	  (tbb::blocked_range<T>(begin, end),								\
	   [&f, root](const tbb::blocked_range<T>& r){						\
		const auto begin = r.begin();									\
		const auto end = r.end();										\
		__VA_ARGS__														\
		  for (auto it=begin; it != end; ++it)							\
			f(it-root, *it);											\
	  });																\
  }

#ifdef __cilk
#define def_cilk_parallel_indexed_for_each_it_non_tabled(name, ...)		\
  template<typename F>													\
  static inline void name(T begin, T end, const F& f) {					\
	auto span = end-begin;												\
	auto grainsize = std::min(2048l, span / (8 * __cilkrts_get_nworkers())); \
	if (grainsize == 0) grainsize = 1;									\
	cilk_for (auto it=begin; it<end; it+=grainsize)	{					\
	  const auto n = std::min(grainsize, end-it);						\
	  const auto offset = it-begin;										\
	  __VA_ARGS__														\
		for (size_t j=0; j<n; ++j) f(offset+j, it[j]);					\
	}																	\
  }
#endif

  namespace {
	template<typename T, typename Enable = void> class _parallel_indexed_for_each_it;

	template<typename T>
	class _parallel_indexed_for_each_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  def_parallel_indexed_for_each_it_tabled(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each_it_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each_it_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each_it_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each_it_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each_it_tabled(novector_loop, _Pragma("novector"));
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each_it_tabled(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};

	template<typename T>
	class _parallel_indexed_for_each_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  def_parallel_indexed_for_each_it_non_tabled(loop);
#ifdef __ICC
	  def_parallel_indexed_for_each_it_non_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_indexed_for_each_it_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_indexed_for_each_it_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_indexed_for_each_it_non_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_indexed_for_each_it_non_tabled(novector_loop, _Pragma("novector"));
#endif
#ifdef __cilk
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_loop);
#ifdef __ICC
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_vector_loop, _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_ivdep_loop, _Pragma("ivdep"));
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_simd_loop, _Pragma("simd"));
	  def_cilk_parallel_indexed_for_each_it_non_tabled(cilk_novector_loop, _Pragma("novector"));
#endif
#endif
	};
  }

  template<typename T, typename F>
  inline void parallel_indexed_for_each(T begin, T end, const F& f)
  {_parallel_indexed_for_each_it<T>::loop(begin, end, f);}

#ifdef __ICC
  template<typename T, typename F>
  inline void parallel_vector_indexed_for_each(T begin, T end, const F& f)
  {_parallel_indexed_for_each_it<T>::vector_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_ivdep_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_vector_ivdep_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::vector_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_simd_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::simd_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_novector_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::novector_loop(begin, end, f);}
#endif

#ifdef __cilk
  template<typename T, typename F>
  inline void cilk_parallel_indexed_for_each(T begin, T end, const F& f)
  {_parallel_indexed_for_each_it<T>::cilk_loop(begin, end, f);}

#ifdef __ICC
  template<typename T, typename F>
  inline void cilk_parallel_vector_indexed_for_each(T begin, T end, const F& f)
  {_parallel_indexed_for_each_it<T>::cilk_vector_loop(begin, end, f);}

  template<typename T, typename F>
  inline void cilk_parallel_ivdep_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::cilk_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void cilk_parallel_vector_ivdep_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::cilk_vector_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void cilk_parallel_simd_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::cilk_simd_loop(begin, end, f);}

  template<typename T, typename F>
  inline void cilk_parallel_novector_indexed_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::cilk_novector_loop(begin, end, f);}
#endif
#endif

}

#endif
