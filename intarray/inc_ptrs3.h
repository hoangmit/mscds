#pragma once

/**
Experimental fusion related structure.
*/


#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include "codec/deltacoder.h"

#include <vector>
#include <iostream>
#include <algorithm>

namespace mscds {

class MicroSDPtr {
public:
	MicroSDPtr(): ba(nullptr) {}

	void init(unsigned int nb) { start_.resize(1); count_ = nb, valid = false; }

	void add(unsigned int blksize) { start_.push_back(blksize); }

	void _build() {
		valid = true;
		assert(start_.size() == count_ + 1);
		start_[0] = 0;
		for (unsigned int i = 1; i <= count_; ++i) {
			start_[i] = start_[i - 1] + start_[i];
		}
	}

	void saveBlock(OBitStream * bs, uint8_t* w_out) {
		if (!valid) _build();

		assert(start_.size() == count_ + 1);
		unsigned mind = std::numeric_limits<unsigned>::max();
		for (unsigned i = 1; i <= count_; ++i)
			mind = std::min<unsigned>(mind, start_[i] - start_[i-1]);

		bs->puts(coder::DeltaCoder::encode(mind + 1));
		for (unsigned i = 1; i <= count_; ++i) {
			assert(start_[i] > i * mind);
			start_[i] = start_[i] - i * mind;
		}
		w = ceillog2(start_.back() / count_ + 1);
		unsigned l1 = (start_.back() >> w) + count_;
		unsigned l2 = count_ * w;
		std::cout << "W=" << w << "  hi-bit-length=" << l1 << "  low-bit-length=" << l2 << "    mind=" << mind << std::endl;
		std::cout << "total " << l1 + l2 << std::endl;

		*w_out = w;
		//debug_print(mind);
		start_.clear();
	}

	void debug_print(unsigned int mind, std::ostream& out = std::cout) {
		out << "AxPtr" << '\n';
		out << "W = " << w << "    " << "A = " << mind << "\n";
		for (unsigned i = 1; i <= count_; ++i) 
			out << start_[i] << ", ";
		out << "\n\n";

		for (unsigned i = 1; i <= count_; ++i)
			out << start_[i] - i*mind << ", ";
		out << "\n\n";
	}

	//---------------------
	void reset() {
		valid = false;
		ba = nullptr;
		valid = false;
		start_.resize(1);
	}

	void loadBlock(BitArray& ba, size_t pt, size_t len, uint8_t w_out) {
		this->w = w_out;
		start_.resize(1);
		start_[0] = 0;
		uint64_t ec = ba.bits(pt, std::min<unsigned int>(len, 64));
		auto p = coder::DeltaCoder::decode2(ec);
		p.first -= 1;

		this->mind = p.first;
		this->ba = &ba;
		this->base_pt = p.second + pt;
		valid = true;
		assert(len >= p.second + w * count_);
	}

	unsigned int ptr_space() const {
		return (count_ - 1) * w;
	}

	//------------------------------------------------------------------------

	unsigned int start(unsigned int i) const {
		if (i == 0) return 0;
		assert(i <= count_);
		return ba->bits(base_pt + (i - 1) * w, w) + mind * i;
	}
	unsigned int end(unsigned int i) const { assert(i < count_); return start(i+1); }
	unsigned int length(unsigned int i) const { assert(i < count_); return start(i+1) - start(i); }


private:
	bool valid;
		
	std::vector<unsigned int> start_;
	unsigned int count_;

	unsigned int w, base_pt, mind;
	BitArray * ba;
};

}//namespace