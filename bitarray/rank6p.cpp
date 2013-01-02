#include "rank6p.h"
#include "bitop.h"
//#include "bitstream.h"
#include <stdexcept>
#include <algorithm>

namespace mscds {

uint64_t Rank6pBuilder::getwordz(const BitArray& v, size_t idx) {
	if (idx < v.word_count()) return v.word(idx);
	else return 0;
}

void Rank6pBuilder::build(const BitArray& b, Rank6p * o) {
	assert(b.length() <= (1ULL << 50));
	o->bits = b;
	uint64_t nc = ((o->bits.length() + 2047) / 2048) * 2;
	o->inv = BitArray::create(nc*64);
	o->inv.fillzero();

	uint64_t cnt = 0;
	uint64_t pos = 0;
	uint64_t i = 0;
	while (i < o->bits.word_count()) {
		o->inv.word(pos) = cnt;
		for (unsigned int k = 0; k < 4; k++)
			cnt += popcnt(getwordz(o->bits, i+k));

		uint64_t overflow = 0;
		for(unsigned int j = 1;  j < 8; j++) {
			uint64_t val = cnt - o->inv.word(pos);
			o->inv.word(pos + 1) |= (val & 0x1FF) << 9 * (j - 1);
			overflow |= ((val >> 9) & 3) << 2 * (j - 1);
			for (unsigned int k = 0; k < 4; k++)
				cnt += popcnt(getwordz(o->bits, i + j * 4 + k));
		}
		o->inv.word(pos) |= overflow << 50;
		i += 8*4; pos += 2;
	}
	o->onecnt = cnt;
}

void Rank6pBuilder::build(const BitArray &b, OArchive &ar) {
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
}

void Rank6p::save(OArchive &ar) const {
	ar.startclass("Rank6p", 1);
	ar.var("bit_len").save(length());
	ar.var("inventory");
	inv.save(ar);
	ar.var("onecnt").save(onecnt);
	ar.endclass();
}

void Rank6p::load(IArchive &ar, BitArray &b) {
	ar.loadclass("Rank6p");
	size_t blen;
	ar.var("bit_len").load(blen);
	if (b.length() != blen) throw std::runtime_error("length mismatch");
	//uint64_t nc = ((blen + 2047) / 2048) * 2;
	inv.load(ar);
	this->bits = b;
	ar.var("onecnt").load(onecnt);
	ar.endclass();
}

bool Rank6p::bit(uint64_t p) const {
	return bits.bit(p);
}

uint64_t Rank6p::rank(const uint64_t p) const {
	assert(p <= bits.length());
	if (p == bits.length()) return onecnt;
	const uint64_t wpos = p >> 6; // div 64
	const uint64_t blk = (p >> 10) & ~1ull;

	uint64_t val = (inv.word(blk) & 0x3FFFFFFFFFFFFULL) + subblkrank(blk, ((p >> 8) & 7ULL));
	for (unsigned int i = 0; i < (unsigned int) (wpos & 3ULL); ++i)
		val += popcnt(bits.word(i + (wpos & ~3ULL)));
	return val + word_rank(wpos, p & 63ULL);
}

uint64_t Rank6p::rankzero(uint64_t p) const {
	return p - rank(p);
}

uint64_t Rank6p::blkrank(size_t blk) const {
	return inv.word(2*blk) & 0x3FFFFFFFFFFFFULL;
}

uint64_t Rank6p::subblkrank(size_t blk, unsigned int off) const {
	// off = [0..7]
	const unsigned int hi = inv.word(blk) >> 50;
	off = (off - 1) & 7;
	uint64_t subblk_rank = (inv.word(blk + 1) >>  off * 9) & 0x1FFULL;
	subblk_rank |= ((hi >> off * 2) & 3ULL) << 9;
	return subblk_rank;
}

uint64_t Rank6p::select(const uint64_t r) const {
	assert(r < onecnt);
	uint64_t lo = 0, len = inv.word_count() / 2;
	while (len > 0) {
		uint64_t d = len / 2;
		uint64_t mid = lo + d;
		if (blkrank(mid) < r) {
			lo = mid + 1;
			len -= d + 1;
		}else
			len = d;
	}
	if (lo >= inv.word_count() / 2 || r < blkrank(lo))
		lo -= 1;
	return selectblock(lo, r - blkrank(lo));
}

uint64_t Rank6p::selectblock(uint64_t blk, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank(blk*2, i) <= d) j = i;
		else break;
	d = d - subblkrank(blk*2, j);
	uint64_t widx = blk * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr = popcnt(bits.word(widx));
		if (d < wr)
			return blk * 2048 + 256* j + 64 * k + selectword(bits.word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	return ~0ull;
}

uint64_t Rank6p::blkrank0(size_t blk) const {
	return blk * 2048 - (inv.word(blk*2) & 0x3FFFFFFFFFFFFULL);
}

uint64_t Rank6p::subblkrank0(size_t blk, unsigned int off) const {
	return off*4*64 - subblkrank(blk, off);
}

uint64_t Rank6p::selectzero(const uint64_t r) const {
	assert(r < bits.length() - onecnt);
	uint64_t lo = 0, len = inv.word_count() / 2;
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
	return selectblock0(lo, r - blkrank0(lo));
}

uint64_t Rank6p::selectblock0(uint64_t lo, uint64_t d) const {
	unsigned int j = 0;
	for (unsigned int i = 0; i < 8; i++)
		if (subblkrank0(lo*2, i) <= d) j = i;
		else break;
	d = d - subblkrank0(lo*2, j);
	uint64_t widx = lo * 32 + j * 4;
	for (unsigned int k = 0; k < 4; k++)  {
		unsigned int wr =  popcnt(~bits.word(widx));
		if (d < wr)
			return lo * 2048 + 256* j + 64 * k + selectword(~bits.word(widx), d);
		else
			d -= wr;
		widx += 1;
	}
	return ~0ull;
}

void Rank6p::clear() {
	inv.clear();
	bits.clear();
	onecnt = 0;
}

unsigned int Rank6p::word_rank(size_t idx, unsigned int i) const {
	return (i > 0) ? popcnt(bits.word(idx) & ((1ULL << i) - 1)) : 0;
}

//------------------------------------------------------------------------

struct BlockIntIterator {
	const Rank6p& r;
	uint64_t p;
	BlockIntIterator(const Rank6p& _r): r(_r), p(0) {}
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
	uint64_t lo = hints[r >> 12]; //  % 4096
	uint64_t len = hints[(r >> 12) + 1] - lo;
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
	return rankst.selectblock(lo, r - rankst.blkrank(lo));
}

void Rank6pHintSel::clear() {
	hints.clear();
	rankst.clear();
}

}//namespace_