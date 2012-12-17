#pragma once

#ifndef __BALANCE_PARATHESIS_AUXILIARY_H_
#define __BALANCE_PARATHESIS_AUXILIARY_H_

#include <stdint.h>
#include "bitarray/bitarray.h"
#include "bitarray/rank6p.h"
#include "bitarray/sdarray.h"
#include "bitarray/sdarray.h"
#include <algorithm>
#include <string>

#include "archive.h"

namespace mscds {

class BP_superblock;
class BP_block {
public:
	BP_block(): blksize(0) {}
	BP_block(BitArray _bp, unsigned int _blksize): bp(_bp), blksize(_blksize) {}

	void init(const BitArray& _bp, unsigned int _blksize);

private:
	BitArray bp;
	unsigned int blksize;
	static int8_t min_excess8_t[256], excess8_t[256], min_op_ex_pos8_t[256], min_op_ex8_t[256];

	static uint8_t revbits(uint8_t c) {
		return (c * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	}
	friend class BP_superblock;

public:
	static int8_t min_excess8_c(uint8_t c);
	static int8_t min_revex8_c(uint8_t c);
	static int8_t excess8_c(uint8_t c);
	static int8_t min_op_ex_pos8_c(uint8_t c);

	static const uint64_t NOTFOUND = 0xFFFFFFFFFFFFFFFFull;

	uint64_t forward_scan(uint64_t pos, int64_t excess) const;
	uint64_t backward_scan(uint64_t pos, int64_t excess) const;
	uint64_t min_excess_pos(uint64_t pos, uint64_t endp) const;

	// find [pos .. x]
	uint64_t forward_scan_slow(uint64_t pos, int64_t excess) const;

	// find [x .. pos]
	uint64_t backward_scan_slow(uint64_t pos, int64_t excess) const;

	uint64_t min_excess_pos_slow(uint64_t l, uint64_t r) const;

	void clear();
	OArchive& save(OArchive& ar) const { return ar;}
	IArchive& load(IArchive& ar) { return ar;}

};

class BP_aux;

class BP_superblock {
	BP_block blk;
	unsigned int spblksize;
	int8_t * min;
	BP_aux * parent;
	int8_t * revmin;

	uint64_t forward_scan(uint64_t pos, int64_t excess) const {
		uint64_t ret = BP_block::NOTFOUND;
		if (pos % spblksize != 0)
			ret = blk.forward_scan(pos, excess);
		if (ret != BP_block::NOTFOUND) return ret;
		pos = (pos / spblksize + 1) * spblksize;
		if (pos >= blk.bp.length()) return BP_block::NOTFOUND;
		assert(pos % spblksize == 0);
		uint64_t endp = blk.bp.length();
		uint64_t idx = pos / spblksize;
		for (; pos < endp; pos += spblksize) {
			
		}
		return 0;
	}
	uint64_t backward_scan(uint64_t pos, int64_t excess) const;
};


BitArray find_pioneers(const BitArray& bp, size_t blksize);
std::vector<size_t> find_pioneers_v(const BitArray& bp, size_t blksize);

/**
Implementation of balance parathesis operations based on:

  S.Gog and J.Fischer. Advantages of Shared Data Structures for Sequences of Balanced Parentheses. DCC 2010.

  Geary, R.F. and Rahman, N. and Raman, R. and Raman, V. A Simple Optimal Representation for Balanced Parentheses. TCS 2006.
  */
class BP_aux {
public:
	BP_aux():blksize(0), lowerlvl(NULL) {}
	~BP_aux() {clear();}
	unsigned int build(const BitArray &bp, unsigned int blksize = 256);
	void clear();
private:
	BP_block blk;
	BitArray bp_bits;
	Rank6p bprank;
	BP_aux * lowerlvl;
	SDRankSelect pioneer_map;

	unsigned int rec_lvl;
	unsigned int blksize;
public:
	uint64_t rank(uint64_t i) const { return bprank.rank(i); }
	uint64_t select(uint64_t i) const { return bprank.select(i); }
	int64_t excess(uint64_t i) const
		{ return (int64_t)bprank.rank(i) * 2 - i; }
	uint64_t find_match(uint64_t p) const
		{ return (bit(p)?find_close(p):find_open(p)); }
	bool bit(uint64_t p) const { return bp_bits[p]; }
	size_t length() const { return bp_bits.length(); }
	std::string to_str() const;

	uint64_t find_close(uint64_t p) const;
	uint64_t find_open(uint64_t p) const;
	uint64_t enclose(uint64_t p) const;
	uint64_t rr_enclose(uint64_t i, uint64_t j) const;
	uint64_t min_excess_pos(uint64_t l, uint64_t r) const;

	OArchive& save(OArchive& ar) const;
	IArchive& load(IArchive& ar);
};




} //namespace


#endif //__BALANCE_PARATHESIS_AUXILIARY_H_
