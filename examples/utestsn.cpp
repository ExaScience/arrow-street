#include "soa/reference_type.hpp"
#include "soa/table.hpp"

#include "aosoa/table_array.hpp"
#include "aosoa/table_vector.hpp"

#include "aosoa/for_each.hpp"
#include "aosoa/for_each_range.hpp"
#include "aosoa/indexed_for_each.hpp"
#include "aosoa/indexed_for_each_range.hpp"

#include "aosoa/parallel_for_each.hpp"
#include "aosoa/parallel_for_each_range.hpp"
#include "aosoa/parallel_indexed_for_each.hpp"
#include "aosoa/parallel_indexed_for_each_range.hpp"

#include <array>
#include <vector>

#include <iostream>

#include "tbb/atomic.h"

#define NO_ITERATORS

class C {
public:
  size_t x, y, z;
};

class Cref {
public:
  size_t &x, &y, &z;

  typedef soa::reference_type<size_t,size_t,size_t> reference;

  Cref(const reference::type& ref) :
	x(reference::get<0>(ref)),
	y(reference::get<1>(ref)),
	z(reference::get<2>(ref))
  {}
};

template<class C> void test(C& container) {
  typedef decltype(container[0]) value_type;
  tbb::atomic<size_t> result;

  std::cout << "for each over containers:                ";

  aosoa::indexed_for_each(container, [](size_t index, value_type& value) {
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::for_each(container, [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result << std::endl;

#ifndef NO_ITERATORS

  std::cout << "for each over iterators:                 ";

  aosoa::indexed_for_each(container.begin(), container.end(), [](size_t index, value_type& value){
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::for_each(container.begin(), container.end(), [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result << std::endl;

#endif

  std::cout << "for each range over containers:          ";

  aosoa::indexed_for_each_range(container, [](typename soa::table_traits<C>::table_reference table, size_t start, size_t end, size_t offset) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::for_each_range(container, [&result](typename soa::table_traits<C>::table_reference table, size_t start, size_t end){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result << std::endl;

#ifndef NO_ITERATORS

  std::cout << "for each range over iterators:           ";

  aosoa::indexed_for_each_range(container.begin(), container.end(), [](typename soa::table_traits<C>::table_reference table, size_t start, size_t end, size_t offset) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::for_each_range(container.begin(), container.end(), [&result](typename soa::table_traits<C>::table_reference table, size_t start, size_t end){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result << std::endl;

#endif

  std::cout << "parallel for each over containers:       ";

  aosoa::parallel_indexed_for_each(container, [](size_t index, value_type& value) {
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::parallel_for_each(container, [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result << std::endl;

#ifndef NO_ITERATORS

  std::cout << "parallel for each over iterators:        ";

  aosoa::parallel_indexed_for_each(container.begin(), container.end(), [](size_t index, value_type& value){
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::parallel_for_each(container.begin(), container.end(), [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result << std::endl;

#endif

  std::cout << "parallel for each range over containers: ";

  aosoa::parallel_indexed_for_each_range(container, [](typename soa::table_traits<C>::table_reference table, size_t start, size_t end, size_t offset) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::parallel_for_each_range(container, [&result](typename soa::table_traits<C>::table_reference table, size_t start, size_t end){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result << std::endl;

#ifndef NO_ITERATORS

  std::cout << "parallel for each range over iterators:  ";

  aosoa::parallel_indexed_for_each_range(container.begin(), container.end(), [](typename soa::table_traits<C>::table_reference table, size_t start, size_t end, size_t offset) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::parallel_for_each_range(container.begin(), container.end(), [&result](typename soa::table_traits<C>::table_reference table, size_t start, size_t end){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result << std::endl;

#endif

}

constexpr size_t len = 100;
constexpr size_t tablesize = 16;

void stdAOS() {
  std::array<C,len> array;
  std::cout << "\nstandard array\n";
  test(array);
}

#ifdef NO_ITERATORS
void flatAOS() {
  soa::table<Cref,len> array;
  std::cout << "\nflat SOA arary\n";
  test(array);
}
#endif

void nestedSOA1() {
  aosoa::table_array<Cref,1,len> array;
  std::cout << "\ntable array with table size " << 1 << std::endl;
  test(array);
}

void nestedSOAN() {
  aosoa::table_array<Cref,len,len> array;
  std::cout << "\ntable array with table size " << len << std::endl;
  test(array);
}

void nestedSOAB() {
  aosoa::table_array<Cref,tablesize,len> array;
  std::cout << "\ntable array with table size " << tablesize << std::endl;
  test(array);
}

void  stdVOS() {
  std::vector<C> array(len);
  std::cout << "\nstandard vector\n";
  test(array);
}

void nestedSOV1() {
  aosoa::table_vector<Cref,1> array(len);
  std::cout << "\ntable vector with table size " << 1 << std::endl;
  test(array);
}

void nestedSOVN() {
  aosoa::table_vector<Cref,len> array(len);
  std::cout << "\ntable vector with table size " << len << std::endl;
  test(array);
}

void nestedSOVB() {
  aosoa::table_vector<Cref,tablesize> array(len);
  std::cout << "\ntable vector with table size " << tablesize << std::endl;
  test(array);
}

int main() {
  stdAOS();
#ifdef NO_ITERATORS
  flatAOS();
#endif
  nestedSOA1();
  nestedSOAN();
  nestedSOAB();

  stdVOS();
  nestedSOV1();
  nestedSOVN();
  nestedSOVB();
}
