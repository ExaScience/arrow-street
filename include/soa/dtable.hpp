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

#ifndef SOA_DTABLE
#define SOA_DTABLE

#include <cstddef>

#include <tuple>
#include <type_traits>

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
		super(n),
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
	  dtable_base (size_t n = 0) : super(n), field(n) {}

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

	template<typename T0>
	class dtable_base<std::tuple<T0>> {
	private:
	  typedef typename std::remove_reference<T0>::type field_type0;

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

	  inline const std::tuple<T0> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]));
	  }
	};

	template<typename T0, typename T1>
	class dtable_base<std::tuple<T0,T1>> {
	private:
	  typedef typename std::remove_reference<T0>::type field_type0;
	  typedef typename std::remove_reference<T1>::type field_type1;

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

	  inline const std::tuple<T0,T1> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2>
	class dtable_base<std::tuple<T0,T1,T2>> {
	private:
	  typedef typename std::remove_reference<T0>::type field_type0;
	  typedef typename std::remove_reference<T1>::type field_type1;
	  typedef typename std::remove_reference<T2>::type field_type2;

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

	  inline const std::tuple<T0,T1,T2> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]),
						const_cast<const T2>(field2[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3>
	class dtable_base<std::tuple<T0,T1,T2,T3>> {
	private:
	  typedef typename std::remove_reference<T0>::type field_type0;
	  typedef typename std::remove_reference<T1>::type field_type1;
	  typedef typename std::remove_reference<T2>::type field_type2;
	  typedef typename std::remove_reference<T3>::type field_type3;

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
		field0 = new field_type0[n];
		field1 = new field_type1[n];
		field2 = new field_type2[n];
		field3 = new field_type3[n];
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

	  inline const std::tuple<T0,T1,T2,T3> operator[] (size_t pos) const {
		return std::tie(const_cast<const T0>(field0[pos]),
						const_cast<const T1>(field1[pos]),
						const_cast<const T2>(field2[pos]),
						const_cast<const T3>(field3[pos]));
	  }
	};

	template<typename T0, typename T1, typename T2, typename T3, typename T4>
	class dtable_base<std::tuple<T0,T1,T2,T3,T4>> {
	private:
	  typedef typename std::remove_reference<T0>::type field_type0;
	  typedef typename std::remove_reference<T1>::type field_type1;
	  typedef typename std::remove_reference<T2>::type field_type2;
	  typedef typename std::remove_reference<T3>::type field_type3;
	  typedef typename std::remove_reference<T4>::type field_type4;

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

  template<class C>
  class dtable : protected dtable_base<typename C::reference::type> {
  private:
	size_t n;
	typedef dtable_base<typename C::reference::type> super;

  public:
	dtable (size_t n = 0) : super(n), n(n) {}
	void allocate (size_t n) {super::allocate(n);}
	void deallocate () {super::deallocate();}
	inline C operator[] (size_t pos) {return C(super::operator[](pos));}
	inline const C operator[] (size_t pos) const {return C(super::operator[](pos));}
	inline size_t size() const {return n;}
	inline dtable<C>* data() {return this;}
  };

}

#endif
