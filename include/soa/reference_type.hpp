/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SOA_REFERENCE_TYPE
#define SOA_REFERENCE_TYPE

#include <tuple>
#include <type_traits>

namespace soa {

#ifndef NVARIADIC

  namespace {
	// support for complex data structures

	// how many leaf types are in a tuple-based tree?

	template<typename T, typename Enable = void> struct count_ref;

	template<typename T>
	struct count_ref<T, typename std::enable_if<
						  std::is_scalar<
							typename std::remove_reference<T>::type>::value>::type> {
	  static constexpr int count = 1;
	};

	template<> struct count_ref<std::tuple<>> {
	  static constexpr int count = 0;
	};

	template<typename Head, typename... Tail>
	struct count_ref<std::tuple<Head, Tail...>> {
	  static constexpr int count =
		count_ref<Head>::count +
		count_ref<std::tuple<Tail...>>::count;
	};

	// construct the flattened reference type

	template<typename T, typename Enable = void> struct get_ref;

	template<typename T>
	struct get_ref<T, typename std::enable_if<std::is_scalar<T>::value>::type> {
	  typedef std::tuple<typename std::add_lvalue_reference<T>::type> type;
	  static constexpr int count = 1;
	};

	template<typename T>
	struct get_ref<T, typename std::enable_if<std::is_class<T>::value>::type> {
	  typedef typename T::reference::type type;
	  static constexpr int count = count_ref<type>::count;
	};

	// extract a subrange from a tuple

	template<int I, int J, typename T> struct tuple_range {
	  typedef tuple_range<I+1,J,T> next_range;
	  typedef decltype(std::tuple_cat(std::tie(std::get<I>(std::declval<T>())),
									  std::declval<typename next_range::result_type>())) result_type;

	  inline static result_type get(const T& tuple) {
		return std::tuple_cat(std::tie(std::get<I>(tuple)), next_range::get(tuple));
	  }
	};

	template<int I, typename T> struct tuple_range<I,I,T> {
	  typedef std::tuple<> result_type;

	  inline static std::tuple<> get(const T&) {
		return std::tie();
	  }
	};

	// match an entry in a flattened tuple tree against
	// the corresponding entry in the structured tuple tree

	template<int N, int Offset, typename T, typename Tuple> struct tuple_match;

	template<int N, int Offset, typename T, typename Head, typename... Tail>
	struct tuple_match<N, Offset, T, std::tuple<Head, Tail...>> {
	  typedef tuple_match<N-1, Offset+get_ref<Head>::count, T, std::tuple<Tail...>> next_match;
	  typedef decltype(next_match::get(std::declval<T>())) result_type;

	  inline static result_type get(const T& tuple) {
		return next_match::get(tuple);
	  }
	};

	template<int Offset, typename Head, typename T, typename... Tail>
	struct tuple_match<0, Offset, T, std::tuple<Head, Tail...>> {
	  typedef tuple_range<Offset, Offset+get_ref<Head>::count,T> match_range;
	  typedef decltype(match_range::get(std::declval<T>())) result_type;

	  inline static result_type get(const T& tuple) {
		return match_range::get(tuple);
	  }
	};
  }

  // user support for defining reference tuples

  template<typename... T> struct reference_type;

  template<> struct reference_type<> {
	typedef std::tuple<> type;

	template<int> inline static type match(const type&) {
	  return std::tie();
	}
  };

  template<typename Head, typename... Tail>
  struct reference_type<Head, Tail...> {
	typedef decltype(std::tuple_cat(std::declval<typename get_ref<Head>::type>(),
									std::declval<typename reference_type<Tail...>::type>())) type;

	typedef std::tuple<Head, Tail...> _tuple;

	template<int N> inline static auto match(const type& tuple)
	  -> decltype(tuple_match<N, 0, type, _tuple>::get(tuple))
	{
	  return tuple_match<N, 0, type, _tuple>::get(tuple);
	}

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<0>(match<N>(tuple)))
	{
	  return std::get<0>(match<N>(tuple));
	}
  };

#else

  template<typename T0,
		   typename T1 = void,
		   typename T2 = void,
		   typename T3 = void,
		   typename T4 = void> struct reference_type;

  template<typename T0> struct reference_type<T0> {
	typedef std::tuple<T0&> type;

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<N>(tuple))
	{
	  return std::get<N>(tuple);
	}
  };

  template<typename T0,
		   typename T1> struct reference_type<T0,T1> {
	typedef std::tuple<T0&, T1&> type;

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<N>(tuple))
	{
	  return std::get<N>(tuple);
	}
  };

  template<typename T0,
		   typename T1,
		   typename T2> struct reference_type<T0,T1,T2> {
	typedef std::tuple<T0&, T1&, T2&> type;

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<N>(tuple))
	{
	  return std::get<N>(tuple);
	}
  };

  template<typename T0,
		   typename T1,
		   typename T2,
		   typename T3> struct reference_type<T0,T1,T2,T3> {
	typedef std::tuple<T0&, T1&, T2&, T3&> type;

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<N>(tuple))
	{
	  return std::get<N>(tuple);
	}
  };

  template<typename T0,
		   typename T1,
		   typename T2,
		   typename T3,
		   typename T4> struct reference_type {
	typedef std::tuple<T0&, T1&, T2&, T3&, T4&> type;

	template<int N> inline static auto get(const type& tuple)
	  -> decltype(std::get<N>(tuple))
	{
	  return std::get<N>(tuple);
	}
  };

#endif

}

#endif
