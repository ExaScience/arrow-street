/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef SOA_TABLE
#define SOA_TABLE

#include <cassert>
#include <cstddef>

#include <tuple>
#include <type_traits>

#include <array>
#include <vector>

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


  template<typename T> class table_traits;

  template<typename T, size_t N> class table_traits<table<T,N>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = N;

	typedef T value_type;
	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
	typedef const table_reference const_table_reference;
  };

  template<typename T, size_t N> class table_traits<std::array<T,N>> {
  public:
	static constexpr auto tabled = false;
	static constexpr auto table_size = 1;

	typedef T value_type;
	typedef typename std::array<T,N>::iterator table_reference;
	typedef typename std::array<T,N>::const_iterator const_table_reference;
  };

  template<typename T> class table_traits<std::vector<T>> {
  public:
	static constexpr auto tabled = false;
	static constexpr auto table_size = 1;

	typedef T value_type;
	typedef typename std::vector<T>::iterator table_reference;
	typedef typename std::vector<T>::const_iterator const_table_reference;
  };
}

#endif
