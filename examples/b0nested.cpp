#include "soa/reference_type.hpp"

#include "aosoa/table_array.hpp"

#include "aosoa/indexed_for_each.hpp"
#include "aosoa/for_each_range.hpp"

#include <array>
#include <ctime>
#include <iostream>

class C {public: float x, y, z;};

class Cr {
public:
  float &x, &y, &z;

  typedef soa::reference_type<float,float,float> reference;

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

auto const len = 10000;
auto const blocksize = 32;
size_t repeat;

typedef std::array<C,len> barray;
typedef aosoa::table_array<Cr,blocksize,len> carray;

void baseline (barray& array, size_t repeat) {
  std::cout << "baseline" << std::endl;

  for (size_t i=0; i<len; ++i) {
	array[i].x = i;
	array[i].y = i;
	array[i].z = i;
  }

  float global = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	float local = 0;

	for (auto& element: array) {
	  element.x += element.y * element.z;
	  local += element.x;
	}

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

void benchmark0 (carray& array, size_t repeat) {
  std::cout << "benchmark 0" << std::endl;

  aosoa::ivdep_indexed_for_each(array, [](size_t index, Cr& element){
	  element.x = index;
	  element.y = index;
	  element.z = index;
	});

  float global = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	float local = 0;

	aosoa::for_each_range(array, [&local](typename soa::table_traits<carray>::table_reference table, size_t start, size_t end){
#pragma simd reduction(+:local)
		for (size_t i=start; i<end; ++i) {
		  table[i].x += table[i].y * table[i].z;
		  local += table[i].x;
		}
	  });

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

void benchmark1 (carray& array, size_t repeat) {
  std::cout << "benchmark 1" << std::endl;

  aosoa::ivdep_indexed_for_each(array, [](size_t index, Cr& element){
	  element.x = index;
	  element.y = index;
	  element.z = index;
	});

  float global = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	aosoa::for_each_range(array, [&global](typename soa::table_traits<carray>::table_reference table, size_t start, size_t end){
		float local = 0;
#pragma simd reduction(+:local)
		for (size_t i=start; i<end; ++i) {
		  table[i].x += table[i].y * table[i].z;
		  local += table[i].x;
		}
		global += local;
	  });
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

void benchmark2 (carray& array, size_t repeat) {
  std::cout << "benchmark 2" << std::endl;

  aosoa::ivdep_indexed_for_each(array, [](size_t index, Cr& element){
	  element.x = index;
	  element.y = index;
	  element.z = index;
	});

  float global = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	float local = 0;

	aosoa::for_each_range(array, [&local](typename soa::table_traits<carray>::table_reference table, size_t start, size_t end){
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

int main(int argc, char *argv[]) {
  repeat = atoi(argv[1]);

  std::cout << "len: " << len << std::endl;
  std::cout << "repeat: " << repeat << std::endl;

  { barray array;
	baseline(array, repeat); }

  { carray array;
	benchmark0(array, repeat);
	benchmark1(array, repeat);
	benchmark2(array, repeat);}
}
