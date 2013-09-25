/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AOSOA_TABLE_ITERATOR
#define AOSOA_TABLE_ITERATOR

#include <iterator>

#include "soa/table.hpp"

#ifdef __cilk
#include <cilk/cilk.h>

template<typename B, typename G, typename F>
void cilk_iterate(B start, B end, G grainsize , const F& f) {
  auto length = end-start;
  if (length <= grainsize) f(start, end);
  else {
	auto half = length/2;
	auto mid = start+half;
	cilk_spawn cilk_iterate(start, mid, grainsize, f);
	cilk_iterate(mid, end, grainsize, f);
	cilk_sync;
  }
}

template<typename B, typename F>
void cilk_iterate(B start, B end, const F& f) {
  auto length = end-start;
  if (length == 1) f(start);
  else if (length > 1) {
	auto half = length/2;
	auto mid = start+half;
	cilk_spawn cilk_iterate(start, mid, f);
	cilk_iterate(mid, end, f);
	cilk_sync;
  }
}

#endif

namespace aosoa {

  template<class C, size_t B>
  class table_iterator {
  public:
	static constexpr auto table_size = B;

	typedef C value_type;
	typedef ptrdiff_t difference_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef std::random_access_iterator_tag iterator_category;

	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
	typedef table_type* table_pointer;

	table_pointer table;
	int index;

	table_iterator(table_pointer table = nullptr, int i = 0) :
	  table(table), index(i)
	{}

	table_iterator(const table_iterator& that) :
	  table(that.table),
	  index(that.index)
	{}

	inline table_iterator& operator=(const table_iterator& that) {
	  table = that.table;
	  index = that.index;
	  return *this;
	}

	inline table_iterator operator++() {
	  ++index;
	  if (index == table_size) {
		++table;
		index = 0;
	  }
	  return *this;
	}

	inline table_iterator operator++(int) {
	  table_iterator result(*this);
	  index++;
	  if (index == table_size) {
		table++;
		index = 0;
	  }
	  return result;
	}

	inline table_iterator operator--() {
	  --index;
	  if (index == -1) {
		--table;
		index = table_size-1;
	  }
	  return *this;
	}

	inline table_iterator operator--(int) {
	  table_iterator result(*this);
	  index--;
	  if (index == -1) {
		table--;
		index = table_size-1;
	  }
	  return result;
	}

	inline value_type operator*() {
	  return (*table)[index];
	}

	inline value_type operator*() const {
	  return (*table)[index];
	}

	inline value_type operator[](difference_type n) {
	  auto nindex = index+n;
	  if (nindex < 0) {
		return (*(table+nindex/table_size-1))[table_size+nindex%table_size];
	  } else {
		return (*(table+nindex/table_size))[nindex%table_size];
	  }
	}

	inline const value_type operator[](difference_type n) const {
	  auto nindex = index+n;
	  if (nindex < 0) {
		return (*(table+nindex/table_size-1))[table_size+nindex%table_size];
	  } else {
		return (*(table+nindex/table_size))[nindex%table_size];
	  }
	}

	/*
	  pointer operator->() {
	  return &(*table)[index];
	  }
	*/

	inline table_iterator operator+(difference_type n) const {
	  auto nindex = index+n;
	  if (nindex < 0) {
		return table_iterator(table+nindex/table_size-1, table_size+nindex%table_size);
	  } else {
		return table_iterator(table+nindex/table_size, nindex%table_size);
	  }
	}

	inline table_iterator operator+=(difference_type n) {
	  index += n;
	  if (index < 0) {
		table += index/table_size-1; index = table_size+index%table_size;
	  } else {
		table += index/table_size; index %= table_size;
	  }
	  return *this;
	}

	inline table_iterator operator-=(difference_type n) {
	  index -= n;
	  if (index < 0) {
		table += index/table_size-1; index = table_size+index%table_size;
	  } else {
		table += index/table_size; index %= table_size;
	  }
	  return *this;
	}

	inline table_iterator operator-(difference_type n) const {
	  auto nindex = index-n;
	  if (nindex < 0) {
		return table_iterator(table+nindex/table_size-1, table_size+nindex%table_size);
	  } else {
		return table_iterator(table+nindex/table_size, nindex%table_size);
	  }
	}

	inline difference_type operator-(const table_iterator& that) const {
	  return (table-that.table)*table_size+index-that.index;
	}

	inline bool operator<(const table_iterator& that) const {
	  if (table<that.table) return true;
	  else if (table==that.table) return index<that.index;
	  else return false;
	}

	inline bool operator>(const table_iterator& that) const {
	  if (table>that.table) return true;
	  else if (table==that.table) return index>that.index;
	  else return false;
	}

	inline bool operator<=(const table_iterator& that) const {
	  if (table<that.table) return true;
	  else if (table==that.table) return index<=that.index;
	  else return false;
	}

	inline bool operator>=(const table_iterator& that) const {
	  if (table>that.table) return true;
	  else if (table==that.table) return index>=that.index;
	  else return false;
	}

	inline bool operator==(const table_iterator& that) const {
	  return (table == that.table) && (index == that.index);
	}

	inline bool operator!=(const table_iterator& that) const {
	  return (table != that.table) || (index != that.index);
	}
  };

  template<typename T> class table_iterator_traits {
  private:
	typedef std::iterator_traits<T> traits;

  public:
	static constexpr auto tabled = false;
	static constexpr auto table_size = 1;

	typedef typename traits::value_type value_type;
	typedef T table_reference;
  };

  template<typename T, size_t N> class table_iterator_traits<table_iterator<T,N>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = N;

	typedef T value_type;
	typedef soa::table<value_type,table_size> table_type;
	typedef table_type* table_pointer;
	typedef table_type& table_reference;
  };

  template<typename... T> class is_compatibly_tabled_iterator;

  template<typename T> class is_compatibly_tabled_iterator<T> {
  public:
	static constexpr auto value = table_iterator_traits<T>::tabled;
  };

  template<typename T, typename T0, typename... TN> class is_compatibly_tabled_iterator<T, T0, TN...> {
  private:
	typedef table_iterator_traits<T> traits;
	typedef table_iterator_traits<T0> traits0;
  public:
	static constexpr auto value =
	  traits::tabled && traits0::tabled &&
	  (traits::table_size == traits0::table_size) &&
	  is_compatibly_tabled_iterator<T, TN...>::value;
  };
}

#endif
