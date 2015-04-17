#pragma once

#include "stdint.h"
#include "bitarray/bitstream.h"
#include "bitarray/bitrange.h"

namespace mscds {

class SDArrayFuse;

class SDArrayBlock {
public:
	typedef uint64_t ValueType;
	typedef unsigned int IndexType;
	void add(ValueType v);
	void saveBlock(OBitStream* bits);
	void loadBlock(const BitRange& br);
	void loadBlock(const BitArray & ba, size_t pt, size_t len);

	ValueType prefixsum(unsigned int  p) const;
	ValueType lookup(unsigned int p) const;
	ValueType lookup(unsigned int p, ValueType& prev_sum) const;
	unsigned int rank(ValueType val) const;

	void clear() { lastpt = ~0ULL; vals.clear(); bits.clear(); width = 0; select_hints = 0; blkptr = 0; }
	SDArrayBlock() { clear(); }
	static const unsigned int BLKSIZE = 512;
private:
	unsigned int select_hi(uint64_t hints, uint64_t start, uint32_t off) const;
	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
	unsigned int select_zerohi(uint64_t hints, uint64_t start, uint32_t off) const;
private:
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;

	std::vector<ValueType> vals;

	uint16_t width;
	uint64_t select_hints;

	size_t lastpt;

	size_t blkptr;
	BitArray bits;
	//template<typename>
	friend class SDArrayFuse;
};


}//namespace