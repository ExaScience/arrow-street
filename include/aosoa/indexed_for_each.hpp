/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_INDEXED_FOR_EACH
#define AOSOA_INDEXED_FOR_EACH

#include <cstddef>
#include <type_traits>

#include "aosoa/table_iterator.hpp"

namespace aosoa {

#define def_indexed_for_each_tabled(name, ...)		  \
  template<typename F>								  \
  static inline void name(C& container, const F& f){  \
	const auto size = container.size();				  \
	auto data = container.data();					  \
	const auto sdb = size/traits::table_size;		  \
	const auto smb = size%traits::table_size;		  \
													  \
	for (size_t i=0; i<sdb; ++i) {					  \
	  auto& t = data[i];							  \
	  const auto offset = i*traits::table_size;		  \
	  __VA_ARGS__									  \
		for (size_t j=0; j<traits::table_size; ++j) { \
		  auto obj = t[j]; f(offset+j, obj);		  \
		}											  \
	}												  \
	if (smb) {										  \
	  auto& t = data[sdb];							  \
	  const auto offset = sdb*traits::table_size;	  \
	  __VA_ARGS__									  \
		for (size_t j=0; j<smb; ++j) {				  \
		  auto obj = t[j]; f(offset+j,obj);			  \
		}											  \
	}												  \
  }

#define def_indexed_for_each_non_tabled(name, ...)	 \
  template<typename F>								 \
  static inline void name(C& container, const F& f){ \
	const auto begin = container.begin();			 \
	const auto end = container.end();				 \
	__VA_ARGS__										 \
	  for (auto it=begin; it != end; ++it)			 \
		f(it-begin,*it);							 \
  }


  namespace {
	template<class C, typename Enable = void> class _indexed_for_each;

	template<class C>
	class _indexed_for_each<C, typename std::enable_if<soa::table_traits<C>::tabled>::type> {
	private:
	  typedef soa::table_traits<C> traits;
	public:
	  def_indexed_for_each_tabled(loop);
#ifdef __ICC
	  def_indexed_for_each_tabled(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each_tabled(simd_loop, _Pragma("simd"));
	  def_indexed_for_each_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename C>
	class _indexed_for_each<C, typename std::enable_if<!soa::table_traits<C>::tabled>::type> {
	public:
	  def_indexed_for_each_non_tabled(loop);
#ifdef __ICC
	  def_indexed_for_each_non_tabled(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each_non_tabled(simd_loop, _Pragma("simd"));
	  def_indexed_for_each_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<class C, typename F>
  inline void indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::loop(container, f);}

#ifdef __ICC
  template<class C, typename F>
  inline void vector_indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::vector_loop(container, f);}

  template<class C, typename F>
  inline void ivdep_indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::ivdep_loop(container, f);}

  template<class C, typename F>
  inline void vector_ivdep_indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::vector_ivdep_loop(container, f);}

  template<class C, typename F>
  inline void simd_indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::simd_loop(container, f);}

  template<class C, typename F>
  inline void novector_indexed_for_each(C& container, const F& f)
  {_indexed_for_each<C>::novector_loop(container, f);}
#endif


#define def_indexed_for_each_it_tabled(name, ...)		   \
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
		  auto obj = table0[0][j]; f(j-index0, obj);	   \
		}												   \
	  const auto range = tablen-table0;					   \
	  for (ptrdiff_t i=1; i<range; ++i) {				   \
		const auto offset = i*traits::table_size-index0;   \
		__VA_ARGS__										   \
		  for (size_t j=0; j<traits::table_size; ++j) {	   \
			auto obj = table0[i][j]; f(offset+j, obj);	   \
		  }												   \
	  }													   \
	  const auto offset = range*traits::table_size-index0; \
	  __VA_ARGS__										   \
		for (size_t j=0; j<indexn; ++j) {				   \
		  auto obj = tablen[0][j]; f(offset+j, obj);	   \
		}												   \
	} else if (table0 == tablen) {						   \
	  __VA_ARGS__										   \
		for (size_t j=index0; j<indexn; ++j) {			   \
		  auto obj = table0[0][j]; f(j-index0, obj);	   \
		}												   \
	}													   \
  }

#define def_indexed_for_each_it_non_tabled(name, ...)	\
  template<typename F>									\
  static inline void name(T begin, T end, const F& f){	\
	__VA_ARGS__											\
	  for (auto it=begin; it != end; ++it)				\
		f(it-begin, *it);								\
  }

  namespace {
	template<typename T, typename Enable = void> class _indexed_for_each_it;

	template<typename T>
	class _indexed_for_each_it<T, typename std::enable_if<table_iterator_traits<T>::tabled>::type> {
	private:
	  typedef table_iterator_traits<T> traits;
	public:
	  def_indexed_for_each_it_tabled(loop);
#ifdef __ICC
	  def_indexed_for_each_it_tabled(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each_it_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each_it_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each_it_tabled(simd_loop, _Pragma("simd"));
	  def_indexed_for_each_it_tabled(novector_loop, _Pragma("novector"));
#endif
	};

	template<typename T>
	class _indexed_for_each_it<T, typename std::enable_if<!table_iterator_traits<T>::tabled>::type> {
	public:
	  def_indexed_for_each_it_non_tabled(loop);
#ifdef __ICC
	  def_indexed_for_each_it_non_tabled(vector_loop, _Pragma("vector always"));
	  def_indexed_for_each_it_non_tabled(ivdep_loop, _Pragma("ivdep"));
	  def_indexed_for_each_it_non_tabled(vector_ivdep_loop, _Pragma("ivdep") _Pragma("vector always"));
	  def_indexed_for_each_it_non_tabled(simd_loop, _Pragma("simd"));
	  def_indexed_for_each_it_non_tabled(novector_loop, _Pragma("novector"));
#endif
	};
  }

  template<typename T, typename F>
  inline void indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::loop(begin, end, f);}

#ifdef __ICC
  template<typename T, typename F>
  inline void vector_indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::vector_loop(begin, end, f);}

  template<typename T, typename F>
  inline void ivdep_indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void vector_ivdep_indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::vector_ivdep_loop(begin, end, f);}

  template<typename T, typename F>
  inline void simd_indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::simd_loop(begin, end, f);}

  template<typename T, typename F>
  inline void novector_indexed_for_each(T begin, T end, const F& f)
  {_indexed_for_each_it<T>::novector_loop(begin, end, f);}
#endif

}

#endif
