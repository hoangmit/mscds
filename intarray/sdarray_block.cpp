#include "sdarray_block.h"

namespace mscds {

void SDArrayBlock::add(ValueType v) {
	vals.push_back(v);
}

void SDArrayBlock::saveBlock(OBitStream *bits) {
	assert(vals.size() <= BLKSIZE);
	if (vals.size() == 0) return;
	while (vals.size() < BLKSIZE) vals.push_back(0);

	for (size_t p = 1; p < vals.size(); ++p)
		vals[p] += vals[p - 1];
	uint64_t width = ceillog2(1 + vals.back() / vals.size());
	assert(width < (1ULL << 7));

	//higher bits' hints
	uint64_t select_hints = 0;
	const unsigned int step = (BLKSIZE + SUBB_PER_BLK - 1) / SUBB_PER_BLK;
	size_t i = step;
	for (size_t p = 0; p < SUBB_PER_BLK - 1; ++p) {
		uint64_t hp = ((vals[i - 1] >> width) + i - 1);
		assert(ceillog2(hp) <= 10);
		select_hints |= (hp << (p * 10));
		assert(p * 10 <= 64);
		i += step;
	}

	bits->puts(width, 7);
	bits->puts(select_hints);

	//lower bits
	for (size_t p = 0; p < vals.size(); ++p)
		bits->puts(vals[p], width); /* will be masked inside */

	//higer bits
	size_t j = 0;
	for (size_t p = 0; p < vals.size(); p++) {
		size_t pos = (vals[p] >> width) + p;
		while (j < pos) { bits->put0(); ++j; }
		bits->put1(); ++j;
	}
	vals.clear();
}

void SDArrayBlock::loadBlock(const BitRange& br) {
	loadBlock(*br.ba, br.start, br.len);
}

void SDArrayBlock::loadBlock(const BitArray &ba, size_t pt, size_t len) {
	if (pt != lastpt) {
		this->bits = ba;
		width = ba.bits(pt, 7);
		select_hints = ba.bits(pt + 7, 64);
		blkptr = pt + 7 + 64;
		lastpt = pt;
	}
}

SDArrayBlock::ValueType SDArrayBlock::prefixsum(unsigned int p) const {
	if (p == 0) return 0;
	ValueType lo = (width > 0) ? bits.bits(blkptr + width * (p - 1), width) : 0;
	ValueType hi = select_hi(select_hints, blkptr + width*BLKSIZE, p - 1) + 1 - p;
	return ((hi << width) | lo);
}

SDArrayBlock::ValueType SDArrayBlock::lookup(unsigned int off) const {
	uint64_t prev = 0;
	int64_t prehi = 0;
	if (off > 0) {
		ValueType prelo = bits.bits(blkptr + width * (off - 1), width);
		prehi = select_hi(select_hints, blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = bits.bits(blkptr + width * off, width);
	//uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	ValueType hi = prehi + scan_hi_next(blkptr + width*BLKSIZE + prehi + off);
	ValueType cur = ((hi << width) | lo);
	return cur - prev;
	//return sum + prev;
}

SDArrayBlock::ValueType SDArrayBlock::lookup(unsigned int off, ValueType &prev_sum) const {
	ValueType prev = 0;
	ValueType prehi = 0;
	if (off > 0) {
		ValueType prelo = bits.bits(blkptr + width * (off - 1), width);
		prehi = select_hi(select_hints, blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = bits.bits(blkptr + width * off, width);
	//uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	ValueType hi = prehi + scan_hi_next(blkptr + width*BLKSIZE + prehi + off);
	ValueType cur = ((hi << width) | lo);
	prev_sum = prev;
	return cur - prev;
}

unsigned int SDArrayBlock::rank(ValueType val) const {
	ValueType vlo = val & ((1ull << width) - 1);
	ValueType vhi = val >> width;
	uint32_t hipos = 0, rank = 0;
	if (vhi > 0) {
		hipos = select_zerohi(select_hints, blkptr + width*BLKSIZE, vhi - 1) + 1;
		//assert(scan_zerohi_bitslow(blkptr + width*BLKSIZE, vhi-1) + 1 == hipos);
		rank = hipos - vhi;
	}
	ValueType curlo = 0;
	while (rank < BLKSIZE && bits.bit(blkptr + width*BLKSIZE + hipos)) {
		curlo = bits.bits(blkptr + width * rank, width);
		if (curlo >= vlo)
			return rank + 1;
		++rank;
		++hipos;
	}
	return rank + 1;
}

unsigned int SDArrayBlock::select_hi(uint64_t hints, uint64_t start, uint32_t off) const {
	unsigned int subblkpos = off / SUBB_SIZE;
	uint32_t res = off % SUBB_SIZE;
	if (res == SUBB_SIZE - 1)
		return getBits(hints, subblkpos * 10, 10);
	unsigned int gb = subblkpos > 0 ? getBits(hints, (subblkpos - 1) * 10, 10) + 1 : 0;
	return scan_hi_bits(start + gb, res) + gb;
}

unsigned int SDArrayBlock::scan_hi_bits(uint64_t start, uint32_t res) const {
	uint64_t wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = bits.word(wpos) >> (start & 63);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return selectword(word, res);
		res -= bitcnt;
		++wpos;
	}
	do {
		uint64_t word = bits.word(wpos);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return (wpos << 6) - start + selectword(word, res);
		res -= bitcnt;
		++wpos;
	} while (true);
}

uint64_t SDArrayBlock::getBits(uint64_t x, uint64_t beg, uint64_t num) {
	return (x >> beg) & ((1ULL << num) - 1);
}

unsigned int SDArrayBlock::scan_hi_next(unsigned int start) const {
	unsigned int wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = bits.word(wpos) >> (start & 63);
		if (word != 0) return lsb_intr(word);// selectword(word, res);
		++wpos;
	}
	do {
		uint64_t word = bits.word(wpos);
		if (word != 0) return (wpos << 6) - start + lsb_intr(word); //selectword(word, res);
		++wpos;
	} while (true);
}

unsigned int SDArrayBlock::select_zerohi(uint64_t hints, uint64_t start, uint32_t off) const {
	uint64_t sblk = 0;
	for (; sblk < 6; ++sblk) {
		uint64_t sbpos = getBits(hints, sblk * 10, 10);
		if (sbpos - (sblk + 1) * SUBB_SIZE + 1 >= off) break;
	}
	unsigned int res = off, sbpos = 0;
	if (sblk > 0) {
		sbpos = getBits(hints, (sblk - 1) * 10, 10) + 1;
		res -= sbpos - sblk * SUBB_SIZE;
	}
	return sbpos + scan_hi_zeros(start + sbpos, res);
}

unsigned int SDArrayBlock::scan_hi_zeros(unsigned int start, uint32_t res) const {
	unsigned int wpos = start >> 6;
	if ((start & 63) != 0) {
		uint64_t word = (~bits.word(wpos)) >> (start & 63);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return selectword(word, res);
		res -= bitcnt;
		++wpos;
	}
	do {
		uint64_t word = ~bits.word(wpos);
		uint32_t bitcnt = popcnt(word);
		if (bitcnt > res) return (wpos << 6) - start + selectword(word, res);
		res -= bitcnt;
		++wpos;
	} while (true);
}

void SDArrayFuseBuilder::register_struct() {
	sid = bd.register_summary(16, 8); // bytes
	did = bd.register_data_block();
	cnt = 0;
	sum = 0;
	blkcnt = 0;
	lastsum = 0;
}

void SDArrayFuseBuilder::add(uint64_t val) {
	blk.add(val);
	sum += val;
	cnt++;
	blkcnt++;
	assert(blkcnt <= blk.BLKSIZE);
}

void SDArrayFuseBuilder::set_block_data() {
	if (blkcnt > 0) {
		uint64_t v = lastsum;
		bd.set_summary(sid, MemRange::wrap(v));
		OBitStream& d1 = bd.start_data(did);
		blk.saveBlock(&d1);
		bd.end_data();
		blkcnt = 0;
		lastsum = sum;
	}
}

void SDArrayFuseBuilder::build_struct() {
	set_block_data();
	struct {
		uint64_t cnt, sum;
	} data;
	data.cnt = cnt;
	data.sum = sum;
	bd.set_global(sid, MemRange::wrap(data));
}

void SDArrayFuseBuilder::deploy(SDArrayFuse *out) {
	out->sid = sid;
	out->did = did;
}

}

