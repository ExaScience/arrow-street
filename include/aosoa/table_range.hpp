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

#ifndef AOSOA_TABLE_RANGE
#define AOSOA_TABLE_RANGE

#include "tbb/tbb_stddef.h"

namespace aosoa {

  template<class Iterator>
  class table_range {
  private:
	Iterator lower, upper;

	typedef table_iterator_traits<Iterator> traits;

  public:
	table_range () {}

	table_range (Iterator lower, Iterator upper) : lower(lower), upper(upper) {}

	bool empty() const {return lower == upper;}

	bool is_divisible() const {return lower.table < upper.table;}

	table_range (table_range& that, tbb::split) {
	  auto mid = (that.upper.table - that.lower.table) / 2;
	  lower.table = that.lower.table + mid + 1;
	  lower.index = 0;
	  upper = that.upper;
	  that.upper.table = that.lower.table + mid;
	  that.upper.index = traits::table_size;
	}

	Iterator begin() {return lower;}
	const Iterator begin() const {return lower;}
	Iterator end() {return upper;}
	const Iterator end() const {return upper;}
  };
}

#endif
