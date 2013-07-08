#include "soa/reference_type.hpp"

#include "aosoa/table_vector.hpp"

#include "aosoa/for_each_range.hpp"
#include "aosoa/indexed_for_each.hpp"

#include <ctime>
#include <iostream>

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

constexpr size_t blocksize = 32;

typedef aosoa::table_vector<Cr,blocksize> cvector;

size_t repeat;
size_t len;

void benchmark (cvector& vec, size_t repeat) {
#ifdef __ICC
  aosoa::vector_indexed_for_each(vec, [](size_t index, Cr& element){
	  element.x = index;
	  element.y = index;
	  element.z = index;
	});
#else
  aosoa::indexed_for_each(vec, [](size_t index, Cr& element){
	  element.x = index;
	  element.y = index;
	  element.z = index;
	});
#endif

  float global = 0;

  auto start = std::time(0);

  for (size_t r = 0; r < repeat; ++r) {

	float local = 0;

	aosoa::for_each_range([&local](size_t start, size_t end,
								   typename soa::table_traits<cvector>::table_reference table){
							float c = 0;
#pragma simd reduction(+:c)
							for (size_t i=start; i<end; ++i) {
							  table[i].x += table[i].y * table[i].z;
							  c += table[i].x;
							}
							local += c;
						  }, vec);

	global += local;
  }

  auto end = std::time(0);

  std::cout << "result: " << global << std::endl;
  std::cout << "time: " << end-start << std::endl;
}

int main(int argc, char *argv[]) {
  len = atoi(argv[1]);
  repeat = atoi(argv[2]);

  std::cout << "len: " << len << std::endl;
  std::cout << "repeat: " << repeat << std::endl;

  cvector vec(len);
  benchmark(vec, repeat);
}
