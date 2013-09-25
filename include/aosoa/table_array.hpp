/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AOSOA_TABLE_ARRAY
#define AOSOA_TABLE_ARRAY

#include <cstddef>

#include <array>
#include <stdexcept>
#include <utility>

#include "soa/table.hpp"
#include "soa/table_traits.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa  {

  template<class C, size_t B, size_t N>
  class table_array {
  public:
	static constexpr auto table_size = B;

	typedef C value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef table_iterator<value_type,table_size> iterator;
	typedef const table_iterator<value_type,table_size> const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
	typedef const table_type& const_table_reference;
	typedef table_type* table_pointer;
	typedef const table_type* const_table_pointer;

  private:
	table_type tables[N/table_size+(N%table_size?1:0)];

  public:
	value_type at (size_type pos) {
	  if (pos >= N) throw std::out_of_range("");
	  return tables[pos/table_size][pos%table_size];
	}

	const value_type at (size_type pos) const {
	  if (pos >= N) throw std::out_of_range("");
	  return tables[pos/table_size][pos%table_size];
	}

	value_type operator[] (size_type pos) {
	  return tables[pos/table_size][pos%table_size];
	}

	const value_type operator[] (size_type pos) const {
	  return tables[pos/table_size][pos%table_size];
	}

	value_type front () { return tables[0][0]; }
	const value_type front () const { return tables[0][0]; }

	value_type back () { return (*this)[N-1]; }
	const value_type back () const { return (*this)[N-1]; }

	table_pointer data () { return tables; }
	const_table_pointer data () const { return tables; }

	iterator begin() { return iterator(&tables[0],0); }
	const_iterator begin() const { return iterator(&tables[0], 0); }
	const_iterator cbegin() const { return iterator(&tables[0], 0); }

	iterator end() { return iterator(&tables[(N-1)/table_size], (N-1)%table_size+1); }
	const_iterator end() const { return iterator(&tables[(N-1)/table_size], (N-1)%table_size+1); }
	const_iterator cend() const { return iterator(&tables[(N-1)/table_size], (N-1)%table_size+1); }

	reverse_iterator rbegin() { return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
	const_reverse_iterator crbegin() const { return reverse_iterator(end()); }

	reverse_iterator rend() { return reverse_iterator(begin()); }
	const_reverse_iterator rend() const { return reverse_iterator(begin()); }
	const_reverse_iterator crend() const { return reverse_iterator(begin()); }

	const bool empty () const { return N==0; }
	const size_type size () const { return N; }
	size_type max_size () const { return std::array<soa::table<value_type,1>,1>::max_size(); }

	void fill (const_reference value) {
	  for (size_type i=0; i<N/table_size; ++i) {
		auto& table = tables[i];
		for (size_type j=0; j<table_size; ++j) table[j] = value;
	  }
	  if (N%table_size) {
		auto& table = tables[N/table_size];
		for (size_type j=0; j<N%table_size; ++j) table[j] = value;
	  }
	}

	void swap (table_array& that) { std::swap(tables, that.tables); }
  };
}

namespace soa {
  template<typename T, size_t B, size_t N> class table_traits<aosoa::table_array<T,B,N>> {
  private:
	typedef aosoa::table_array<T,B,N> table_array_type;
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = B;

	typedef typename table_array_type::value_type value_type;
	typedef typename table_array_type::table_reference table_reference;
	typedef typename table_array_type::const_table_reference const_table_reference;
  };
}

#endif
