#include "soa/reference_type.hpp"
#include "soa/table.hpp"
#include "soa/dtable.hpp"

#include "aosoa/table_array.hpp"
#include "aosoa/table_vector.hpp"

#include "aosoa/for_each_range.hpp"
#include "aosoa/indexed_for_each.hpp"

#include <ctime>
#include <iostream>

// define a class that can be used for
// plain AOS C arrays, std::array, and std::vector.

class C0 {public: float x, y, z;};

// define a class that can be used for
// flat AOS and nested AOSOA representations.

class Cr {
public:
  float &x, &y, &z;

  // define reference type

  typedef soa::reference_type<float,float,float> reference;

  // convert from reference type to C

  Cr(const reference::type& ref) :
	x(reference::get<0>(ref)),
	y(reference::get<1>(ref)),
	z(reference::get<2>(ref))
  {}

  Cr& operator= (const Cr& that) {
	x = that.x;
	y = that.y;
	z = that.z;
	return *this;
  }
};

// define the benchmarks.

constexpr size_t len = 100000;     // constant for plain C arrays, std::array and aosoa::table_array
constexpr size_t blocksize = 32;   // constant, somewhat arbitrary choice, seems to yield good results for this particular test case

size_t repeat;                 // repeat each benchmark this many times. not constant to prevent optimizations based on this value
							   // (not sure if that is really important)

// define the benchmark for flat AOS and SOA representations.
// no special looping constructs needed, just plain indexing.

template<typename A> void flat_benchmark(A& a0, A& a1, size_t len, size_t repeat) {
  for (size_t i=0; i<len; ++i) {
	a0[i].x = i;
	a0[i].y = i;
	a0[i].z = i;
	a1[i].x = 2*i;
	a1[i].y = 2*i;
	a1[i].z = 2*i;
  }

  float global = 0;

  auto start = std::time(0);

  for (size_t r=0; r<repeat; ++r) {

	float local = 0;

#pragma simd reduction(+:local)
	for (size_t i=0; i<len; ++i) {
	  a0[i].x += a0[i].y * a0[i].z;
	  a1[i].x += a1[i].y * a1[i].z;
	  local += a1[i].x - a0[i].x;
	}

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

// define the benchmark for std::array, std::vector, aosoa::table_array, and aosoa::table_vector.
// special looping constructs used to hide away details of nested looping on
// AOSOA representations.

template<typename A> void nested_benchmark (A& a0, A& a1, size_t repeat) {
  typedef decltype(a0[0]) C;

  aosoa::indexed_for_each([](size_t i, C& e0, C& e1){
	  e0.x = i;
	  e0.y = i;
	  e0.z = i;
	  e1.x = 2*i;
	  e1.y = 2*i;
	  e1.z = 2*i;
	}, a0, a1);

  float global = 0;

  auto start = std::time(0);

  for (size_t r=0; r<repeat; ++r) {

	float local = 0;

	aosoa::for_each_range([&local](size_t start, size_t end,
								   typename soa::table_traits<A>::table_reference t0,
								   typename soa::table_traits<A>::table_reference t1){
							float c = 0;
#pragma simd reduction(+:c)
							for (size_t i=start; i<end; ++i) {
							  t0[i].x += t0[i].y * t0[i].z;
							  t1[i].x += t1[i].y * t1[i].z;
							  c += t1[i].x - t0[i].x;
							}
							local += c;
						  }, a0, a1);

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

// enumerate all the cases. main difference is in the local variable declarations.

void flatAOS() {
  std::cout << "\nflat AOS array\n";
  C0 a0[len], a1[len];
  flat_benchmark(a0, a1, len, repeat);
}

void flatSOA() {
  std::cout << "\nflat SOA array\n";
  soa::table<Cr,len> a0, a1;
  flat_benchmark(a0, a1, len, repeat);
}

void flatDSOA() {
  std::cout << "\nflat dynamic SOA array\n";
  soa::dtable<Cr> a0(len), a1(len);
  flat_benchmark(a0, a1, len, repeat);
}

void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<C0,len> a0, a1;
  nested_benchmark(a0, a1, repeat);
}

void nestedSOA1() {
  std::cout << "\nnested SOA array, blocksize 1 (should be same as std::array)\n";
  aosoa::table_array<Cr,1,len> a0, a1;
  nested_benchmark(a0, a1, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_array<Cr,len,len> a0, a1;
  nested_benchmark(a0, a1, repeat);
}

void nestedSOAB() {
  std::cout << "\nnested SOA array, blocksize " << blocksize << std::endl;
  aosoa::table_array<Cr,blocksize,len> a0, a1;
  nested_benchmark(a0, a1, repeat);
}

void stdVOS() {
  std::cout << "\nstd::vector\n";
  std::vector<C0> a0(len), a1(len);
  nested_benchmark(a0, a1, repeat);
}

void nestedSOV1() {
  std::cout << "\nnested SOA vector, blocksize 1 (should be same as std::vector)\n";
  aosoa::table_vector<Cr,1> a0(len), a1(len);
  nested_benchmark(a0, a1, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_vector<Cr,len> a0(len), a1(len);
  nested_benchmark(a0, a1, repeat);
}

void nestedSOVB() {
  std::cout << "\nnested SOA vector, blocksize " << blocksize << std::endl;
  aosoa::table_vector<Cr,blocksize> a0(len), a1(len);
  nested_benchmark(a0, a1, repeat);
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
