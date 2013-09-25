/*
Copyright (c) 2013, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AOSOA_APPLY_TUPLE
#define AOSOA_APPLY_TUPLE

#include <tuple>

namespace aosoa {

  namespace {
	template<int N> class _apply_tuple;

	template<> class _apply_tuple<0> {
	public:
	  template<typename F, typename T, typename... Args>
	  inline static auto call (F&& f, T&, Args&... args)
		-> decltype(f(args...))
	  {return f(args...);}
	};

	template<int N> class _apply_tuple {
	public:
	  template<typename F, typename T, typename... Args>
	  inline static auto call (F&& f, T& t, Args&... args)
		-> decltype(_apply_tuple<N-1>::call(f, t, std::get<N-1>(t), args...))
	  {return _apply_tuple<N-1>::call(f, t, std::get<N-1>(t), args...);}
	};
  }

  template<typename F, typename... T>
  inline auto apply_tuple(F&& f, std::tuple<T...>&& t)
	-> decltype(_apply_tuple<sizeof...(T)>::call(f, t))
  {return _apply_tuple<sizeof...(T)>::call(f, t);}
}

#endif
