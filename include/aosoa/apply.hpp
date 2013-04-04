/// Copyright (c) 2012, 2013 by Pascal Costanza, Intel Corporation.

#ifndef AOSOA_APPLY
#define AOSOA_APPLY

#include <cstddef>

#include "soa/table.hpp"

namespace aosoa {
  template<typename F, typename T, typename I>
  inline void apply(const F& f, T&& table, I index) {
	auto& obj = table[index]; f(obj);
  }

  template<typename F, typename T, size_t B, typename I>
  inline void apply(const F& f, soa::table<T,B>& table, I index) {
	auto obj = table[index]; f(obj);
  }

  template<typename F, typename T, typename I>
  inline void apply(const F& f, T&& table, I offset, I index) {
	auto& obj = table[index]; f(offset, obj);
  }

  template<typename F, typename T, size_t B, typename I>
  inline void apply(const F& f, soa::table<T,B>& table, I offset, I index) {
	auto obj = table[index]; f(offset, obj);
  }
}

#endif
