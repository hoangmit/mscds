#pragma once

#include <stdint.h>
#include "bitarray/bitarray.h"
#include "bitarray/rank6p.h"
#include "bitarray/sdarray.h"
#include "bitarray/sdarray.h"
#include <algorithm>

namespace mscds {

struct BP_block {
	BP_block():blksize(0), bp(NULL) {}
	BP_block(BitArray _bp, unsigned int _blksize): bp(_bp), blksize(_blksize) {}
	//BP_block& operator=(const BP_block& other) {bp = other.bp; blksize = other.blksize; return * this;}

	void init(BitArray& _bp, unsigned int _blksize) {
		this->bp = _bp;
		blksize = _blksize;
	}

	BitArray bp;
	unsigned int blksize;
	static int8_t min_excess8_t[256], excess8_t[256];

	static uint8_t revbits(uint8_t c) {
		return (c * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	}

	static int8_t min_excess8_c(uint8_t c);
	static int8_t min_revex8_c(uint8_t c);
	static int8_t excess8_c(uint8_t c);

	static const uint64_t NOTFOUND = -1ull;

	uint64_t forward_scan(uint64_t pos, int64_t excess) const;
	uint64_t backward_scan(uint64_t pos, int64_t excess) const;
	uint64_t min_excess_pos(uint64_t l, uint64_t r) const;

	// find [pos .. x]
	uint64_t forward_scan_slow(uint64_t pos, int64_t excess) const;

	// find [x .. pos]
	uint64_t backward_scan_slow(uint64_t pos, int64_t excess) const;
	void clear() {
		blksize = 0;
		bp = NULL;
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
	//uint64_t find_close(uint64_t) const;
	//uint64_t find_open(uint64_t) const;
	//uint64_t enclose(uint64_t) const;

	void build(BitArray& bp, unsigned int blksize = 128) {
		assert(blksize >= 4);
		this->blksize = blksize;
		bp_bits = bp;
		blk.init(bp, blksize);
		Rank6pBuilder::build(bp, &bprank);
		if (bp.length() > blksize) {
			std::vector<uint64_t> pio = find_pioneers_v(bp, blksize);
			//pioneer_map
			pioneer_map.build(pio);
			//nextlvl
			BitArray nxtlvl = BitArray::create(pio.size());
			for (size_t i = 0; i < pio.size(); i++)
				nxtlvl.setbit(i, bp[pio[i]]);
			lowerlvl = new BP_aux();
			lowerlvl->build(nxtlvl, blksize);
		} else {
			lowerlvl = NULL;
		}
	}
	~BP_aux() {clear();}

	void clear() {
		blk.clear();
		bprank.clear();
		bp_bits.clear();
		pioneer_map.clear();
		delete lowerlvl;
		blksize = 0;
	}
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
		if (bp_bits[p]) return find_close(p);
		else return find_open(p);
	}

	bool bit(uint64_t p) const {
		return bp_bits[p];
	}

	uint64_t find_close(uint64_t p) const {
		assert(p < length());
		if (!bit(p)) return p;
		uint64_t pc = blk.forward_scan(p, 0);
		if (pc != BP_block::NOTFOUND) return pc;
		assert(length() > blksize);
		uint64_t pio_rank = pioneer_map.rank(p+1) - 1;
		uint64_t close_pio_rank = lowerlvl->find_close(pio_rank);
		uint64_t pio_close = pioneer_map.select(close_pio_rank);
		assert(!bp_bits[pio_close]);
		int64_t diff = excess(p) - excess((pio_close/blksize)*blksize);
		uint64_t ret = blk.forward_scan((pio_close/blksize) * blksize, diff);
		assert(!bp_bits[ret]);
		return ret;
	}

	uint64_t find_open(uint64_t p) const {
		assert(p < length());
		if (bit(p)) return p;
		uint64_t po = blk.backward_scan(p, 0);
		if (po != BP_block::NOTFOUND) return po;
		assert(length() > blksize);
		uint64_t pio_rank = pioneer_map.rank(p);
		uint64_t open_pio_rank = lowerlvl->find_open(pio_rank);
		uint64_t open_pio = pioneer_map.select(open_pio_rank);
		assert(bp_bits[open_pio]);
		int64_t diff = excess((open_pio/blksize + 1)*blksize) - excess(p + 1);
		uint64_t ret = blk.backward_scan((open_pio/blksize + 1) * blksize - 1, diff);
		assert(bit(ret));
		return ret;
	}

	uint64_t enclose(uint64_t p) const {
		if (p >= length()) return BP_block::NOTFOUND;
		if (!bit(p)) return find_open(p);
		if (p == 0) return BP_block::NOTFOUND;
		uint64_t pe = BP_block::NOTFOUND;
		if (p % blksize != 0) pe = blk.backward_scan(p - 1, 1);
		if (pe != BP_block::NOTFOUND || p / blksize == 0) return pe;
		uint64_t pio_rank = pioneer_map.rank(p);
		uint64_t open_pio_rank = lowerlvl->enclose(pio_rank);
		if (open_pio_rank == BP_block::NOTFOUND) return BP_block::NOTFOUND;
		uint64_t open_pio = pioneer_map.select(open_pio_rank);
		assert(bp_bits[open_pio]);
		int64_t diff = excess((open_pio/blksize + 1)*blksize) - excess(p) + 1;
		uint64_t ret = blk.backward_scan((open_pio/blksize + 1) * blksize - 1, diff);
		assert(bit(ret));
		return ret;
	}

	uint64_t rr_enclose(uint64_t i, uint64_t j) const {
		assert(i < j && j < length());
		assert(bit(i) && bit(j));
		return min_excess_pos(find_close(i) + 1, j);
	}

	uint64_t min_excess_pos(uint64_t l, uint64_t r) const {
		if (l >= r) return BP_block::NOTFOUND;
		uint64_t k = BP_block::NOTFOUND;
		if (l/blksize == r/blksize) return blk.min_excess_pos(l, r);
		uint64_t min_ex = excess(r) + 2 * (!bp_bits[r] ? 1 : 0);
		uint64_t pl = pioneer_map.rank(l);
		uint64_t pr = pioneer_map.rank(r);
		uint64_t kp = lowerlvl->min_excess_pos(pl, pr);
		if (kp != BP_block::NOTFOUND) {
			k = pioneer_map.select(kp);
			min_ex = excess(k);
		}else {
			kp = blk.min_excess_pos((r/blksize)*blksize, r);
			if (k != BP_block::NOTFOUND) {
				k = kp;
				min_ex = excess(k);
			}
		}
		kp = blk.min_excess_pos(l, (l/blksize+1)*blksize);
		if (kp != BP_block::NOTFOUND && excess(kp) < min_ex) {
			kp = k;
		}
		return k;
	}

};




} //namespace
