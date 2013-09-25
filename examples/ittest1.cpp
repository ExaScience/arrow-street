/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "soa/reference_type.hpp"
#include "soa/table.hpp"
#include "soa/dtable.hpp"

#include "aosoa/table_array.hpp"
#include "aosoa/table_vector.hpp"

#include "aosoa/for_each_range.hpp"
#include "aosoa/indexed_for_each.hpp"
#include "aosoa/parallel_for_each_range.hpp"
#include "aosoa/parallel_indexed_for_each.hpp"

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

  aosoa::parallel_indexed_for_each(array.begin(), array.end(), [](size_t i, C& element){
	  element.x = i;
	  element.y = i;
	  element.z = i;
	});

  float global = 0;

  auto start = std::time(0);

  for (size_t r=0; r<repeat; ++r) {

	float local = 0;

	aosoa::parallel_for_each_range
	  (array.begin(), array.end(),
	   [&local](size_t start, size_t end,
				typename soa::table_traits<A>::table_reference table){
#pragma simd
		for (size_t i=start; i<end; ++i) {
		  table[i].x += table[i].y * table[i].z;
		}
	  });

	aosoa::for_each_range(array.begin(), array.end(), [&local](size_t start, size_t end, typename soa::table_traits<A>::table_reference table){
		float c = 0;
#pragma simd reduction(+:c)
		for (size_t i=start; i<end; ++i) {
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

void flatDSOA() {
  std::cout << "\nflat dynamic SOA array\n";
  soa::dtable<Cr> array(len);
  flat_benchmark(array, len, repeat);
}

  void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<C0,len> array;
  nested_benchmark(array, repeat);
  }

void nestedSOA2() {
  std::cout << "\nnested SOA array, blocksize 2\n";
  aosoa::table_array<Cr,2,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max\n";
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

void nestedSOV2() {
  std::cout << "\nnested SOA vector, blocksize 2\n";
  aosoa::table_vector<Cr,2> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max\n";
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
  flatDSOA();

  stdAOS();
  nestedSOA2();
  nestedSOAN();
  nestedSOAB();

  stdVOS();
  nestedSOV2();
  nestedSOVN();
  nestedSOVB();
}
