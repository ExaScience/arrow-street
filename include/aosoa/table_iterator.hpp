/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_TABLE_ITERATOR
#define AOSOA_TABLE_ITERATOR

#include <iterator>

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
  };

  template<typename T, size_t N> class table_iterator_traits<table_iterator<T,N>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = N;

	typedef T value_type;
	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
  };
}

#endif
