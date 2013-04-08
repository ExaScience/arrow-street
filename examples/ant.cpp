#include "soa/reference_type.hpp"
#include "soa/table.hpp"

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

  aosoa::ivdep_for_each(a, [](C& e){
	  e.pos.x += e.vel.x;
	  e.pos.y += e.vel.y;
	});
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

  aosoa::indexed_for_each(array, [](size_t index, C& e){
	  e.pos.x = index; e.vel.x = 0.3;
	  e.pos.y = index/2; e.vel.y = 0.5;
	});

  float globalx = 0, globaly = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	nested_update(array);

	float localx = 0, localy = 0;

	aosoa::for_each_range(array, [&](typename soa::table_traits<A>::table_reference t, size_t start, size_t end) {
		float cx = 0, cy = 0;
#pragma simd reduction(+:cx,cy)
		for (size_t i=start; i<end; ++i) {
		  cx += t[i].pos.x;
		  cy += t[i].pos.y;
		}
		localx += cx;
		localy += cy;
	  });

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

void stdAOS() {
  std::cout << "\nstd::array\n";
  std::array<Ant0,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOA1() {
  std::cout << "\nnested SOA array, blocksize 1 (should be same as std::array)\n";
  aosoa::table_array<Ant,1,len> array;
  nested_benchmark(array, repeat);
}

void nestedSOAN() {
  std::cout << "\nnested SOA array, blocksize max (should be same as flat SOA array)\n";
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

void nestedSOV1() {
  std::cout << "\nnested SOA vector, blocksize 1 (should be same as std::vector)\n";
  aosoa::table_vector<Ant,1> array(len);
  nested_benchmark(array, repeat);
}

void nestedSOVN() {
  std::cout << "\nnested SOA vector, blocksize max (should be same as flat SOA array)\n";
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

  stdAOS();
  nestedSOA1();
  nestedSOAN();
  nestedSOAB();

  stdVOS();
  nestedSOV1();
  nestedSOVN();
  nestedSOVB();
}
