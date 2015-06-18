#pragma once

#include "stdint.h"
#include "bitarray/bitstream.h"
#include "bitarray/bitrange.h"

namespace mscds {


class SDArrayBlock2 {
public:
	typedef uint64_t ValueType;
	typedef unsigned int IndexType;
	void add(ValueType v);
	void saveBlock(OBitStream* bits);
	void loadBlock(const BitRange& br);
	void loadBlock(const BitArrayInterface * ba, size_t pt, size_t len);

	ValueType prefixsum(unsigned int  p) const;
	ValueType lookup(unsigned int p) const;
	ValueType lookup(unsigned int p, ValueType& prev_sum) const;
	unsigned int rank(ValueType val) const;

	void clear() { lastpt = ~0ULL; vals.clear(); bits = nullptr; width = 0; blkptr = 0; }
    SDArrayBlock2() { clear(); }

	static const unsigned int BLKSIZE = 1023;
private:
	static const unsigned int SUBB_PER_BLK = 6;
	static const unsigned int SUBB_SIZE = 147; // BLKSIZE / (SUBB_PER_BLK + 1)
	static const unsigned int H_WIDTH = 11;    // log(BLKSIZE * 2)
	// BLKSIZE=511, 6, 73, 10

	unsigned int select_hi(const uint16_t* hints, uint64_t start, uint32_t off) const;
	unsigned int select_zerohi(const uint16_t * hints, uint64_t start, uint32_t off) const;
private:
	uint16_t hints[SUBB_PER_BLK];
	uint16_t width;
	size_t lastpt, blkptr;

	const BitArrayInterface * bits;
	std::vector<ValueType> vals;
};


}//namespace
