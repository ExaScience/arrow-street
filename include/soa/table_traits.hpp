/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SOA_TABLE_TRAITS
#define SOA_TABLE_TRAITS

#include <cstdint>
#include <array>
#include <vector>

#include "soa/table.hpp"
#include "soa/dtable.hpp"

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

namespace soa {

  template<typename T> class table_traits;

  template<typename T, size_t N> class table_traits<soa::table<T,N>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = N;

	typedef T value_type;
	typedef soa::table<value_type,table_size> table_type;
	typedef table_type& table_reference;
	typedef const table_type& const_table_reference;
  };

  template<typename T> class table_traits<soa::dtable<T>> {
  public:
	static constexpr auto tabled = true;
	static constexpr auto table_size = SIZE_MAX;

	typedef T value_type;
	typedef soa::dtable<value_type> table_type;
	typedef table_type& table_reference;
	typedef const table_type& const_table_reference;
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

  template<class... C> class is_compatibly_tabled;

  template<class C> class is_compatibly_tabled<C> {
  public:
	static constexpr auto value = table_traits<C>::tabled;
  };

  template<class C, class C0, class... CN> class is_compatibly_tabled<C, C0, CN...> {
  private:
	typedef table_traits<C> traits;
	typedef table_traits<C0> traits0;
  public:
	static constexpr auto value =
	  traits::tabled && traits0::tabled &&
	  (traits::table_size == traits0::table_size) &&
	  is_compatibly_tabled<C, CN...>::value;
  };
}

#endif
