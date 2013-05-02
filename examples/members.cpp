#ifdef NVARIADIC

#pragma message "inheritance / nesting not supported in the non-variadic version of the SOA library"

#include <iostream>

int main() {
  std::cout << "inheritance / nesting not supported in the non-variadic version of the SOA library\n";
}

#else

#include "soa/reference_type.hpp"
#include "soa/table.hpp"
#include "soa/dtable.hpp"

#include "aosoa/table_array.hpp"
#include "aosoa/table_vector.hpp"

#include "aosoa/for_each.hpp"
#include "aosoa/indexed_for_each.hpp"

#include <ctime>
#include <iostream>

struct A {
  float x, y;
};

struct B {
  A a;
};

struct C {
  float x, y;
  A ca;
  B cb;
};

struct Aref {
  float &x, &y;

  typedef soa::reference_type<float,float> reference;

  inline Aref(const reference::type& tuple) :
	x(reference::get<0>(tuple)),
	y(reference::get<1>(tuple))
  {}
};

struct Bref {
  Aref a;

  typedef Aref::reference reference;

  inline Bref(const reference::type& tuple) :
	a(tuple)
  {}
};

struct Cref {
  float &x, &y;
  Aref ca;
  Bref cb;

  typedef soa::reference_type<float,float,Aref,Bref> reference;

  inline Cref(const reference::type& tuple) :
	x(reference::get<0>(tuple)),
	y(reference::get<1>(tuple)),
	ca(reference::match<2>(tuple)),
	cb(reference::match<3>(tuple))
  {}
};

constexpr size_t len = 100000;
constexpr size_t blocksize = 32;

size_t repeat;

template<typename A> inline void flat_benchmark(A& array, size_t len, size_t repeat) {
  for (size_t i=0; i<len; ++i) {
	auto&& a = array[i]; // hack to work around an ICC bug in combination with dtable
	a.x = i;
	a.y = i+1;
	a.ca.x = i+2;
	a.ca.y = i+3;
	a.cb.a.x = i+4;
	a.cb.a.y = i+5;
  }

  float globalx = 0, globaly = 0;

  auto start = std::time(0);

  for (size_t r=0; r < repeat; ++r) {

	for (size_t i=0; i<len; ++i) {
	  array[i].x += array[i].ca.x * array[i].cb.a.x;
	  array[i].y += array[i].ca.y * array[i].cb.a.y;
	}

	float localx = 0, localy = 0;

	for (size_t i=0; i<len; ++i) {
	  localx += array[i].x;
	  localy += array[i].y;
	}

	globalx += localx;
	globaly += localy;
  }

  auto end = std::time(0);

  std::cout << "result: " << globalx << ", " << globaly << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

template<typename A> inline void nested_benchmark (A& array, size_t repeat) {
  typedef decltype(array[0]) C;

#ifdef __ICC
  aosoa::ivdep_indexed_for_each(array, [](size_t i, C& e){
	  e.x = i;
	  e.y = i+1;
	  e.ca.x = i+2;
	  e.ca.y = i+3;
	  e.cb.a.x = i+4;
	  e.cb.a.y = i+5;
	});
#else
  aosoa::indexed_for_each(array, [](size_t i, C& e){
	  e.x = i;
	  e.y = i+1;
	  e.ca.x = i+2;
	  e.ca.y = i+3;
	  e.cb.a.x = i+4;
	  e.cb.a.y = i+5;
	});
#endif

  float globalx = 0, globaly = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

#ifdef __ICC
	aosoa::ivdep_for_each(array, [](C& e) {
		e.x += e.ca.x * e.cb.a.x;
		e.y += e.ca.y * e.cb.a.y;
	  });
#else
	aosoa::for_each(array, [](C& e) {
		e.x += e.ca.x * e.cb.a.x;
		e.y += e.ca.y * e.cb.a.y;
	  });
#endif

	float localx = 0, localy = 0;

#ifdef __ICC
	aosoa::ivdep_for_each(array, [&](C& e) {
		localx += e.x;
		localy += e.y;
	  });
#else
	aosoa::for_each(array, [&](C& e) {
		localx += e.x;
		localy += e.y;
	  });
#endif

	globalx += localx;
	globaly += localy;
  }

  auto end = std::time(0);

  std::cout << "result: " << globalx << ", " << globaly << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

// enumerate all the cases. main difference is in the local variable declarations.

void flatAOS() {
  std::cout << "\nflat AOS array\n";
  C array[len];
  flat_benchmark(array, len, repeat);
}

void flatSOA() {
  std::cout << "\nflat SOA array\n";
  soa::table<Cref,len> array;
  flat_benchmark(array, len, repeat);
}

void flatDSOA() {
  std::cout << "\nflat dynamic SOA array\n";
  soa::dtable<Cref> array(len);
  flat_benchmark(array, len, repeat);
}

void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<C,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOA1() {
  std::cout << "\nnested SOA array, blocksize 1 (should be same as std::array)\n";
  aosoa::table_array<Cref,1,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_array<Cref,len,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAB() {
  std::cout << "\nnested SOA array, blocksize " << blocksize << std::endl;
  aosoa::table_array<Cref,blocksize,len> array;
  nested_benchmark(array, repeat);
}

void stdVOS() {
  std::cout << "\nstd::vector\n";
  std::vector<C> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOV1() {
  std::cout << "\nnested SOA vector, blocksize 1 (should be same as std::vector)\n";
  aosoa::table_vector<Cref,1> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_vector<Cref,len> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVB() {
  std::cout << "\nnested SOA vector, blocksize " << blocksize << std::endl;
  aosoa::table_vector<Cref,blocksize> array(len);
  nested_benchmark(array, repeat);
}


int main() {
  std::cout << "len: " << len << std::endl;
  std::cout << "repeat: "; std::cin >> repeat;

  flatAOS();
  flatSOA();
  flatDSOA();

  stdAOS();
  nestedSOA1();
  nestedSOAN();
  nestedSOAB();

  stdVOS();
  nestedSOV1();
  nestedSOVN();
  nestedSOVB();
}

#endif
