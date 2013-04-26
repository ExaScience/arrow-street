/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef SOA_DTABLE
#define SOA_DTABLE

#include <cassert>
#include <cstddef>

#include <tuple>
#include <type_traits>

#include <array>
#include <vector>

namespace soa {

  namespace {

	template<typename T, typename Enable = void> class dtable_base;

	template<> class dtable_base<std::tuple<>> {
	public:
	  dtable_base (size_t n = 0) {}
	  ~dtable_base () {}

	  void allocate (size_t n) {}
	  void deallocate () {}

	  inline std::tuple<> operator[](size_t) {return std::tie();}
	  inline const std::tuple<> operator[](size_t) const {return std::tie();}
	};

#ifndef NVARIADIC

	template<typename Head, typename... Tail>
	class dtable_base<std::tuple<Head, Tail...>,
					  typename std::enable_if<
						std::is_scalar<
						  typename std::remove_reference<Head>::type>::value>::type>
	   : protected dtable_base<std::tuple<Tail...>>
	{
	private:
	  typedef dtable_base<std::tuple<Tail...>> super;
	  typedef typename std::remove_reference<Head>::type field_type;

	  field_type* field;

	public:
	  dtable_base (size_t n = 0) :
		field(n?(new field_type[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field;
	  }

	  void allocate (size_t n) {
		super::allocate(n);
		field = new field_type[n];
	  }

	  void deallocate () {
		delete [] field; field = nullptr;
		super::deallocate();
	  }

	  inline std::tuple<Head, Tail...> operator[] (size_t pos) {
		return std::tuple_cat(std::tie(field[pos]), super::operator[](pos));
	  }

	  inline const std::tuple<Head, Tail...> operator[] (size_t pos) const {
		return std::tuple_cat(std::tie(const_cast<const Head>(field[pos])), super::operator[](pos));
	  }
	};

	template<typename Head, typename... Tail>
	class dtable_base<std::tuple<Head, Tail...>,
					  typename std::enable_if<
						std::is_class<Head>::value>::type>
	   : protected dtable_base<std::tuple<Tail...>>
	{
	private:
	  typedef dtable_base<std::tuple<Tail...>> super;
	  dtable_base<typename Head::reference::type> field;

	public:
	  dtable_base (size_t n = 0) : field(n) {}

	  void allocate (size_t n) {
		super::allocate(n);
		field.allocate(n);
	  }

	  void deallocate () {
		field.deallocate();
		super::deallocate();
	  }

	  inline std::tuple<Head, Tail...> operator[] (size_t pos) {
		return std::tuple_cat(field[pos], super::operator[](pos));
	  }

	  inline const std::tuple<Head, Tail...> operator[] (size_t pos) const {
		return std::tuple_cat(field[pos], super::operator[](pos));
	  }
	};

#else

	template<typename T0, size_t N>
	class dtable_base<std::tuple<T0>, N> {
	private:
	  typedef std::remove_reference<T0>::type field_type0;

	  field_type0* field0;

	public:
	  dtable_base (size_t n = 0) :
		field0(n?(new field_type0[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field0;
	  }

	  void allocate (size_t n) {
		field0 = new field_type0[n];
	  }

	  void deallocate () {
		delete [] field0; field0 = nullptr;
	  }

	  inline std::tuple<T0> operator[] (size_t pos) {
		return std::tie(field0[pos]);
	  }
	};

	template<typename T0, typename T1, size_t N>
	class dtable_base<std::tuple<T0,T1>, N> {
	private:
	  typedef std::remove_reference<T0>::type field_type0;
	  typedef std::remove_reference<T1>::type field_type1;

	  field_type0* field0;
	  field_type1* field1;

	public:
	  dtable_base (size_t n = 0) :
		field0(n?(new field_type0[n]):nullptr),
		field1(n?(new field_type1[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field1;
		delete [] field0;
	  }

	  void allocate (size_t n) {
		field0 = new field_type0[n];
		field1 = new field_type1[n];
	  }

	  void deallocate () {
		delete [] field1; field1 = nullptr;
		delete [] field0; field0 = nullptr;
	  }

	  inline std::tuple<T0,T1> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos]);
	  }
	};

	template<typename T0, typename T1, typename T2, size_t N>
	class dtable_base<std::tuple<T0,T1,T2>, N> {
	private:
	  typedef std::remove_reference<T0>::type field_type0;
	  typedef std::remove_reference<T1>::type field_type1;
	  typedef std::remove_reference<T2>::type field_type2;

	  field_type0* field0;
	  field_type1* field1;
	  field_type2* field2;

	public:
	  dtable_base (size_t n = 0) :
		field0(n?(new field_type0[n]):nullptr),
		field1(n?(new field_type1[n]):nullptr),
		field2(n?(new field_type2[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field2;
		delete [] field1;
		delete [] field0;
	  }

	  void allocate (size_t n) {
		field0 = new field_type0[n];
		field1 = new field_type1[n];
		field2 = new field_type2[n];
	  }

	  void deallocate () {
		delete [] field2; field2 = nullptr;
		delete [] field1; field1 = nullptr;
		delete [] field0; field0 = nullptr;
	  }

	  inline std::tuple<T0,T1,T2> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos]);
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3, size_t N>
	class dtable_base<std::tuple<T0,T1,T2,T3>, N> {
	private:
	  typedef std::remove_reference<T0>::type field_type0;
	  typedef std::remove_reference<T1>::type field_type1;
	  typedef std::remove_reference<T2>::type field_type2;
	  typedef std::remove_reference<T3>::type field_type3;

	  field_type0* field0;
	  field_type1* field1;
	  field_type2* field2;
	  field_type3* field3;

	public:
	  dtable_base (size_t n = 0) :
		field0(n?(new field_type0[n]):nullptr),
		field1(n?(new field_type1[n]):nullptr),
		field2(n?(new field_type2[n]):nullptr),
		field3(n?(new field_type3[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field3;
		delete [] field2;
		delete [] field1;
		delete [] field0;
	  }

	  void allocate (size_t n) {
		field0 = field_type0[n];
		field1 = field_type1[n];
		field2 = field_type2[n];
		field3 = field_type3[n];
	  }

	  void deallocate () {
		delete [] field3; field3 = nullptr;
		delete [] field2; field2 = nullptr;
		delete [] field1; field1 = nullptr;
		delete [] field0; field0 = nullptr;
	  }

	  inline std::tuple<T0,T1,T2,T3> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos], field3[pos]);
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3, typename T4, size_t N>
	class dtable_base<std::tuple<T0,T1,T2,T3,T4>, N> {
	private:
	  typedef std::remove_reference<T0>::type field_type0;
	  typedef std::remove_reference<T1>::type field_type1;
	  typedef std::remove_reference<T2>::type field_type2;
	  typedef std::remove_reference<T3>::type field_type3;
	  typedef std::remove_reference<T4>::type field_type4;

	  field_type0* field0;
	  field_type1* field1;
	  field_type2* field2;
	  field_type3* field3;
	  field_type4* field4;

	public:
	  dtable_base (size_t n = 0) :
		field0(n?(new field_type0[n]):nullptr),
		field1(n?(new field_type1[n]):nullptr),
		field2(n?(new field_type2[n]):nullptr),
		field3(n?(new field_type3[n]):nullptr),
		field4(n?(new field_type4[n]):nullptr)
	  {}

	  ~dtable_base () {
		delete [] field4;
		delete [] field3;
		delete [] field2;
		delete [] field1;
		delete [] field0;
	  }

	  void allocate (size_t n) {
		field0 = new field_type0[n];
		field1 = new field_type1[n];
		field2 = new field_type2[n];
		field3 = new field_type3[n];
		field4 = new field_type4[n];
	  }

	  void deallocate () {
		delete [] field4; field4 = nullptr;
		delete [] field3; field3 = nullptr;
		delete [] field2; field2 = nullptr;
		delete [] field1; field1 = nullptr;
		delete [] field0; field0 = nullptr;
	  }

	  inline std::tuple<T0,T1,T2,T3,T4> operator[] (size_t pos) {
		return std::tie(field0[pos], field1[pos], field2[pos], field3[pos], field4[pos]);
	  }
	};

#endif
  }

  template<class C>
  class dtable : protected dtable_base<typename C::reference::type> {
  private:
	size_t n;
	typedef dtable_base<typename C::reference::type> super;

  public:
	dtable (size_t n = 0) : n(n), super(n) {}
	void allocate (size_t n) {super::allocate(n);}
	void deallocate () {super::deallocate();}
	inline C operator[] (size_t pos) {return C(super::operator[](pos));}
	inline const C operator[] (size_t pos) const {return C(super::operator[](pos));}
	inline size_t size() const {return n;}
	inline dtable<C>* data() {return this;}
  };


  /*
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
  */
}

#endif
