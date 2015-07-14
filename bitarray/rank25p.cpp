
#include "rank25p.h"

namespace mscds {

void Rank25pBuilder::build_aux(const BitArrayInterface * b, Rank25pAux *o) {
	o->bits = b;
	uint64_t nc = ((o->bits->length() + 511) / 512) * 2;
	o->inv = BitArrayBuilder::create(nc * 64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits->word_count()) {
		o->inv.setword(pos, cnt);
		cnt += o->bits->popcntw(i);
		for (unsigned int j = 1; j < 8; j++) {
			uint64_t v = o->inv.word(pos + 1);
			v |= (cnt - o->inv.word(pos)) << 9 * (j - 1);
			o->inv.setword(pos + 1, v);
			if (i + j < o->bits->word_count()) cnt += o->bits->popcntw(i + j);
		}
		i += 8; pos += 2;
	}
	o->onecnt = cnt;
}

void Rank25pBuilder::build(const BitArray &b, Rank25p *o) {
	o->_own_bits = b;
	build_aux(&o->_own_bits, o);
}


/** first word: rank(p); second word: 7 sub-blocks, each uses 9 bits */
uint64_t Rank25pAux::rank(const uint64_t p) const {
	assert(p <= bits->length());
	if (p == bits->length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 8) & ~1ull;
	return inv.word(blk)
		+ ((inv.word(blk + 1) >>  (((wpos & 7) - 1) & 7) * 9) & 0x1FF)
		+ word_rank(wpos, p & 63);
}

uint64_t Rank25pAux::select(uint64_t r) const {
	assert(r < onecnt);
	r += 1;
	uint64_t start = 0, end = bits->length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rank(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}

uint64_t Rank25pAux::selectzero(uint64_t r) const {
	assert(r < bits->length() - onecnt);
	r += 1;
	uint64_t start = 0, end = bits->length();
	while (start < end) {
		uint64_t mid = (start + end) / 2;
		if (rankzero(mid) < r)
			start = mid + 1;
		else end = mid;
	}
	return start > 0 ? start - 1 : 0;
}

unsigned int Rank25pAux::word_rank(size_t idx, unsigned int i) const {
	return (i != 0) ? popcnt(bits->word(idx) & ((1ULL << i) - 1)) : 0;
}

void Rank25pAux::save_aux(OutArchive &ar) const {
	ar.startclass("Rank25pAux", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
	inv.save(ar);
	ar.var("onecnt").save(onecnt);
	ar.endclass();
}

void Rank25pAux::load_aux(InpArchive &ar, const BitArrayInterface * bits) {
	ar.loadclass("Rank25pAux");
	size_t blen;
	ar.var("bit_len").load(blen);
	inv.load(ar);
	ar.var("onecnt").load(onecnt);
	ar.endclass();
	this->bits = bits;
	if (bits->length() != blen) throw std::runtime_error("length mismatch");
}

void Rank25p::load(InpArchive &ar) {
	ar.loadclass("Rank25p");
	_own_bits.load(ar.var("bits"));
	load_aux(ar, &_own_bits);
	ar.endclass();
}

void Rank25p::save(OutArchive &ar) const {
	ar.startclass("Rank25p");
	_own_bits.save(ar.var("bits"));
	save_aux(ar);
	ar.endclass();
}

void mscds::Rank25p::clear() {
	_own_bits.clear();
	Rank25pAux::clear();
}

}//namespace
