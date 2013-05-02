/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef SOA_TABLE_TRAITS
#define SOA_TABLE_TRAITS

#include <array>
#include <vector>

#include "soa/table.hpp"
#include "soa/dtable.hpp"

namespace soa {

  template<typename T> class table_traits;

  template<typename T, size_t N> class table_traits<soa::table<T,N>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = N;

	typedef T value_type;
	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
	typedef const table_reference const_table_reference;
  };

  template<typename T> class table_traits<soa::dtable<T>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = SIZE_MAX;

	typedef T value_type;
	typedef soa::dtable<value_type> table_type;
	typedef table_type& table_reference;
	typedef const table_reference const_table_reference;
  };

  template<typename T, size_t N> class table_traits<std::array<T,N>> {
  public:
	static constexpr auto tabled = false;
	static constexpr auto table_size = 1;

	typedef T value_type;
	typedef typename std::array<T,N>::iterator table_reference;
	typedef typename std::array<T,N>::const_iterator const_table_reference;
  };

  template<typename T> class table_traits<std::vector<T>> {
  public:
	static constexpr auto tabled = false;
	static constexpr auto table_size = 1;

	typedef T value_type;
	typedef typename std::vector<T>::iterator table_reference;
	typedef typename std::vector<T>::const_iterator const_table_reference;
  };

}

#endif
