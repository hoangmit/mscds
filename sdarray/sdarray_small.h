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
	void build(OArchive& ar);
	void clear();

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
	
	std::string to_str(bool psum) const;
	void dump_text(std::ostream& fo) const;
	void clear();
	void save(OArchive& ar) const;
	void load(IArchive& ar);
	uint64_t total() const { return sum; }
private:
	uint64_t rank(uint64_t lo, uint64_t hi, uint64_t val) const;

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
	friend struct SDASIIterator;
	friend class SDRankSelectSml;

};

class SDRankSelectSml {
public:
	SDRankSelectSml() {}
	~SDRankSelectSml() { clear(); }

	void build(const std::vector<uint64_t>& inc_pos);
	void build(const std::vector<unsigned int>& inc_pos);
	void build(BitArray& ba);

	uint64_t one_count() const { return qs.length(); }

	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t r) const { assert(r < one_count()); return qs.prefixsum(r+1); }

	void load(IArchive& ar);
	void save(OArchive& ar) const;

	void clear() { qs.clear(); rankhints.clear(); }
	std::string to_str() const;
private:
	void initrank();
	unsigned int ranklrate;
	SDArraySml qs;
	FixedWArray rankhints;
};



}//namespace