#pragma once

#include <stdint.h>
#include "bitarray/bitarray.h"
#include "bitarray/rank6p.h"
#include "bitarray/sdarray.h"
#include "bitarray/sdarray.h"
#include <algorithm>
#include <string>

#include "utils/debug.h"

namespace mscds {

struct BP_block {
	BP_block(): blksize(0) {}
	BP_block(BitArray _bp, unsigned int _blksize): bp(_bp), blksize(_blksize) {}
	//BP_block& operator=(const BP_block& other) {bp = other.bp; blksize = other.blksize; return * this;}

	void init(const BitArray& _bp, unsigned int _blksize) {
		this->bp = _bp;
		blksize = _blksize;
	}

	BitArray bp;
	unsigned int blksize;
	static int8_t min_excess8_t[256], excess8_t[256], min_op_ex_pos8_t[256], min_op_ex8_t[256];

	static uint8_t revbits(uint8_t c) {
		return (c * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	}

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

	void clear() {
		blksize = 0;
		bp.clear();
	}
};

class BP_aux;

struct BP_superblock {
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

class BP_aux {
public:
	bool get(uint64_t p) const { return bp_bits[p]; }
	int64_t excess(uint64_t i) const { return (int64_t)bprank.rank(i) * 2 - i; }
	void build(const BitArray &bp, unsigned int blksize = 128);
	~BP_aux() {clear();}
	void clear();
	size_t length() const { return bp_bits.length(); }
private:
	BP_block blk;
	BitArray bp_bits;
	Rank6p bprank;
	BP_aux * lowerlvl;
	SDRankSelect pioneer_map;

	unsigned int rec_lvl;
	unsigned int blksize;
public:
	uint64_t find_match(uint64_t p) const {
		return (bit(p) ? find_close(p) : find_open(p));
	}

	bool bit(uint64_t p) const { return bp_bits[p]; }

	std::string to_str() const;
	uint64_t pioneer_count() const {
		if (lowerlvl != NULL) return pioneer_map.one_count();
		else return 0;
	}

	uint64_t find_close(uint64_t p) const;
	uint64_t find_open(uint64_t p) const;
	uint64_t enclose(uint64_t p) const;

	uint64_t rr_enclose(uint64_t i, uint64_t j) const {
		assert(i < j && j < length());
		assert(bit(i) && bit(j));
		return min_excess_pos(find_close(i) + 1, j);
	}

	uint64_t min_excess_pos(uint64_t l, uint64_t r) const {
		if (l >= r || r >= length()) return BP_block::NOTFOUND;
		uint64_t k = BP_block::NOTFOUND;
		if (l/blksize == (r-1)/blksize) return blk.min_excess_pos(l, r);
		uint64_t min_ex = excess(r+1) + 2*(!bit(r) ? 1 : 0);
		uint64_t pl = pioneer_map.rank(l);
		uint64_t pr = pioneer_map.rank(r);
		uint64_t kp = BP_block::NOTFOUND;
		
			kp = lowerlvl->min_excess_pos(pl, pr);
		if (kp != BP_block::NOTFOUND) {
			k = pioneer_map.select(kp);
			min_ex = excess(k+1);
		}else {
			kp = blk.min_excess_pos((r/blksize)*blksize, r);
			if (kp != BP_block::NOTFOUND) {
				k = kp;
				min_ex = excess(k+1);
			}
		}
		kp = blk.min_excess_pos(l, (l/blksize+1)*blksize);
		if (kp != BP_block::NOTFOUND && excess(kp+1) < min_ex) {
			k = kp;
		}
		return k;
	}

};




} //namespace
