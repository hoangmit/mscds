#include "sdarray_small.h"

#include "bitarray/bitop.h"

#include <cassert>

namespace mscds {

SDArraySmlBuilder::SDArraySmlBuilder() {
	vals.reserve(BLKSIZE);
	cnt = 0;
	p_sum = 0;
}

void SDArraySmlBuilder::add(uint64_t val) {
	cnt++;
	vals.push_back(val);
	if (vals.size() == BLKSIZE)
		build_blk();
}

void SDArraySmlBuilder::build(SDArraySml* out) {
	build_blk();
	bits.close();
	out->len = cnt;
	out->sum = p_sum;
	out->bits = BitArray::create(bits.data(), bits.length());
	out->table = BitArray::create(table.data(), table.size()*64);
	cnt = 0;
	p_sum = 0;
}

const uint64_t SDArraySmlBuilder::BLKSIZE = 512;
const uint16_t SDArraySmlBuilder::SUBB_PER_BLK = 7;
const uint64_t SDArraySml::BLKSIZE = 512;
const uint64_t SDArraySml::SUBB_SIZE = 74;//=(ceil(BLKSIZE/SUBB_PER_BLK))


void SDArraySmlBuilder::build_blk(){
	assert(vals.size() <= BLKSIZE);
	if (vals.size() == 0) return;
	while (vals.size() < BLKSIZE) vals.push_back(0);

	for (size_t p = 1; p < vals.size(); ++p)
		vals[p] += vals[p - 1];

	uint64_t begPos  = bits.length();
	table.push_back(p_sum);
	p_sum += vals.back();

	// blkinfo
	assert(begPos < (1ULL << 50));
	uint64_t blkinfo = (uint64_t)begPos;

	//lower bits
	uint64_t width = ceillog2(1+vals.back() / vals.size());
	assert(width < (1ULL << 7));

	for (size_t p = 0; p < vals.size(); ++p)
		bits.puts(vals[p], width); /* will be masked inside */
	blkinfo |= (width << 57);

	//higher bits' hints
	uint64_t select_hints = 0;
	const unsigned int step = (BLKSIZE + SUBB_PER_BLK - 1) / SUBB_PER_BLK;
	size_t i = step;
	for (size_t p = 0; p < SUBB_PER_BLK - 1; ++p) {
		uint64_t hp = ((vals[i-1] >> width) + i-1);
		assert(ceillog2(hp) <= 10);
		select_hints |= (hp << (p*10));
		assert(p*10 <= 64);
		i += step;
	}

	//higer bits
	size_t j = 0;
	for (size_t p = 0; p < vals.size(); p++) {
		size_t pos = (vals[p] >> width) + p;
		while (j < pos) { bits.put0(); ++j; }
		bits.put1(); ++j;
	}
	table.push_back(blkinfo);
	table.push_back(select_hints);
	vals.clear();
}

//----------------------------------------------------------------------------

struct BlkHintInfo {
	uint64_t hints;
	BlkHintInfo(){}
	BlkHintInfo(uint64_t v):hints(v){}
	uint32_t getHints(uint32_t p) const {
		if (p == 0) return 0;
		return 0;
	}
};

uint64_t SDArraySml::getBits(uint64_t x, uint64_t beg, uint64_t num) {
	return (x >> beg) & ((1ULL << num) - 1);
}

uint64_t SDArraySml::prefixsum(size_t p) const {
	if (p >= len) return this->sum;
	uint64_t bpos = p / BLKSIZE;
	uint32_t off  = p % BLKSIZE;
	uint64_t sum  = table.word(bpos * 3);
	if (off == 0) return sum;
	uint64_t info   = table.word(bpos * 3 + 1);
	uint64_t blkptr = info & 0x01FFFFFFFFFFFFFFull;
	uint32_t width  = info >> 57;
	uint64_t lo = (width > 0) ? bits.bits(blkptr + width * (off - 1), width) : 0;
	uint64_t hi = select_hi(table.word(bpos * 3 + 2), blkptr + width*BLKSIZE, off - 1) + 1 - off;
	return sum + ((hi << width) | lo);
}

uint64_t SDArraySml::select_hi(uint64_t hints, uint64_t start, uint32_t off) const {
	uint64_t subblkpos = off / SUBB_SIZE;
	uint32_t res       = off % SUBB_SIZE;
	if (res == SUBB_SIZE - 1)
		return getBits(hints, subblkpos*10, 10);
	uint64_t gb = subblkpos > 0 ? getBits(hints, (subblkpos-1)*10, 10) + 1 : 0;
	return scan_hi_bits(start + gb, res) + gb;
}

uint64_t SDArraySml::scan_hi_bits(uint64_t start, uint32_t res) const {
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
	} while(true);
}

uint64_t SDArraySml::lookup(const uint64_t p) const {
	uint64_t bpos = p / BLKSIZE;
	uint32_t off  = p % BLKSIZE;
	uint64_t info   = table.word(bpos * 3 + 1);
	uint64_t blkptr = info & 0x01FFFFFFFFFFFFFFull;
	uint32_t width  = info >> 57;
	uint64_t prev = 0;
	int64_t prehi;
	if (off == 0) {
		prehi = 0;
		prev = 0;
	} else {
		uint64_t prelo = bits.bits(blkptr + width * (off - 1), width);
		prehi = select_hi(table.word(bpos * 3 + 2), blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	uint64_t lo = bits.bits(blkptr + width * off, width);
	uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	uint64_t cur = ((hi << width) | lo);
	return cur - prev;
	//return sum + prev;
}

uint64_t SDArraySml::lookup(const uint64_t p, uint64_t& prev_sum) const {
	uint64_t bpos = p / BLKSIZE;
	uint32_t off  = p % BLKSIZE;
	uint64_t info   = table.word(bpos * 3 + 1);
	uint64_t blkptr = info & 0x01FFFFFFFFFFFFFFull;
	uint32_t width  = info >> 57;
	uint64_t prev = 0;
	int64_t prehi;
	if (off == 0) {
		prehi = 0;
		prev = 0;
	} else {
		uint64_t prelo = bits.bits(blkptr + width * (off - 1), width);
		prehi = select_hi(table.word(bpos * 3 + 2), blkptr + width*BLKSIZE, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	uint64_t lo = bits.bits(blkptr + width * off, width);
	uint64_t hi = prehi + scan_hi_bits(blkptr + width*BLKSIZE + prehi + off, 0);
	uint64_t cur = ((hi << width) | lo);
	uint64_t sum  = table.word(bpos * 3);
	prev_sum = sum + prev;
	return cur - prev;
}

//number of 1 that is less than val
uint64_t SDArraySml::rank(uint64_t val) const {
	uint64_t lo = 0;
	uint64_t hi = table.word_count() / 3;
	while (lo < hi) {
		uint64_t mid = lo + (hi - lo) / 2;
		if (table.word(mid*3) < val) lo = mid + 1;
		else hi = mid;
	}
	if (lo == 0) return 0;
	lo--;
	assert(val > table.word(lo*3));
	assert(lo < table.word_count()/3 || val <= table.word((lo+1)*3));
	return lo * BLKSIZE + rankBlk(lo, val - table.word(lo*3));
}

uint64_t SDArraySml::rankBlk(uint64_t blk, uint64_t val) const {
	uint64_t info   = table.word(blk * 3 + 1);
	uint64_t blkptr = info & 0x01FFFFFFFFFFFFFFull;
	uint32_t width  = info >> 57;
	
	uint64_t vlo = val & ((1ull << width) - 1);
	uint64_t vhi = val >> width;
	uint32_t hipos = 0, rank = 0;
	if (vhi > 0) {
		hipos = select_zerohi(table.word(blk * 3 + 2), blkptr + width*BLKSIZE, vhi-1)+1;
		//uint32_t ck = scan_zerohi_bitslow(blkptr + width*BLKSIZE, vhi-1) + 1;
		//assert(ck == hipos);
		rank = hipos - vhi;
	}
	uint64_t curlo = 0;
	while (rank < BLKSIZE && bits.bit(blkptr + width*BLKSIZE + hipos)) {
		curlo =  bits.bits(blkptr + width * rank, width);
		if (curlo >= vlo) break;
		++rank;
		++hipos;
	} 
	return rank+1;
}

uint64_t SDArraySml::select_zerohi(uint64_t hints, uint64_t start, uint32_t off) const {
	uint64_t sblk = 0;
	for (; sblk < 6; ++sblk) {
		uint64_t sbpos = getBits(hints, sblk*10, 10);
		if (sbpos - (sblk+1) * SUBB_SIZE + 1 >= off) break;
	}
	uint64_t res = off, sbpos = 0;
	if (sblk > 0) {
		sbpos = getBits(hints, (sblk-1)*10, 10) + 1;
		res -= sbpos - sblk * SUBB_SIZE; 
	}
	return sbpos + scan_hi_zeros(start + sbpos, res);
}

uint64_t SDArraySml::scan_hi_zeros(uint64_t start, uint32_t res) const {
	uint64_t wpos = start >> 6;
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
	} while(true);
}


uint64_t SDArraySml::scan_zerohi_bitslow(uint64_t start, uint32_t res) const {
	for(size_t i = start; i < start + BLKSIZE*3; i++) {
		if (!bits.bit(i)) {
			if (res == 0) return i - start;
			else res--;
		}
	}
	return ~0ull;
}


/*
uint64_t SDArraySml::scan_hi_bitslow(uint64_t start, uint32_t res) const {
	for(size_t i = start; i < start + BLKSIZE*3; i++) {
		if (bits.bit(i)) {
			if (res == 0) return i - start;
			else res--;
		}
	}
	return ~0ull;
}
*/

}//namespace