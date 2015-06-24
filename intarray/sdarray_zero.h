#pragma once

#include "sdarray_interface.h"

#include <vector>
#include <algorithm>
#include <stdint.h>
#include <cassert>

/**  \file
Mock SDArray
*/

namespace mscds {

/// array of integers with SDArray interface
/// sdarray without compression, 
/// implement using vector
/// for testing the correctness of other implementations
struct SDArrayZero: public SDArrayInterface{
	SDArrayZero() { cums.push_back(0); }

	template<typename List>
	SDArrayZero(const List& lst) {
		cums.resize(lst.size() + 1);
		cums[0] = 0;
		unsigned int p = 1;
		for (auto it = lst.cbegin(); it != lst.cend(); ++it) {
			cums[p] = *it + cums[p-1];
			p += 1;
		}
	}

	void add(uint64_t val) { cums.push_back(cums.back() + val); }

	/** returns the value of A[i] */
	uint64_t lookup(uint64_t i) const {
		return cums[i + 1] - cums[i];
	}

	/** returns the value of A[i] and  prev_sum=prefix_sum(i) */
	uint64_t lookup(uint64_t i, uint64_t& prev_sum) const {
		prev_sum = cums[i];
		return cums[i + 1] - cums[i];
	}

	/** return the value of prefix_sum(i) */
	uint64_t prefixsum(size_t i) const {
		return cums[i];
	}

	/** return the value of rank(p) */
	uint64_t rank(uint64_t p) const {
		auto it = std::lower_bound(cums.begin(), cums.end(), p);
		size_t ind = it - cums.begin();

		if (ind < cums.size())
			assert(p <= cums[ind]);
		return ind;
	}

	uint64_t length() const {
		return cums.size() - 1;
	}

	uint64_t total() const {return cums.back(); }

	/** clear the array (length becomes 0) */
	void clear() {
		cums.clear();
		cums.push_back(0);
	}

	void load(InpArchive& ar) {}
	void save(OutArchive& ar) const {}

	std::vector<uint64_t> cums;
};

}//namespace