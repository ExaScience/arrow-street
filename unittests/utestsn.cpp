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

#include "soa/reference_type.hpp"
#include "soa/table.hpp"
#include "soa/dtable.hpp"

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

template<class C> bool test(C& container) {
  bool all_fine = true;

  typedef decltype(container[0]) value_type;
  tbb::atomic<size_t> result;

  std::cout << "for each over containers:                ";

  aosoa::indexed_for_each([](size_t index, value_type& value) {
	  value.x = index;
	  value.y = index;
	  value.z = index;
	}, container);

  result = 0;

  aosoa::for_each([&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "for each over iterators:                 ";

  aosoa::indexed_for_each
	(container.begin(), container.end(),
	 [](size_t index, value_type& value){
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::for_each
	(container.begin(), container.end(),
	 [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "for each range over containers:          ";

  aosoa::indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	}, container);

  result = 0;

  aosoa::for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "for each range over iterators:           ";

  aosoa::indexed_for_each_range
	(container.begin(), container.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::for_each_range
	(container.begin(), container.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "parallel for each over containers:       ";

  aosoa::parallel_indexed_for_each([](size_t index, value_type& value) {
	  value.x = index;
	  value.y = index;
	  value.z = index;
	}, container);

  result = 0;

  aosoa::parallel_for_each([&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "parallel for each over iterators:        ";

  aosoa::parallel_indexed_for_each
	(container.begin(), container.end(),
	 [](size_t index, value_type& value){
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::parallel_for_each
	(container.begin(), container.end(),
	 [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "parallel for each range over containers: ";

  aosoa::parallel_indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	}, container);

  result = 0;

  aosoa::parallel_for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "parallel for each range over iterators:  ";

  aosoa::parallel_indexed_for_each_range
	(container.begin(), container.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::parallel_for_each_range
	(container.begin(), container.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifdef __cilk

  std::cout << "cilk_parallel for each over containers:       ";

  aosoa::cilk_parallel_indexed_for_each([](size_t index, value_type& value) {
	  value.x = index;
	  value.y = index;
	  value.z = index;
	}, container);

  result = 0;

  aosoa::cilk_parallel_for_each([&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "cilk_parallel for each over iterators:        ";

  aosoa::cilk_parallel_indexed_for_each
	(container.begin(), container.end(),
	 [](size_t index, value_type& value){
	  value.x = index;
	  value.y = index;
	  value.z = index;
	});

  result = 0;

  aosoa::cilk_parallel_for_each
	(container.begin(), container.end(),
	 [&result](value_type& value){
	  value.x += value.y + value.z;
	  result += value.x;
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "cilk_parallel for each range over containers: ";

  aosoa::cilk_parallel_indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	}, container);

  result = 0;

  aosoa::cilk_parallel_for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	}, container);

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "cilk_parallel for each range over iterators:  ";

  aosoa::cilk_parallel_indexed_for_each_range
	(container.begin(), container.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C>::table_reference table) {
	  for (size_t i=start; i<end; ++i) {
		table[i].x = offset+i;
		table[i].y = offset+i;
		table[i].z = offset+i;
	  }
	});

  result = 0;

  aosoa::cilk_parallel_for_each_range
	(container.begin(), container.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C>::table_reference table){
	  for (size_t i=start; i<end; ++i) {
		table[i].x += table[i].y + table[i].z;
		result += table[i].x;
	  }
	});

  std::cout << result;
  if (result == 14850) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#endif

  return all_fine;

}

template<class C0, class C1, class C2>
bool testmulti(C0& c0, C1& c1, C2& c2) {
  bool all_fine = true;

  typedef decltype(c0[0]) V0;
  typedef decltype(c1[0]) V1;
  typedef decltype(c2[0]) V2;

  tbb::atomic<size_t> result;

  std::cout << "multi for each over containers:                ";

  aosoa::indexed_for_each([](size_t index, V0& v0, V1& v1, V2& v2) {
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c0, c1, c2);

  result = 0;

  aosoa::for_each([&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "multi for each over iterators:                 ";

  aosoa::indexed_for_each
	(c0.begin(), c0.end(),
	 [](size_t index, V0& v0, V1& v1, V2& v2){
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::for_each
	(c0.begin(), c0.end(),
	 [&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "multi for each range over containers:          ";

  aosoa::indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c0, c1, c2);

  result = 0;

  aosoa::for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2){
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "multi for each range over iterators:           ";

  aosoa::indexed_for_each_range
	(c0.begin(), c0.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::for_each_range
	(c0.begin(), c0.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifdef __ICC

  std::cout << "multi parallel for each over containers:       ";

  aosoa::parallel_indexed_for_each([](size_t index, V0& v0, V1& v1, V2& v2) {
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c0, c1, c2);

  result = 0;

  aosoa::parallel_for_each([&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifndef NO_ITERATORS

  std::cout << "multi parallel for each over iterators:        ";

  aosoa::parallel_indexed_for_each
	(c0.begin(), c0.end(),
	 [](size_t index, V0& v0, V1& v1, V2& v2){
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::parallel_for_each
	(c0.begin(), c0.end(),
	 [&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifdef __ICC

  std::cout << "multi parallel for each range over containers: ";

  aosoa::parallel_indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c0, c1, c2);

  result = 0;

  aosoa::parallel_for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2){
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifndef NO_ITERATORS

  std::cout << "multi parallel for each range over iterators:  ";

  aosoa::parallel_indexed_for_each_range
	(c0.begin(), c0.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::parallel_for_each_range
	(c0.begin(), c0.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#ifdef __cilk

  std::cout << "multi cilk_parallel for each over containers:       ";

  aosoa::cilk_parallel_indexed_for_each([](size_t index, V0& v0, V1& v1, V2& v2) {
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c0, c1, c2);

  result = 0;

  aosoa::cilk_parallel_for_each([&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "multi cilk_parallel for each over iterators:        ";

  aosoa::cilk_parallel_indexed_for_each
	(c0.begin(), c0.end(),
	 [](size_t index, V0& v0, V1& v1, V2& v2){
	  v0.x = index; v0.y = index; v0.z = index;
	  v1.x = index; v1.y = index; v1.z = index;
	  v2.x = index; v2.y = index; v2.z = index;
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::cilk_parallel_for_each
	(c0.begin(), c0.end(),
	 [&result](V0& v0, V1& v1, V2& v2){
	  v0.x += v0.y + v0.z;
	  v1.x += v1.y + v1.z;
	  v2.x += v2.y + v2.z;
	  result += v0.x + v1.x + v2.x;
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

  std::cout << "multi cilk_parallel for each range over containers: ";

  aosoa::cilk_parallel_indexed_for_each_range
	([](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c0, c1, c2);

  result = 0;

  aosoa::cilk_parallel_for_each_range
	([&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2){
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c0, c1, c2);

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#ifndef NO_ITERATORS

  std::cout << "multi cilk_parallel for each range over iterators:  ";

  aosoa::cilk_parallel_indexed_for_each_range
	(c0.begin(), c0.end(),
	 [](size_t start, size_t end, size_t offset,
		typename soa::table_traits<C0>::table_reference t0,
		typename soa::table_traits<C1>::table_reference t1,
		typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x = offset+i;
		t0[i].y = offset+i;
		t0[i].z = offset+i;
		t1[i].x = offset+i;
		t1[i].y = offset+i;
		t1[i].z = offset+i;
		t2[i].x = offset+i;
		t2[i].y = offset+i;
		t2[i].z = offset+i;
	  }
	}, c1.begin(), c2.begin());

  result = 0;

  aosoa::cilk_parallel_for_each_range
	(c0.begin(), c0.end(),
	 [&result](size_t start, size_t end,
			   typename soa::table_traits<C0>::table_reference t0,
			   typename soa::table_traits<C1>::table_reference t1,
			   typename soa::table_traits<C2>::table_reference t2) {
	  for (size_t i=start; i<end; ++i) {
		t0[i].x += t0[i].y + t0[i].z;
		t1[i].x += t1[i].y + t1[i].z;
		t2[i].x += t2[i].y + t2[i].z;
		result += t0[i].x + t1[i].x + t2[i].x;
	  }
	}, c1.begin(), c2.begin());

  std::cout << result;
  if (result == 44550) std::cout << " ok\n";
  else {
	all_fine = false;
	std::cout << " NOT OK!\n";
  }

#endif

#endif

  return all_fine;

}

constexpr size_t len = 100;
constexpr size_t tablesize = 16;

bool stdAOS() {
  std::array<C,len> array;
  std::cout << "\nstandard array\n";
  return test(array);
}

#ifdef NO_ITERATORS
bool flatSOA() {
  soa::table<Cref,len> array;
  std::cout << "\nflat SOA array\n";
  return test(array);
}

bool flatDSOA() {
  soa::dtable<Cref> array(len);
  std::cout << "\nflat dynamic SOA array\n";
  return test(array);
}
#endif

bool nestedSOA1() {
  aosoa::table_array<Cref,1,len> array;
  std::cout << "\ntable array with table size " << 1 << std::endl;
  return test(array);
}

bool nestedSOAN() {
  aosoa::table_array<Cref,len,len> array;
  std::cout << "\ntable array with table size " << len << std::endl;
  return test(array);
}

bool nestedSOAB() {
  aosoa::table_array<Cref,tablesize,len> array;
  std::cout << "\ntable array with table size " << tablesize << std::endl;
  return test(array);
}

bool stdVOS() {
  std::vector<C> array(len);
  std::cout << "\nstandard vector\n";
  return test(array);
}

bool nestedSOV1() {
  aosoa::table_vector<Cref,1> array(len);
  std::cout << "\ntable vector with table size " << 1 << std::endl;
  return test(array);
}

bool nestedSOVN() {
  aosoa::table_vector<Cref,len> array(len);
  std::cout << "\ntable vector with table size " << len << std::endl;
  return test(array);
}

bool nestedSOVB() {
  aosoa::table_vector<Cref,tablesize> array(len);
  std::cout << "\ntable vector with table size " << tablesize << std::endl;
  return test(array);
}

// multi compatibly tabled

bool mcstdAOS() {
  std::array<C,len> a0, a1, a2;
  std::cout << "\nstandard array\n";
  return testmulti(a0, a1, a2);
}

#ifdef NO_ITERATORS
bool mcflatSOA() {
  soa::table<Cref,len> a0, a1, a2;
  std::cout << "\nflat SOA array\n";
  return testmulti(a0, a1, a2);
}

bool mcflatDSOA() {
  soa::dtable<Cref> a0(len), a1(len), a2(len);
  std::cout << "\nflat dynamic SOA array\n";
  return testmulti(a0, a1, a2);
}
#endif

bool mcnestedSOA1() {
  aosoa::table_array<Cref,1,len> a0, a1, a2;
  std::cout << "\ntable array with table size " << 1 << std::endl;
  return testmulti(a0, a1, a2);
}

bool mcnestedSOAN() {
  aosoa::table_array<Cref,len,len> a0, a1, a2;
  std::cout << "\ntable array with table size " << len << std::endl;
  return testmulti(a0, a1, a2);
}

bool mcnestedSOAB() {
  aosoa::table_array<Cref,tablesize,len> a0, a1, a2;
  std::cout << "\ntable array with table size " << tablesize << std::endl;
  return testmulti(a0, a1, a2);
}

bool mcstdVOS() {
  std::vector<C> a0(len), a1(len), a2(len);
  std::cout << "\nstandard vector\n";
  return testmulti(a0, a1, a2);
}

bool mcnestedSOV1() {
  aosoa::table_vector<Cref,1> a0(len), a1(len), a2(len);
  std::cout << "\ntable vector with table size " << 1 << std::endl;
  return testmulti(a0, a1, a2);
}

bool mcnestedSOVN() {
  aosoa::table_vector<Cref,len> a0(len), a1(len), a2(len);
  std::cout << "\ntable vector with table size " << len << std::endl;
  return testmulti(a0, a1, a2);
}

bool mcnestedSOVB() {
  aosoa::table_vector<Cref,tablesize> a0(len), a1(len), a2(len);
  std::cout << "\ntable vector with table size " << tablesize << std::endl;
  return testmulti(a0, a1, a2);
}

/*
// multi incompatibly tabled

For this to work, the xyz_for_each_range templates need to be passed
generic lambda expressions, which will only be part of C++14.

bool mistdAOS() {
  std::array<C,len> a0, a2;
  std::vector<C> a1(len);
  std::cout << "\nstandard array\n";
  return testmulti(a0, a1, a2);
}

#ifdef NO_ITERATORS
bool miflatSOA() {
  soa::table<Cref,len> a0, a2;
  std::array<C,len> a1;
  std::cout << "\nflat SOA array\n";
  return testmulti(a0, a1, a2);
}

bool miflatDSOA() {
  soa::dtable<Cref> a0(len), a2(len);
  std::array<C,len> a1;
  std::cout << "\nflat dynamic SOA array\n";
  return testmulti(a0, a1, a2);
}
#endif

bool minestedSOA1() {
  aosoa::table_array<Cref,1,len> a0, a2;
  std::array<C,len> a1;
  std::cout << "\ntable array with table size " << 1 << std::endl;
  return testmulti(a0, a1, a2);
}

bool minestedSOAN() {
  aosoa::table_array<Cref,len,len> a0, a2;
  std::array<C,len> a1;
  std::cout << "\ntable array with table size " << len << std::endl;
  return testmulti(a0, a1, a2);
}

bool minestedSOAB() {
  aosoa::table_array<Cref,tablesize,len> a0, a2;
  std::array<C,len> a1;
  std::cout << "\ntable array with table size " << tablesize << std::endl;
  return testmulti(a0, a1, a2);
}

bool mistdVOS() {
  std::vector<C> a0(len), a2(len);
  std::array<C,len> a1;
  std::cout << "\nstandard vector\n";
  return testmulti(a0, a1, a2);
}

bool minestedSOV1() {
  aosoa::table_vector<Cref,1> a0(len), a2(len);
  std::array<C,len> a1;
  std::cout << "\ntable vector with table size " << 1 << std::endl;
  return testmulti(a0, a1, a2);
}

bool minestedSOVN() {
  aosoa::table_vector<Cref,len> a0(len), a2(len);
  std::array<C,len> a1;
  std::cout << "\ntable vector with table size " << len << std::endl;
  return testmulti(a0, a1, a2);
}

bool minestedSOVB() {
  aosoa::table_vector<Cref,tablesize> a0(len), a2(len);
  std::array<C,len> a1;
  std::cout << "\ntable vector with table size " << tablesize << std::endl;
  return testmulti(a0, a1, a2);
}
*/

int main() {
  bool all_fine = true;

  all_fine = stdAOS() && all_fine;
#ifdef NO_ITERATORS
  all_fine = flatSOA() && all_fine;
  all_fine = flatDSOA() && all_fine;
#endif
  all_fine = nestedSOA1() && all_fine;
  all_fine = nestedSOAN() && all_fine;
  all_fine = nestedSOAB() && all_fine;

  all_fine = stdVOS() && all_fine;
  all_fine = nestedSOV1() && all_fine;
  all_fine = nestedSOVN() && all_fine;
  all_fine = nestedSOVB() && all_fine;

  std::cout << "\nmulti compatibly tabled\n";

  all_fine = mcstdAOS() && all_fine;
#ifdef NO_ITERATORS
  all_fine = mcflatSOA() && all_fine;
  all_fine = mcflatDSOA() && all_fine;
#endif
  all_fine = mcnestedSOA1() && all_fine;
  all_fine = mcnestedSOAN() && all_fine;
  all_fine = mcnestedSOAB() && all_fine;

  all_fine = mcstdVOS() && all_fine;
  all_fine = mcnestedSOV1() && all_fine;
  all_fine = mcnestedSOVN() && all_fine;
  all_fine = mcnestedSOVB() && all_fine;

  /*
  std::cout << "\nmulti incompatibly tabled\n";

  all_fine = mistdAOS() && all_fine;
#ifdef NO_ITERATORS
  all_fine = miflatSOA() && all_fine;
  all_fine = miflatDSOA() && all_fine;
#endif
  all_fine = minestedSOA1() && all_fine;
  all_fine = minestedSOAN() && all_fine;
  all_fine = minestedSOAB() && all_fine;

  all_fine = mistdVOS() && all_fine;
  all_fine = minestedSOV1() && all_fine;
  all_fine = minestedSOVN() && all_fine;
  all_fine = minestedSOVB() && all_fine;
  */

  if (all_fine) std::cout << "\ndone\n";
  else std::cout << "\nFAILURES OCCURRED!\n";
}
