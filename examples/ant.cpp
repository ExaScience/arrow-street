/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
#include "aosoa/for_each_range.hpp"
#include "aosoa/indexed_for_each.hpp"

#include <ctime>
#include <iostream>

struct Point0 {
  float x, y;
};

struct Ant0 {
  Point0 pos, vel;
};

struct Point {
  float &x, &y;

  typedef soa::reference_type<float, float> reference;

  Point(const reference::type& ref) :
	x(reference::get<0>(ref)),
	y(reference::get<1>(ref))
  {}
};

struct Ant {
  Point pos, vel;

  typedef soa::reference_type<Point,Point> reference;

  Ant(const reference::type& ref) :
	pos(reference::match<0>(ref)),
	vel(reference::match<1>(ref))
  {}
};

template<typename A> void flat_update(A& a, int n) {
#pragma ivdep
  for (int i=0; i<n; ++i) {
	a[i].pos.x += a[i].vel.x;
	a[i].pos.y += a[i].vel.y;
  }
}

template<typename A> void nested_update(A& a) {
  typedef decltype(a[0]) C;

#ifdef __ICC
  aosoa::ivdep_for_each([](C& e){
	  e.pos.x += e.vel.x;
	  e.pos.y += e.vel.y;
	}, a);
#else
  aosoa::for_each([](C& e){
	  e.pos.x += e.vel.x;
	  e.pos.y += e.vel.y;
	}, a);
#endif
}

constexpr size_t len = 100000;
constexpr size_t blocksize = 32;

size_t repeat;

template<typename A> void flat_benchmark(A& array, size_t len, size_t repeat) {
  for (size_t i=0; i<len; ++i) {
	array[i].pos.x = i; array[i].vel.x = 0.3;
	array[i].pos.y = i/2; array[i].vel.y = 0.5;
  }

  float globalx = 0, globaly = 0;

  auto start = std::time(0);

  for (size_t r=0; r < repeat; ++r) {

	flat_update(array, len);

	float localx = 0, localy = 0;

#pragma simd reduction(+:localx,localy)
	for (size_t i=0; i<len; ++i) {
	  localx += array[i].pos.x;
	  localy += array[i].pos.y;
	}

	globalx += localx;
	globaly += localy;
  }

  auto end = std::time(0);

  std::cout << "result: " << globalx << ", " << globaly << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

template<typename A> void nested_benchmark (A& array, size_t repeat) {
  typedef decltype(array[0]) C;

  aosoa::indexed_for_each([](size_t index, C& e){
	  e.pos.x = index; e.vel.x = 0.3;
	  e.pos.y = index/2; e.vel.y = 0.5;
	}, array);

  float globalx = 0, globaly = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	nested_update(array);

	float localx = 0, localy = 0;

	aosoa::for_each_range([&](size_t start, size_t end,
							  typename soa::table_traits<A>::table_reference t) {
							float cx = 0, cy = 0;
#pragma simd reduction(+:cx,cy)
							for (size_t i=start; i<end; ++i) {
							  cx += t[i].pos.x;
							  cy += t[i].pos.y;
							}
							localx += cx;
							localy += cy;
						  },
						  array);

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
  Ant0 array[len];
  flat_benchmark(array, len, repeat);
}

void flatSOA() {
  std::cout << "\nflat SOA array\n";
  soa::table<Ant,len> array;
  flat_benchmark(array, len, repeat);
}

void flatDSOA() {
  std::cout << "\nflat dynamic SOA array\n";
  soa::dtable<Ant> array(len);
  flat_benchmark(array, len, repeat);
}

void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<Ant0,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOA2() {
  std::cout << "\nnested SOA array, blocksize 2\n";
  aosoa::table_array<Ant,2,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max\n";
  aosoa::table_array<Ant,len,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAB() {
  std::cout << "\nnested SOA array, blocksize " << blocksize << std::endl;
  aosoa::table_array<Ant,blocksize,len> array;
  nested_benchmark(array, repeat);
}

void stdVOS() {
  std::cout << "\nstd::vector\n";
  std::vector<Ant0> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOV2() {
  std::cout << "\nnested SOA vector, blocksize 2\n";
  aosoa::table_vector<Ant,2> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max\n";
  aosoa::table_vector<Ant,len> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVB() {
  std::cout << "\nnested SOA vector, blocksize " << blocksize << std::endl;
  aosoa::table_vector<Ant,blocksize> array(len);
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

#endif
