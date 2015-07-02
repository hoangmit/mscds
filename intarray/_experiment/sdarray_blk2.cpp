
#include "sdarray_blk2.h"

namespace mscds {

void SDArrayBlock2::add(ValueType v) {
	vals.push_back(v);
}

void SDArrayBlock2::saveBlock(OBitStream *bits) {
	assert(vals.size() <= BLKSIZE);
	if (vals.size() == 0) return;
	while (vals.size() < BLKSIZE) vals.push_back(0);
	same_value = true;
	for (size_t p = 1; p < vals.size(); ++p)
		if (vals[p] != vals[0]) { same_value = false; break; }

	if (same_value) {
		bits->put1();
		svalue = vals[0];
		uint16_t width = val_bit_len(svalue);
		assert(width < 128);
		bits->puts(width, 7);
		bits->puts(svalue, width);
		vals.clear();
		return ;
	} else {
		bits->put0();
	}

	for (size_t p = 1; p < vals.size(); ++p)
		vals[p] += vals[p - 1];
	uint64_t width = ceillog2(1 + vals.back() / vals.size());
	assert(width < 128);
	bits->puts(width, 7);

	//higher bits' hints
	uint64_t select_hints = 0;
    const unsigned int step = SUBB_SIZE;
    //assert(step == (BLKSIZE + SUBB_PER_BLK) / (SUBB_PER_BLK + 1));
	size_t i = step;
	for (size_t p = 0; p < SUBB_PER_BLK; ++p) {
		uint64_t hp = ((vals[i - 1] >> width) + i - 1);
		assert(val_bit_len(hp) <= H_WIDTH);		
		bits->puts(hp, H_WIDTH);
		i += step;
	}

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

void SDArrayBlock2::loadBlock(const BitRange& br) {
	loadBlock(br.ba, br.start, br.len);
}

void SDArrayBlock2::loadBlock(const BitArrayInterface *ba, size_t pt, size_t len) {
	if (pt != lastpt || ba != this->bits) {
		this->bits = ba;
		lastpt = pt;
		same_value = ba->bit(pt);
		width = ba->bits(pt+1, 7);
		if (same_value) {
			svalue = ba->bits(pt+8, width);
		} else {
			for (unsigned i = 0; i < SUBB_PER_BLK; ++i)
				hints[i] = ba->bits(pt + 8 + H_WIDTH * i, H_WIDTH);
			blkptr = pt + 8 + H_WIDTH * SUBB_PER_BLK;
		}
	}
}

SDArrayBlock2::ValueType SDArrayBlock2::prefixsum(unsigned int p) const {
	if (p == 0) return 0;
	if (!same_value) {
		ValueType lo = (width > 0) ? bits->bits(blkptr + width * (p - 1), width) : 0;
		ValueType hi = select_hi(hints, blkptr + width*BLKSIZE, p - 1) + 1 - p;
		return ((hi << width) | lo);
	} else {
		return svalue * p;
	}
}

SDArrayBlock2::ValueType SDArrayBlock2::lookup(unsigned int off) const {
	if (same_value) {
		return svalue;
	}
	uint64_t prev = 0;
	int64_t prehi = 0;
	if (off > 0) {
		ValueType prelo = bits->bits(blkptr + width * (off - 1), width);
		prehi = select_hi(hints, blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = bits->bits(blkptr + width * off, width);
	//uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	ValueType hi = prehi + bits->scan_next(blkptr + width*BLKSIZE + prehi + off);
	ValueType cur = ((hi << width) | lo);
	return cur - prev;
	//return sum + prev;
}

SDArrayBlock2::ValueType SDArrayBlock2::lookup(unsigned int off, ValueType &prev_sum) const {
	if (same_value) {
		prev_sum = off * svalue;
		return svalue;
	}
	ValueType prev = 0;
	ValueType prehi = 0;
	if (off > 0) {
		ValueType prelo = bits->bits(blkptr + width * (off - 1), width);
		prehi = select_hi(hints, blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = bits->bits(blkptr + width * off, width);
	//uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	ValueType hi = prehi + bits->scan_next(blkptr + width*BLKSIZE + prehi + off);
	ValueType cur = ((hi << width) | lo);
	prev_sum = prev;
	return cur - prev;
}

unsigned int SDArrayBlock2::rank(ValueType val) const {
    if (same_value) {
		if (val == 0) return 0;
		else if (val > svalue * BLKSIZE) return BLKSIZE + 1;
		else {
			if (svalue != 0) return (val + svalue - 1) / svalue;
			else
				if (val == 0) return 0;
		}
		return BLKSIZE+1;
	}
	ValueType vlo = val & ((1ull << width) - 1);
	ValueType vhi = val >> width;
	uint32_t hipos = 0, rank = 0;
	if (vhi > 0) {
		hipos = select_zerohi(hints, blkptr + width*BLKSIZE, vhi - 1) + 1;
		//assert(scan_zerohi_bitslow(blkptr + width*BLKSIZE, vhi-1) + 1 == hipos);
		rank = hipos - vhi;
	}
	if (rank > BLKSIZE) return BLKSIZE+1;
	ValueType curlo = 0;
	while (rank < BLKSIZE && bits->bit(blkptr + width*BLKSIZE + hipos)) {
		curlo = bits->bits(blkptr + width * rank, width);
		if (curlo >= vlo)
			return rank + 1;
		++rank;
		++hipos;
	}
	if (rank > BLKSIZE) return BLKSIZE+1;
	else return rank + 1;
}

unsigned int SDArrayBlock2::select_hi(const uint16_t* hints, uint64_t start, uint32_t off) const {
	unsigned int subblkpos = off / SUBB_SIZE;
	uint32_t res = off % SUBB_SIZE;
	//if (res == SUBB_SIZE - 1)
	//	return getBits(hints, subblkpos * 10, 10);
	unsigned int gb = subblkpos > 0 ? hints[subblkpos - 1] + 1 : 0;
	return bits->scan_bits(start + gb, res) + gb;
}

unsigned int SDArrayBlock2::select_zerohi(const uint16_t* hints, uint64_t start, uint32_t off) const {
	uint64_t sblk = 0;
	for (; sblk < SUBB_PER_BLK; ++sblk) {
		uint64_t sbpos = hints[sblk];
		if (sbpos - (sblk + 1) * SUBB_SIZE + 1 >= off) break;
	}
	unsigned int res = off, sbpos = 0;
	if (sblk > 0) {
		sbpos = hints[sblk-1] + 1;
		res -= sbpos - sblk * SUBB_SIZE;
	}
	return sbpos + bits->scan_zeros(start + sbpos, res);
}

}//namespace

