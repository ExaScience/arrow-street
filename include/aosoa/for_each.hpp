/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_FOR_EACH
#define AOSOA_FOR_EACH

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "soa/table_traits.hpp"
#include "aosoa/apply_tuple.hpp"
#include "aosoa/table_iterator.hpp"

#include "aosoa/for_each_range.hpp"

namespace aosoa {

  /*
#define def_for_each_tabled(name, ...)				  \
  template<typename F>									 \
  static inline void name(const F& f, C& container, CN&... other_containers){	\
	const auto size = container.size();				  \
	const auto sdb = size/traits::table_size;		  \
	const auto smb = size%traits::table_size;		  \
													  \
	for (size_t i=0; i<sdb; ++i) {					  \
	  __VA_ARGS__														\
		for (size_t j=0; j<traits::table_size; ++j) 					\
		  apply_tuple(f, apply_tuple(tuple_access_value<decltype(apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(i), std::make_tuple(container.data(), other_containers.data()...)))>(j), apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(i), std::make_tuple(container.data(), other_containers.data()...)))); \
	}																	\
	if (smb) {															\
	  __VA_ARGS__														\
		for (size_t j=0; j<smb; ++j)									\
		  apply_tuple(f, apply_tuple(tuple_access_value<decltype(apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(sdb), std::make_tuple(container.data(), other_containers.data()...)))>(j), apply_tuple(tuple_access_lvalue<decltype(std::make_tuple(container.data(), other_containers.data()...))>(sdb), std::make_tuple(container.data(), other_containers.data()...)))); \
	}																	\
  }
  */

#define def_for_each_tabled(name, ...)									\
  template<typename F>													\
  static inline void name(const F& f, C& container, CN&... other_containers){ \
	for_each_range([&](size_t start, size_t end,						\
					   typename soa::table_traits<C>::table_reference table, \
					   typename soa::table_traits<CN>::table_reference... tablen){ \
					 __VA_ARGS__										\
					   for (size_t i=start; i<end; ++i)					\
						 apply_tuple(f, std::make_tuple(table[i], tablen[i]...)); \
				   }, container, other_containers...);										\
  }																		\

#define def_for_each_non_tabled(name, ...)			 \
  template<typename F>								 \
  static inline void name(const F& f, C& container){				 \
	const auto begin = container.begin();			 \
	const auto end = container.end();				 \
	__VA_ARGS__										 \
	  for (auto it=begin; it < end; ++it)			 \
		f(*it);										 \
  }


  namespace {
	template<class C, typename Enable = void, class... CN> class _for_each;

	template<class C, class... CN>
	class _for_each<C, typename std::enable_if<soa::table_traits<C>::tabled>::type, CN...> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  def_for_each_tabled(loop);
#ifdef __ICC
	  def_for_each_tabled(vector_loop, _Pragma("vector always"));
	  def_for_each_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_for_each_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each_tabled(simd_loop, _Pragma("simd"));
	  def_for_each_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename C>
	class _for_each<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  def_for_each_non_tabled(loop);
#ifdef __ICC
	  def_for_each_non_tabled(vector_loop, _Pragma("vector always"));
	  def_for_each_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_for_each_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each_non_tabled(simd_loop, _Pragma("simd"));
	  def_for_each_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename F, class C, class... CN>
  inline void for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::loop(f, container, other_containers...);}

#ifdef __ICC
  template<typename F, class C, class... CN>
  inline void vector_for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::vector_loop(f, container, other_containers...);}

  template<typename F, class C, class... CN>
  inline void ivdep_for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::ivdep_loop(f, container, other_containers...);}

  template<typename F, class C, class... CN>
  inline void vector_ivdep_for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::vector_ivdep_loop(f, container, other_containers...);}

  template<typename F, class C, class... CN>
  inline void simd_for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::simd_loop(f, container, other_containers...);}

  template<typename F, class C, class... CN>
  inline void novector_for_each(const F& f, C& container, CN&... other_containers)
  {_for_each<C>::novector_loop(f, container, other_containers...);}
#endif


#define def_for_each_it_tabled(name, ...)				   \
  template<typename F>									   \
  static inline void name(T begin, T end, const F& f){	   \
	const auto table0 = begin.table;					   \
	const auto index0 = begin.index;					   \
	const auto tablen = end.table;						   \
	const auto indexn = end.index;						   \
														   \
	if (table0 < tablen) {								   \
	  __VA_ARGS__										   \
		for (size_t j=index0; j<traits::table_size; ++j) { \
		  auto obj = table0[0][j]; f(obj);				   \
		}												   \
	  const auto range = tablen-table0;					   \
	  for (ptrdiff_t i=1; i<range; ++i) {				   \
		__VA_ARGS__										   \
		  for (size_t j=0; j<traits::table_size; ++j) {	   \
			auto obj = table0[i][j]; f(obj);			   \
		  }												   \
	  }													   \
	  __VA_ARGS__										   \
		for (size_t j=0; j<indexn; ++j) {				   \
		  auto obj = tablen[0][j]; f(obj);				   \
		}												   \
	} else if (table0 == tablen) {						   \
	  __VA_ARGS__										   \
		for (size_t j=index0; j<indexn; ++j) {			   \
		  auto obj = table0[0][j]; f(obj);				   \
		}												   \
	}													   \
  }

#define def_for_each_it_non_tabled(name, ...)			\
  template<typename F>									\
  static inline void name(T begin, T end, const F& f){	\
	__VA_ARGS__											\
	  for (auto it=begin; it != end; ++it)				\
		f(*it);											\
  }

  namespace {
	template<typename T, typename Enable = void> class _for_each_it;

	template<typename T>
	class _for_each_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  def_for_each_it_tabled(loop);
#ifdef __ICC
	  def_for_each_it_tabled(vector_loop, _Pragma("vector always"));
	  def_for_each_it_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_for_each_it_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each_it_tabled(simd_loop, _Pragma("simd"));
	  def_for_each_it_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename T>
	class _for_each_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  def_for_each_it_non_tabled(loop);
#ifdef __ICC
	  def_for_each_it_non_tabled(vector_loop, _Pragma("vector always"));
	  def_for_each_it_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_for_each_it_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_for_each_it_non_tabled(simd_loop, _Pragma("simd"));
	  def_for_each_it_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename T, typename F>
  inline void for_each(T begin, T end, const F& f)
  {_for_each_it<T>::loop(begin, end, f);}

#ifdef __ICC
  template<typename T, typename F>
  inline void vector_for_each(T begin, T end, const F& f)
  {_for_each_it<T>::vector_loop(begin, end, f);}

  template<typename T, typename F>
  inline void ivdep_for_each(T begin, T end, const F& f)
  {_for_each_it<T>::ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void vector_ivdep_for_each(T begin, T end, const F& f)
  {_for_each_it<T>::vector_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void simd_for_each(T begin, T end, const F& f)
  {_for_each_it<T>::simd_loop(begin, end, f);}

  template<typename T, typename F>
  inline void novector_for_each(T begin, T end, const F& f)
  {_for_each_it<T>::novector_loop(begin, end, f);}
#endif

}

#endif
