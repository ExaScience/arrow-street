/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_PARALLEL_FOR_EACH
#define AOSOA_PARALLEL_FOR_EACH

#include <cstddef>

#include <algorithm>

#include "aosoa/table_iterator.hpp"
#include "aosoa/table_range.hpp"

#include "tbb/blocked_range.h"
#include "tbb/parallel_for.h"
#include "tbb/task_group.h"

namespace aosoa {

#define def_parallel_for_each_tabled(name, ...)							\
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
		__VA_ARGS__														\
		for (size_t j=0; j<traits::table_size; ++j)	{					\
		  auto obj = t[j]; f(obj);										\
		}																\
	  }																	\
	};																	\
	  																	\
	if (smb) {															\
	  tbb::task_group g;												\
	  g.run([&f,data,sdb,smb]{											\
		  auto& t = data[sdb];											\
		  __VA_ARGS__													\
			for (size_t j=0; j<smb; ++j) {								\
			  auto obj = t[j]; f(obj);									\
			}															\
		});																\
	  tbb::parallel_for(range, fun);									\
	  g.wait();															\
	} else tbb::parallel_for(range, fun);								\
  }

#define def_parallel_for_each_non_tabled(name, ...)						\
  template<typename F>													\
  static inline void name(C& container, const F& f) {					\
	tbb::parallel_for													\
	  (tbb::blocked_range<typename C::iterator>							\
	   (container.begin(), container.end()),							\
	   [&f](const tbb::blocked_range<typename C::iterator>& r){			\
		const auto begin = r.begin();									\
		const auto end = r.end();										\
		__VA_ARGS__														\
		  for (auto it=begin; it != end; ++it)							\
			f(*it);														\
	  });																\
  }

  namespace {
	template<class C, typename Enable = void> class _parallel_for_each;

	template<class C>
	class _parallel_for_each<C, typename std::enable_if<soa::table_traits<C>::tabled>::type> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  def_for_each_tabled(loop);
#ifdef __ICC
	  def_parallel_for_each_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_for_each_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_for_each_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_for_each_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_for_each_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename C>
	class _parallel_for_each<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  def_parallel_for_each_non_tabled(loop);
#ifdef __ICC
	  def_parallel_for_each_non_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_for_each_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_for_each_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_for_each_non_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_for_each_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<class C, typename F>
  inline void parallel_for_each(C& container, const F& f)
  {_parallel_for_each<C>::loop(container, f);}

#ifdef __ICC
  template<class C, typename F>
  inline void parallel_vector_for_each(C& container, const F& f)
  {_parallel_for_each<C>::vector_loop(container, f);}

  template<class C, typename F>
  inline void parallel_ivdep_for_each(C& container, const F& f)
  {_parallel_for_each<C>::ivdep_loop(container, f);}

  template<class C, typename F>
  inline void parallel_vector_ivdep_for_each(C& container, const F& f)
  {_parallel_for_each<C>::vector_ivdep_loop(container, f);}

  template<class C, typename F>
  inline void parallel_simd_for_each(C& container, const F& f)
  {_parallel_for_each<C>::simd_loop(container, f);}

  template<class C, typename F>
  inline void parallel_novector_for_each(C& container, const F& f)
  {_parallel_for_each<C>::novector_loop(container, f);}
#endif


#define def_parallel_for_each_it_tabled(name, ...)				\
  template<typename F>											\
  static inline void name(T begin, T end, const F& f) {			\
	tbb::parallel_for											\
	  (table_range<T>(begin, end),								\
	   [&f](const table_range<T>& r) {							\
		const auto table0 = r.begin().table;					\
		const auto index0 = r.begin().index;					\
		const auto tablen = r.end().table;						\
		const auto indexn = r.end().index;						\
																\
		if (table0 < tablen) {									\
		  __VA_ARGS__											\
			for (size_t j=index0; j<traits::table_size; ++j) {	\
			  auto obj = table0[0][j]; f(obj);					\
			}													\
		  const auto range = tablen-table0;						\
		  for (ptrdiff_t i=1; i<range; ++i) {					\
			__VA_ARGS__											\
			  for (size_t j=0; j<traits::table_size; ++j) {		\
				auto obj = table0[i][j]; f(obj);				\
			  }													\
		  }														\
		  __VA_ARGS__											\
			for (size_t j=0; j<indexn; ++j) {					\
			  auto obj = tablen[0][j]; f(obj);					\
			}													\
		} else if (table0 == tablen) {							\
		  __VA_ARGS__											\
			for (size_t j=index0; j<indexn; ++j) {				\
			  auto obj = table0[0][j]; f(obj);					\
			}													\
		}														\
	  });														\
  }

#define def_parallel_for_each_it_non_tabled(name, ...)					\
  template<typename F>													\
  static inline void name(T begin, T end, const F& f) {					\
	tbb::parallel_for													\
	  (tbb::blocked_range<T>(begin, end),								\
	   [&f](const tbb::blocked_range<T>& r){							\
		const auto begin = r.begin();									\
		const auto end = r.end();										\
		__VA_ARGS__														\
		  for (auto it=begin; it != end; ++it)							\
			f(*it);														\
	  });																\
  }

  namespace {
	template<typename T, typename Enable = void> class _parallel_for_each_it;

	template<typename T>
	class _parallel_for_each_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  def_parallel_for_each_it_tabled(loop);
#ifdef __ICC
	  def_parallel_for_each_it_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_for_each_it_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_for_each_it_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_for_each_it_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_for_each_it_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename T>
	class _parallel_for_each_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  def_parallel_for_each_it_non_tabled(loop);
#ifdef __ICC
	  def_parallel_for_each_it_non_tabled(vector_loop, _Pragma("vector always"));
	  def_parallel_for_each_it_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_parallel_for_each_it_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_parallel_for_each_it_non_tabled(simd_loop, _Pragma("simd"));
	  def_parallel_for_each_it_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename T, typename F>
  inline void parallel_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::loop(begin, end, f);}

#ifdef __ICC
  template<typename T, typename F>
  inline void parallel_vector_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::vector_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_ivdep_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_vector_ivdep_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::vector_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_simd_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::simd_loop(begin, end, f);}

  template<typename T, typename F>
  inline void parallel_novector_for_each(T begin, T end, const F& f)
  {_parallel_for_each_it<T>::novector_loop(begin, end, f);}
#endif

}

#endif
