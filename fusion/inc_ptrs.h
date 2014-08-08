#pragma once

/**
Experimental fusion related structure. 
*/

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include <vector>
#include <algorithm>
#include <iostream>

namespace mscds {

class FixBlockPtr {
public:
	FixBlockPtr(): count_(0) {}

	void init(unsigned int nb) { start_.resize(1); start_[0] = 0; count_ = nb, valid = false; }

	void add(unsigned int blksize) { start_.push_back(blksize); }

	void _build() {
		valid = true;
		start_[0] = 0;
		assert(start_.size() == count_ + 1);
		for (unsigned int i = 1; i <= count_; ++i) {
			start_[i] = start_[i - 1] + start_[i];
		}
	}

	void saveBlock(OBitStream * bs) {
		if (!valid) _build();

		assert(start_.size() == count_ + 1);
		unsigned int w = 0;
		for (unsigned i = 0; i < count_; ++i)
			w = std::max<unsigned>(ceillog2(1 + start_[i+1] - start_[i]), w);
		bs->puts(w, 8);
		for (unsigned i = 0; i < count_; ++i)
			bs->puts(start_[i+1] - start_[i], w);
	}

	void reset() {
		valid = false;
		start_.resize(1);
	}


	void loadBlock(BitArray& ba, size_t pt, size_t len) {
		w = ba.bits(pt, 8);
		start_.resize(1 + count_);
		start_[0] = 0;
		for (unsigned i = 0; i < count_; ++i)
			start_[i + 1] = (start_[i] + ba.bits(pt + 8 + i * w, w)); 
		valid = true;
	}

	unsigned int ptr_space() const {
		return 8 + count_ * w;
	}

	//------------------------------------------------------------------------

	unsigned int start(unsigned int i) const {
		return start_[i];
	}
	unsigned int end(unsigned int i) const { return start_[i+1]; }
	unsigned int length(unsigned int i) const { return start_[i+1] - start_[i]; }
	
	void clear() { valid = false; start_.clear(); }

private:
	bool valid;
	unsigned int count_, w;
	std::vector<unsigned int> start_;
};



}//namespace