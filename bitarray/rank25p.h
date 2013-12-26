#pragma once

#include "bitarray.h"
#include "bitop.h"
#include "rankselect.h"

namespace mscds {

class Rank25pBuilder;

class Rank25p : public RankSelect {
	BitArray inv, bits;
	uint64_t onecnt;
public:
	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t p) const;
	uint64_t one_count() const { return onecnt; }
	bool access(uint64_t pos) const { return bits[pos]; }
	uint64_t length() const { return bits.length(); }
	uint64_t selectzero(uint64_t r) const;
	typedef Rank25pBuilder BuilderTp;
	void clear() { onecnt = 0; inv.clear(); bits.clear(); }

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
private:
	unsigned int word_rank(size_t idx, unsigned int i) const;
	friend class Rank25pBuilder;
};

class Rank25pBuilder {
public:
	static void build(const BitArray& b, Rank25p * o);
	typedef Rank25p QueryTP;
};

inline void Rank25pBuilder::build(const BitArray& b, Rank25p * o) {
	o->bits = b;
	uint64_t nc = ((o->bits.length() + 511) / 512) * 2;
	o->inv = BitArrayBuilder::create(nc * 64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits.word_count()) {
		o->inv.setword(pos, cnt);
		cnt += popcnt(o->bits.word(i));
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t v = o->inv.word(pos + 1);
			v |= (cnt - o->inv.word(pos)) << 9 * (j - 1);
			o->inv.setword(pos + 1, v);
			if (i + j < o->bits.word_count()) cnt += popcnt(o->bits.word(i + j));
		}
		i += 8; pos += 2;
	}
	o->onecnt = cnt;
}

/* first word: rank(p); second word: 7 sub-blocks, each uses 9 bits */
inline uint64_t Rank25p::rank(const uint64_t p) const {
	assert(p <= bits.length());
	if (p == bits.length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 8) & ~1ull;
	return inv.word(blk)
		+ ((inv.word(blk + 1) >>  (((wpos & 7) - 1) & 7) * 9) & 0x1FF)
			+ word_rank(wpos, p & 63);
}

inline uint64_t Rank25p::select(uint64_t r) const {
	assert(r < onecnt);
	r += 1;
	uint64_t start = 0, end = bits.length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rank(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}

inline uint64_t Rank25p::selectzero(uint64_t r) const {
	assert(r < bits.length() - onecnt);
	r += 1;
	uint64_t start = 0, end = bits.length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rankzero(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}

inline unsigned int Rank25p::word_rank(size_t idx, unsigned int i) const {
	return (i != 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}


inline void Rank25p::save(OutArchive &ar) const {
	ar.startclass("Rank25p", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
	inv.save(ar);
	ar.var("onecnt").save(onecnt);
	bits.save(ar.var("bits"));
	ar.endclass();
}

inline void Rank25p::load(InpArchive &ar) {
	ar.loadclass("Rank25p");
	size_t blen;
	ar.var("bit_len").load(blen);
	inv.load(ar);
	ar.var("onecnt").load(onecnt);
	bits.load(ar.var("bits"));
	ar.endclass();
	if (bits.length() != blen) throw std::runtime_error("length mismatch");
}


}//namespace
