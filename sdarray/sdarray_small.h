#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"
#include <iostream>

namespace mscds {

class SDArraySml;

class SDArraySmlBuilder {
public:
	SDArraySmlBuilder();

	void add(uint64_t val);

	void build(SDArraySml* out);

	static const uint64_t BLKSIZE;
	static const uint16_t SUBB_PER_BLK;

private:
	void packHighs(uint64_t begPos, uint64_t width);
	void packLows(uint64_t begPos, uint64_t width);

	void build_blk();
	std::vector<uint64_t> vals;
	OBitStream bits;
	std::vector<uint64_t> table;
	uint64_t cnt, p_sum;
};

class SDArraySml {
public:
	uint64_t prefixsum(size_t p) const ;
	uint64_t length() const { return len; }
	uint64_t lookup(const uint64_t p) const;
	uint64_t lookup(const uint64_t pos, uint64_t& prev_sum) const;
	uint64_t rank(uint64_t val) const;
private:
	uint64_t scan_hi_bits(uint64_t start, uint32_t p) const;
	uint64_t select_hi(uint64_t hints, uint64_t start, uint32_t p) const;

	uint64_t scan_hi_zeros(uint64_t start, uint32_t p) const;
	uint64_t select_zerohi(uint64_t hints, uint64_t start, uint32_t p) const;
	uint64_t rankBlk(uint64_t blk, uint64_t val) const;

	uint64_t scan_zerohi_bitslow(uint64_t start, uint32_t res) const;
	
	BitArray bits;
	BitArray table;
	uint64_t len, sum;
	friend class SDArraySmlBuilder;
	static const uint64_t BLKSIZE;
	static const uint64_t SUBB_SIZE;

	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);

};

}//namespace