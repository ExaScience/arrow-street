/// Copyright (c) 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_APPLY_TUPLE
#define AOSOA_APPLY_TUPLE

#include <tuple>

namespace aosoa {

  namespace {
	template<int N> class _apply_tuple;

	template<> class _apply_tuple<0> {
	public:
	  template<typename F, typename T, typename... Args>
	  inline static auto call (F&& f, T&, Args&... args)
		-> decltype(f(args...))
	  {return f(args...);}
	};

	template<int N> class _apply_tuple {
	public:
	  template<typename F, typename T, typename... Args>
	  inline static auto call (F&& f, T& t, Args&... args)
		-> decltype(_apply_tuple<N-1>::call(f, t, std::get<N-1>(t), args...))
	  {return _apply_tuple<N-1>::call(f, t, std::get<N-1>(t), args...);}
	};
  }

  template<typename F, typename... T>
  inline auto apply_tuple(F&& f, std::tuple<T...>&& t)
	-> decltype(_apply_tuple<sizeof...(T)>::call(f, t))
  {return _apply_tuple<sizeof...(T)>::call(f, t);}
}

#endif
