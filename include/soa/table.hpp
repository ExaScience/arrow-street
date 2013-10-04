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

#ifndef SOA_TABLE
#define SOA_TABLE

#include <cstddef>

#include <tuple>
#include <type_traits>

namespace soa {

  namespace {

	template<typename T, size_t N, typename Enable = void> class table_base;

	template<size_t N> class table_base<std::tuple<>, N> {
	public:
	  inline std::tuple<> operator[](size_t) {return std::tie();}
	  inline const std::tuple<> operator[](size_t) const {return std::tie();}
	};

#ifndef NVARIADIC

	template<typename Head, typename... Tail, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<Head, Tail...>, N,
			   typename std::enable_if<
				 std::is_scalar<
				   typename std::remove_reference<Head>::type>::value>::type>
			: protected table_base<std::tuple<Tail...>, N>
	{
	private:
	  typedef table_base<std::tuple<Tail...>, N> super;
	  typename std::remove_reference<Head>::type field[N];

	public:
	  inline std::tuple<Head, Tail...> operator[] (size_t pos) {
		return std::tuple_cat(std::tie(field[pos]), super::operator[](pos));
	  }

	  inline const std::tuple<Head, Tail...> operator[] (size_t pos) const {
		return std::tuple_cat(std::tie(const_cast<const Head>(field[pos])), super::operator[](pos));
	  }
	};

	template<typename Head, typename... Tail, size_t N>
	class table_base<std::tuple<Head, Tail...>, N,
					 typename std::enable_if<
					   std::is_class<Head>::value>::type>
		  : protected table_base<std::tuple<Tail...>, N>
	{
	private:
	  typedef table_base<std::tuple<Tail...>, N> super;
	  table_base<typename Head::reference::type, N> field;

	public:
	  inline std::tuple<Head, Tail...> operator[] (size_t pos) {
		return std::tuple_cat(field[pos], super::operator[](pos));
	  }

	  inline const std::tuple<Head, Tail...> operator[] (size_t pos) const {
		return std::tuple_cat(field[pos], super::operator[](pos));
	  }
	};

#else

	template<typename T0, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<T0>, N> {
	private:
	  typename std::remove_reference<T0>::type field0[N];

	public:
	  inline std::tuple<T0> operator[] (size_t pos) {
		return std::tie(field0[pos]);
	  }

	  inline const std::tuple<T0> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]));
	  }
	};

	template<typename T0, typename T1, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<T0,T1>, N> {
	private:
	  typename std::remove_reference<T0>::type field0[N];
	  typename std::remove_reference<T1>::type field1[N];

	public:
	  inline std::tuple<T0,T1> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos]);
	  }

	  inline const std::tuple<T0,T1> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<T0,T1,T2>, N> {
	private:
	  typename std::remove_reference<T0>::type field0[N];
	  typename std::remove_reference<T1>::type field1[N];
	  typename std::remove_reference<T2>::type field2[N];

	public:
	  inline std::tuple<T0,T1,T2> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos]);
	  }

	  inline const std::tuple<T0,T1,T2> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]),
						const_cast<const T2>(field2[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<T0,T1,T2,T3>, N> {
	private:
	  typename std::remove_reference<T0>::type field0[N];
	  typename std::remove_reference<T1>::type field1[N];
	  typename std::remove_reference<T2>::type field2[N];
	  typename std::remove_reference<T3>::type field3[N];

	public:
	  inline std::tuple<T0,T1,T2,T3> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos], field3[pos]);
	  }

	  inline const std::tuple<T0,T1,T2,T3> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]),
						const_cast<const T2>(field2[pos]),
						const_cast<const T3>(field3[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3, typename T4, size_t N> class
#ifdef __ICC
#ifdef __MIC__
	__declspec(align(64))
#else
	__declspec(align(32))
#endif
#endif
	table_base<std::tuple<T0,T1,T2,T3,T4>, N> {
	private:
	  typename std::remove_reference<T0>::type field0[N];
	  typename std::remove_reference<T1>::type field1[N];
	  typename std::remove_reference<T2>::type field2[N];
	  typename std::remove_reference<T3>::type field3[N];
	  typename std::remove_reference<T4>::type field4[N];

	public:
	  inline std::tuple<T0,T1,T2,T3,T4> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos], field3[pos], field4[pos]);
	  }

	  inline const std::tuple<T0,T1,T2,T3,T4> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]),
						const_cast<const T2>(field2[pos]),
						const_cast<const T3>(field3[pos]),
						const_cast<const T4>(field4[pos]));
	  }
	};

#endif
  }

  template<class C, size_t N>
  class table : protected table_base<typename C::reference::type, N> {
  private:
	typedef table_base<typename C::reference::type, N> super;

  public:
	inline C operator[] (size_t pos) {return C(super::operator[](pos));}
	inline const C operator[] (size_t pos) const {return C(super::operator[](pos));}
	inline size_t size() const {return N;}
	inline table<C,N>* data() {return this;}
  };


  template<class C>
  class singleton_table : protected table_base<typename C::reference::type, 1> {
  private:
	typedef table_base<typename C::reference::type, 1> super;

  public:
	inline C operator() () {return C(super::operator[](0));}
	inline const C operator() () const {return C(super::operator[](0));}
  };

}

#endif
