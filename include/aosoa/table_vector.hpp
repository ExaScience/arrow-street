/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_TABLE_VECTOR
#define AOSOA_TABLE_VECTOR

#include <cstddef>

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef __MIC__
#include "tbb/cache_aligned_allocator.h"
#endif

#include "soa/table.hpp"
#include "aosoa/table_iterator.hpp"

namespace aosoa {

  template<class C, size_t B, class Allocator =
#ifdef __MIC__
		   tbb::cache_aligned_allocator<soa::table<C,B>>
#else
		   std::allocator<soa::table<C,B>>
#endif
		   >
	class table_vector {
	public:
	  static constexpr auto table_size = B;

	  typedef C value_type;
	  typedef Allocator allocator_type;
	  typedef size_t size_type;
	  typedef ptrdiff_t difference_type;
	  typedef value_type& reference;
	  typedef const reference const_reference;
	  typedef value_type* pointer;
	  typedef const pointer const_pointer;
	  typedef table_iterator<value_type,table_size> iterator;
	  typedef const iterator const_iterator;
	  typedef std::reverse_iterator<iterator> reverse_iterator;
	  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	  typedef soa::table<value_type,table_size> table_type;
	  typedef table_type& table_reference;
	  typedef const table_reference const_table_reference;
	  typedef table_type* table_pointer;
	  typedef const table_pointer const_table_pointer;

	private:
	  size_type n;
	  std::vector<table_type, allocator_type> tables;

	public:
	  explicit table_vector (size_type count = 0) :
		n(count), tables(count/table_size+(count%table_size?1:0))
	  {}

	  explicit table_vector (size_type count, const_reference value) :
		n(count), tables(count/table_size+(count%table_size?1:0))
	  { fill(value); }

	  table_vector (const table_vector& that) :
		n(that.n), tables(that.tables)
	  {}

	  table_vector (table_vector&& that) :
		n(that.n), tables(std::move(that.tables))
	  {}

	  table_vector& operator= (const table_vector& that) {
		n = that.n;
		tables = that.tables;
		return *this;
	  }

	  table_vector& operator= (table_vector&& that) {
		n = that.n;
		tables = std::move(that.tables);
		return *this;
	  }

	  void assign (size_type count, const_reference value) {
		n = count;
		tables.resize(count/table_size+(count%table_size?1:0));
		fill(value);
	  }

	  allocator_type get_allocator () const { return tables.get_allocator(); }

	  value_type at (size_type pos) {
		if (pos >= n) throw std::out_of_range("");
		return tables[pos/table_size][pos%table_size];
	  }

	  const value_type at (size_type pos) const {
		if (pos >= n) throw std::out_of_range("");
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

	  value_type back () { return (*this)[n-1]; }
	  const value_type back () const { return (*this)[n-1]; }

	  table_pointer data () { return tables.data(); }
	  const_table_pointer data () const { return tables.data(); }

	  iterator begin() { return iterator(&tables[0],0); }
	  const_iterator begin() const { return iterator(&tables[0], 0); }
	  const_iterator cbegin() const { return iterator(&tables[0], 0); }

	  iterator end() { return iterator(&tables[(n-1)/table_size], (n-1)%table_size+1); }
	  const_iterator end() const { return iterator(&tables[(n-1)/table_size], (n-1)%table_size+1); }
	  const_iterator cend() const { return iterator(&tables[(n-1)/table_size], (n-1)%table_size+1); }

	  reverse_iterator rbegin() { return reverse_iterator(end()); }
	  const_reverse_iterator rbegin() const { return reverse_iterator(end()); }
	  const_reverse_iterator crbegin() const { return reverse_iterator(end()); }

	  reverse_iterator rend() { return reverse_iterator(begin()); }
	  const_reverse_iterator rend() const { return reverse_iterator(begin()); }
	  const_reverse_iterator crend() const { return reverse_iterator(begin()); }

	  bool empty () const { return n==0; }
	  size_type size () const { return n; }
	  size_type max_size () const { return std::vector<soa::table<value_type,1>, allocator_type>::max_size(); }

	  void reserve (size_type size) { tables.reserve(size/table_size+(size%table_size?1:0)); }
	  size_type capacity () const { return tables.capacity() * table_size; }
	  void shrink_to_fit () { tables.shrink_to_fit(); }
	  void clear () { n = 0; tables.clear(); }

	  void push_back (const_reference value) {
		auto m = n;
		resize(n+1);
		(*this)[m] = value;
	  }

	  void push_back (value_type&& value) {
		auto m = n;
		resize(n+1);
		(*this)[m] = std::move(value);
	  }

	  void pop_back () { n--; }

	  void resize (size_type count) {
		if (n != count) {
		  n = count;
		  tables.resize(count/table_size+(count%table_size?1:0));
		}
	  }

	  void resize (size_type count, const_reference value) {
		auto o = n;
		n = count;
		auto ndb = n/table_size, nmb = n%table_size;
		tables.resize(ndb+(nmb?1:0));

		if (n > o) {
		  auto odb = o/table_size, omb = o%table_size;
		  if (odb == ndb) {
			auto& table = tables(ndb);
			for (size_type j=omb; j<nmb; ++j) table[j] = value;
		  } else {
			if (omb) {
			  auto& table = tables[odb];
			  for (size_type j=omb; j<table_size; ++j) table[j] = value;
			}
			for (size_type i=odb+(omb?1:0); i<ndb; ++i) {
			  auto& table = tables[i];
			  for (size_type j=0; j<table_size; ++j) table[j] = value;
			}
			if (nmb) {
			  auto& table = tables[ndb];
			  for (size_type j=0; j<nmb; ++j) table[j] = value;
			}
		  }
		}
	  }

	  void fill (const_reference value) {
		for (size_type i=0; i<n/table_size; ++i) {
		  auto& table = tables[i];
		  for (size_type j=0; j<table_size; ++j) table[j] = value;
		}
		if (n%table_size) {
		  auto& table = tables[n/table_size];
		  for (size_type j=0; j<table_size; ++j) table[j] = value;
		}
	  }

	  void swap (table_vector& that) {
		std::swap(n, that.n);
		std::swap(tables, that.tables);
	  }

	};
}

namespace soa {
  template<typename T, size_t B> class table_traits<aosoa::table_vector<T,B>> {
  private:
	typedef aosoa::table_vector<T,B> table_vector_type;
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = B;

	typedef typename table_vector_type::value_type value_type;
	typedef typename table_vector_type::table_reference table_reference;
	typedef typename table_vector_type::const_table_reference const_table_reference;
  };
}

#endif
