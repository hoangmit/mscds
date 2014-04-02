#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include "codec/deltacoder.h"

#include <vector>
#include <algorithm>
#include <iostream>

namespace mscds {

class AxPtr {
public:
	AxPtr(): ba(nullptr) {}

	void init(unsigned int nb) { start_.resize(nb + 1); count_ = nb, valid = false; }

	void set(unsigned int i, unsigned int blksize) { start_[i + 1] = blksize; }
	void set_abs(unsigned int i, unsigned int start_val) { assert(start_val >= start_[i]); start_[i+1] = start_val - start_[i]; }

	void _build() {
		valid = true;
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
		w = 0;
		w = ceillog2(start_[count_] + 1 - count_*mind);
		bs->puts(coder::DeltaCoder::encode(mind + 1));
		for (unsigned i = 1; i <= count_; ++i) {
			assert(start_[i] > i * mind);
			bs->puts(start_[i] - i * mind, w);
		}
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