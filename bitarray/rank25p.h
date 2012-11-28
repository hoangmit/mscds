#pragma once

#include "bitarray.h"


namespace mscds {

class rank25p_builder;

class rank25p {
	BitArray inv, bits;
	uint64_t onecnt;
public:
	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t p) const;
	uint64_t one_count() const { return onecnt; }
private:
	unsigned int word_rank(size_t idx, unsigned int i) const;
	friend class rank25p_builder;
};

class rank25p_builder {
public:
	void build(rank25p * o, BitArray& b);
};

void rank25p_builder::build(rank25p * o, BitArray& b) {
	o->bits = b;
	uint64_t nc = ((o->bits.length() + 511) / 512) * 2;
	o->inv = BitArray::create(nc*64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits.word_count()) {
		o->inv.word(pos) = cnt;
		cnt += popcnt(o->bits.word(i));
		for(unsigned int j = 1;  j < 8; j++) {
			o->inv.word(pos + 1) |= (cnt - o->inv.word(pos)) << 9 * (j - 1);
			if (i + j < o->bits.word_count()) cnt += popcnt(o->bits.word(i + j));
		}
		i += 8; pos += 2;
	}
	o->onecnt = cnt;
}

/* first word: rank(p); second word: 7 sub-blocks, each uses 9 bits */

uint64_t rank25p::rank(const uint64_t p) const {
	assert(p <= bits.length());
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 8) & ~1ull;
	return inv.word(blk)
		+ ((inv.word(blk + 1) >>  (((wpos & 7) - 1) & 7) * 9) & 0x1FF)
		+ word_rank(wpos, p & 63);
}

unsigned int rank25p::word_rank(size_t idx, unsigned int i) const {
	return (i != 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}


}//namespace