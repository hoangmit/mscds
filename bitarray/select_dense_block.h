#pragma once

#include "bitarray.h"
#include "bitstream.h"

namespace mscds {
//for internal use only
struct DenseSelectBlock {
	static const unsigned int BLK_COUNT = 512;
	struct Header {
		uint64_t v1;
		uint64_t v2;
	};

	Header h;

	void build(const std::vector<unsigned int>& _inp,
		OBitStream& overflow, uint64_t start_val, unsigned int start_flow);

	uint64_t blk_ptr() const;

	unsigned int get_span() const;
	unsigned int get_sublen() const;
	unsigned int get_casex(unsigned int pos,
		const BitArray& v, unsigned int start) const;

	// private functions
	static const uint64_t SUBMASK = (1ull << 11) - 1;
	static const uint64_t MASK_1 = 0x00FFFFFFFFFFFFFFull;

	unsigned int _get_case0(unsigned int pos) const;
	void _write_case(unsigned int z, unsigned int w, const std::vector<unsigned int>& vals,
		OBitStream& overflow, unsigned int start_flow);
	// the number of bits to store number p
	static unsigned int _numbit(unsigned int p);
	static bool _check_span(unsigned int span, const std::vector<unsigned int>& inp,
		const unsigned int max_diff);
	static std::vector<unsigned int> _make_span(unsigned int span,
		const std::vector<unsigned int>& inp, unsigned int& width);
};


}//namespace