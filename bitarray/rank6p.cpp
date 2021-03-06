#include "rank6p.h"
#include "bitop.h"
//#include "bitstream.h"
#include <stdexcept>
#include <algorithm>

namespace mscds {

uint64_t Rank6pBuilder::popcntwz(const BitArrayInterface* v, size_t idx) {
	if (idx < v->word_count()) return v->popcntw(idx);
	else return 0;
}

void Rank6pBuilder::build_aux(const BitArrayInterface *b, Rank6pAux * o) {
	assert(b->length() <= (1ULL << 50));
	o->bits = b;
	uint64_t nc = ((o->bits->length() + 2047) / 2048) * 2;
	o->inv = BitArrayBuilder::create(nc*64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits->word_count()) {
		o->inv.setword(pos, cnt);
		for (unsigned int k = 0; k < 4; k++)
			cnt += popcntwz(o->bits, i+k);

		uint64_t overflow = 0;
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t val = cnt - o->inv.word(pos);
			uint64_t v2 = o->inv.word(pos + 1);
			v2 |= (val & 0x1FF) << 9 * (j - 1);
			o->inv.setword(pos + 1, v2);
			overflow |= ((val >> 9) & 3) << 2 * (j - 1);
			for (unsigned int k = 0; k < 4; k++)
				cnt += popcntwz(o->bits, i + j * 4 + k);
		}
		uint64_t v = o->inv.word(pos);
		v |= overflow << 50;
		o->inv.setword(pos, v);
		i += 8*4; pos += 2;
	}
	o->onecnt = cnt;
}

void Rank6pBuilder::build(const BitArray &b, Rank6p *o) {
	o->own_bits = b;
	build_aux(&o->own_bits, o);
}

/*
void Rank6pBuilder::build(const BitArray &b, OutArchive &ar) {
	ar.startclass("Rank6p", 1);
	assert(b.length() <= (1ULL << 50));
	ar.var("bit_len").save(b.length());

	uint64_t nc = ((b.length() + 2047) / 2048) * 2; // every 2048 bits is a block
	ar.var("inventory");
	BitArraySeqBuilder bd(nc, ar);

	uint64_t w1 = 0, w2 = 0;

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < b.word_count()) {
		w1 = cnt;
		for (unsigned int k = 0; k < 4; k++)
			cnt += popcnt(getwordz(b, i+k));

		uint64_t overflow = 0;
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t val = cnt - w1;
			w2 |= (val & 0x1FF) << 9 * (j - 1);
			overflow |= ((val >> 9) & 3) << 2 * (j - 1);
			for (unsigned int k = 0; k < 4; k++)
				cnt += popcnt(getwordz(b, i + j * 4 + k));
		}
		w1 |= overflow << 50;
		i += 8*4; pos += 2;
		bd.addword(w1);
		bd.addword(w2);
		w1 = 0; w2 = 0;
	}
	bd.done();
	ar.var("onecnt").save(cnt);
	ar.endclass();
}*/

void Rank6pAux::save_aux(OutArchive &ar) const {
	ar.startclass("Rank6p_auxiliary", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
	inv.save(ar);
	ar.var("onecnt").save(onecnt);
	ar.endclass();
}

void Rank6pAux::load_aux(InpArchive &ar, const BitArrayInterface* b) {
	ar.loadclass("Rank6p_auxiliary");
	size_t blen;
	ar.var("bit_len").load(blen);
	inv.load(ar);
	ar.var("onecnt").load(onecnt);
	ar.endclass();
	if (b->length() != blen) throw std::runtime_error("length mismatch");
	//if (b->count_one() != onecnt) throw std::runtime_error("count_one mismatch");
	this->bits = b;
}

void Rank6p::save(OutArchive &ar) const {
	/*ar.startclass("Rank6p", 1);       // declare a new data structure class (for error checking)
	ar.var("bit_len").save(length()); // save integer variable
	ar.var("inventory");              // declare the name of a sub-data-structure
	inv.save(ar);                     // save the sub-data-structure. The sub-data-structure needs to implement "save"
	ar.var("onecnt").save(onecnt);    // another way of saving a integer variable
	bits.save(ar.var("bits"));   // another way of saving a sub-data-structure
	ar.endclass();                    // end data structure class (for error checking)
	*/
	ar.startclass("Rank6p");
	own_bits.save(ar.var("bits"));
	save_aux(ar);
	ar.endclass();
}

void Rank6p::clear() {
	own_bits.clear();
	Rank6pAux::clear();
}

void Rank6p::load(InpArchive &ar) {
	/*ar.loadclass("Rank6p");
	size_t blen;
	ar.var("bit_len").load(blen);
	inv.load(ar);
	ar.var("onecnt").load(onecnt);
	own_bits.load(ar.var("bits"));
	ar.endclass();
	bits = &own_bits;
	if (bits->length() != blen)
		throw std::runtime_error("length mismatch");*/
	ar.loadclass("Rank6p");
	own_bits.load(ar.var("bits"));
	load_aux(ar, &own_bits);
	ar.endclass();
}

bool Rank6pAux::bit(uint64_t p) const {
	return bits->bit(p);
}

uint64_t Rank6pAux::rank(const uint64_t p) const {
	assert(p <= bits->length());
	if (p == bits->length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 10) & ~1ull;

	uint64_t val = (inv.word(blk) & 0x3FFFFFFFFFFFFULL) + subblkrank(blk, ((p >> 8) & 7ULL));
	for (unsigned int i = 0; i < (unsigned int) (wpos & 3ULL); ++i)
		val += bits->popcntw(i + (wpos & ~3ULL));
	return val + word_rank(wpos, p & 63ULL);
}

uint64_t Rank6pAux::rankzero(uint64_t p) const {
	return p - rank(p);
}

uint64_t Rank6pAux::blkrank(size_t blk) const {
	return inv.word(2*blk) & 0x3FFFFFFFFFFFFULL;
}

uint64_t Rank6pAux::subblkrank(size_t blk, unsigned int off) const {
	// off = [0..7]
	const unsigned int hi = inv.word(blk) >> 50;
	off = (off - 1) & 7;
	uint64_t subblk_rank = (inv.word(blk + 1) >>  off * 9) & 0x1FFULL;
	subblk_rank |= ((hi >> off * 2) & 3ULL) << 9;
	return subblk_rank;
}

uint64_t Rank6pAux::select(const uint64_t r) const {
	assert(r < onecnt);
	//TODO: change to use upper_bound
	uint64_t end = inv.word_count() / 2;
	uint64_t lo = 0, len = end;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (blkrank(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else
			len = d;
	}
	
	if (lo >= end || r < blkrank(lo))
		lo -= 1;
	//skip 0 blocks
	while (lo + 1 < end && r >= blkrank(lo+1)) lo++;
	return selectblock(lo, r - blkrank(lo));
}

uint64_t Rank6pAux::selectblock(uint64_t blk, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank(blk*2, i) <= d) j = i;
		else break;
	d = d - subblkrank(blk*2, j);
	uint64_t widx = blk * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr = bits->popcntw(widx);
		if (d < wr)
			return blk * 2048 + 256* j + 64 * k + selectword(bits->word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	assert(false);
	return ~0ull;
}

uint64_t Rank6pAux::blkrank0(size_t blk) const {
	return blk * 2048 - (inv.word(blk*2) & 0x3FFFFFFFFFFFFULL);
}

uint64_t Rank6pAux::subblkrank0(size_t blk, unsigned int off) const {
	return off*4*64 - subblkrank(blk, off);
}

uint64_t Rank6pAux::selectzero(const uint64_t r) const {
	assert(r < bits->length() - onecnt);
	//TODO: change to use upper_bound
	uint64_t end = inv.word_count() / 2;
	uint64_t lo = 0, len = end;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (blkrank0(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else 
			len = d;
	}
	if (lo >= inv.word_count() / 2 || r < blkrank0(lo))
		lo -= 1;
	while (lo + 1 < end && r >= blkrank0(lo+1)) lo++;
	return selectblock0(lo, r - blkrank0(lo));
}

uint64_t Rank6pAux::selectblock0(uint64_t lo, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank0(lo*2, i) <= d) j = i;
		else break;
	d = d - subblkrank0(lo*2, j);
	uint64_t widx = lo * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr = 64-bits->popcntw(widx);
		if (d < wr)
			return lo * 2048 + 256* j + 64 * k + selectword(~bits->word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	assert(false);
	return ~0ull;
}

void Rank6pAux::clear() {
	inv.clear();
	bits = nullptr;
	onecnt = 0;
}

unsigned int Rank6pAux::word_rank(size_t idx, unsigned int i) const {
	return (i > 0) ? popcnt(bits->word(idx) & ((1ULL << i) - 1)) : 0;
}

//------------------------------------------------------------------------

struct BlockIntIterator {
	const Rank6pAux& r;
	uint64_t p;
	BlockIntIterator(const Rank6pAux& _r): r(_r), p(0) {}
	void operator++() { ++p; }
	uint64_t operator*() const { return r.blkrank(p); }
};

void Rank6pHintSel::init() {
	hints.clear();
	BlockIntIterator it(rankst);
	// every 4096 positions
	hints = bsearch_hints(it, rankst.inv.word_count()/2, rankst.one_count(), 12); 
}

void Rank6pHintSel::init(Rank6p& r) {
	hints.clear();
	this->rankst = r;
	init();
}

void Rank6pHintSel::init(BitArray& b) {
	hints.clear();
	//Rank6pBuilder bd;
	Rank6pBuilder::build(b, &rankst);
	init();
}

uint64_t Rank6pHintSel::select(uint64_t r) const {
	assert(r < rankst.one_count());
	//TODO: change to use upper_bound
	uint64_t lo = hints[r >> 12]; //  % 4096
	uint64_t end = hints[(r >> 12) + 1];
	uint64_t len = end - lo;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (rankst.blkrank(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else
			len = d;
	}
	if (lo >= rankst.inv.word_count() / 2 || r < rankst.blkrank(lo))
		lo--;
	while (lo + 1 < end && r >= rankst.blkrank(lo + 1)) lo++;
	return rankst.selectblock(lo, r - rankst.blkrank(lo));
}

void Rank6pHintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_
