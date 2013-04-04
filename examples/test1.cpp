#include "soa/reference_type.hpp"
#include "soa/table.hpp"

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

const size_t len = 100000;     // constant for plain C arrays, std::array and aosoa::table_array
const size_t blocksize = 32;   // constant, somewhat arbitrary choice, seems to yield good results for this particular test case

size_t repeat;                 // repeat each benchmark this many times. not constant to prevent optimizations based on this value
							   // (not sure if that is really important)

// define the benchmark for flat AOS and SOA representations.
// no special looping constructs needed, just plain indexing.

template<typename A> void flat_benchmark(A& array, size_t len, size_t repeat) {
  for (size_t i=0; i<len; ++i) {
	array[i].x = i;
	array[i].y = i;
	array[i].z = i;
  }

  float global = 0;

  auto start = std::time(0);

  for (size_t r=0; r<repeat; ++r) {

	float local = 0;

#pragma simd reduction(+:local)
	for (size_t i=0; i<len; ++i) {
	  array[i].x += array[i].y * array[i].z;
	  local += array[i].x;
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

template<typename A> void nested_benchmark (A& array, size_t repeat) {
  typedef decltype(array[0]) C;

  aosoa::indexed_for_each(array, [](size_t i, C& element){
	  element.x = i;
	  element.y = i;
	  element.z = i;
	});

  float global = 0;

  auto start = std::time(0);

  for (size_t r=0; r<repeat; ++r) {

	float local = 0;

	aosoa::for_each_range(array, [&local](typename soa::table_traits<A>::table_reference table, size_t start, size_t end){
		float c = 0;
#pragma simd reduction(+:c)
		for (size_t i=start; i<end; ++i) {
		  table[i].x += table[i].y * table[i].z;
		  c += table[i].x;
		}
		local += c;
	  });

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

// enumerate all the cases. main difference is in the local variable declarations.

void flatAOS() {
  std::cout << "\nflat AOS array\n";
  C0 array[len];
  flat_benchmark(array, len, repeat);
}

void flatSOA() {
  std::cout << "\nflat SOA array\n";
  soa::table<Cr,len> array;
  flat_benchmark(array, len, repeat);
}

void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<C0,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOA1() {
  std::cout << "\nnested SOA array, blocksize 1 (should be same as std::array)\n";
  aosoa::table_array<Cr,1,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_array<Cr,len,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAB() {
  std::cout << "\nnested SOA array, blocksize " << blocksize << std::endl;
  aosoa::table_array<Cr,blocksize,len> array;
  nested_benchmark(array, repeat);
}

void stdVOS() {
  std::cout << "\nstd::vector\n";
  std::vector<C0> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOV1() {
  std::cout << "\nnested SOA vector, blocksize 1 (should be same as std::vector)\n";
  aosoa::table_vector<Cr,1> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max (should be same as flat SOA array)\n";
  aosoa::table_vector<Cr,len> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVB() {
  std::cout << "\nnested SOA vector, blocksize " << blocksize << std::endl;
  aosoa::table_vector<Cr,blocksize> array(len);
  nested_benchmark(array, repeat);
}


int main() {
  std::cout << "len: " << len << std::endl;
  std::cout << "repeat: "; std::cin >> repeat;

  flatAOS();
  flatSOA();

  stdAOS();
  nestedSOA1();
  nestedSOAN();
  nestedSOAB();

  stdVOS();
  nestedSOV1();
  nestedSOVN();
  nestedSOVB();
}
