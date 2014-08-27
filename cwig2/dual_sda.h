#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include <stdint.h>
#include <vector>


namespace mscds {

class SDABGetterInterface {
public:
	virtual void loadBlk(unsigned int i) const = 0;
	virtual uint64_t sum(unsigned int b) const = 0;
	virtual uint8_t width() const = 0;
	virtual uint64_t hints() const = 0;
	virtual size_t upper_start() const = 0;
	virtual uint64_t lower(unsigned int i) const = 0;
};

class SDABSetterInterface {
public:
	virtual void newblock() = 0;
	virtual void sum(uint64_t) = 0;
	virtual void width(uint8_t) = 0;
	virtual void hints(uint64_t) = 0;

	virtual void upper_len(unsigned int len) = 0;
	virtual void set_upper_pos(unsigned int p) = 0;

	virtual void lower(unsigned int i, uint64_t value) = 0;
	virtual void finishblock() = 0;
};

template<typename S>
class SDArrayBlockGBuilder {
public:
	SDArrayBlockGBuilder(S& setter_): setter(setter_) {}
	void add(unsigned int val) { vals.push_back(val); }
	void buildBlk();
	static const unsigned int BLKSIZE = 512;
private:
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;
	S& setter;
	std::vector<uint64_t> vals;
};

template<typename G>
class SDArrayBlockG {
public:
	typedef uint64_t ValueType;
	SDArrayBlockG(const G& getter_): getter(getter_), bits(NULL), lastblk(~0ull) {}
	void bind(BitArray* bits_) { bits = bits_; }
	void loadBlock(unsigned int id);
	ValueType prefixsum(unsigned int  p) const;
	ValueType lookup(unsigned int p) const;
	ValueType lookup(unsigned int p, ValueType& prev_sum) const;
	ValueType blk_sum(unsigned int b) const;
	unsigned int rank(ValueType val) const;
	static const unsigned int BLKSIZE = 512;
private:
	unsigned int select_hi(uint64_t hints, uint32_t off) const;
	static uint64_t getBits(uint64_t x, uint64_t beg, uint64_t num);
	unsigned int select_zerohi(uint64_t hints, uint32_t off) const;
	uint64_t lower(size_t off) const;
private:
	static const unsigned int SUBB_PER_BLK = 7;
	static const unsigned int SUBB_SIZE = 74;
private:
	const G& getter;
	uint16_t width;
	uint64_t select_hints;
	mscds::BitArray* bits;
	size_t upper_pos;
	size_t lastblk;
};

template<typename B>
class SDArrayInterBlkG {
public:
	typedef uint64_t ValueType;
	mutable B& blk;
	SDArrayInterBlkG(B& b): blk(b) {}
	static const unsigned int BLKSIZE = 512;

	void loadBlk(unsigned int b) const {
		blk.loadBlock(b);
	}

	size_t getBlkSum(unsigned int b) const {
		return blk.blk_sum(b);
	}

	ValueType prefixsum(unsigned int p) const {
		//if (p == this->len) return this->sum;
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		auto sum = getBlkSum(bpos);
		if (off == 0) return sum;
		else {
			loadBlk(bpos);
			return sum + blk.prefixsum(off);
		}
	}

	ValueType lookup(unsigned int p) const {
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		loadBlk(bpos);
		return blk.lookup(off);
	}

	ValueType lookup(unsigned int p, ValueType &prev_sum) const {
		uint64_t bpos = p / BLKSIZE;
		uint32_t off = p % BLKSIZE;
		auto sum = getBlkSum(bpos);
		loadBlk(bpos);
		auto v = blk.lookup(p, prev_sum);
		prev_sum += sum;
		return v;
	}

	unsigned int rank(ValueType val) const {
		if (val > total_sum()) return length();
		uint64_t lo = 0;
		uint64_t hi = mng->blkCount();
		while (lo < hi) {
			uint64_t mid = lo + (hi - lo) / 2;
			if (getBlkSum(mid) < val) lo = mid + 1;
			else hi = mid;
		}
		if (lo == 0) return 0;
		lo--;
		assert(val > getBlkSum(lo));
		loadBlk(lo);
		ValueType ret = lo * BLKSIZE + blk.rank(val - getBlkSum(lo));
		return ret;
	}

	unsigned int _rank(ValueType val, unsigned int begin, unsigned int end) const {
		if (val > total_sum()) return length();
		uint64_t lo = begin;
		uint64_t hi = end;
		assert(lo <= hi && hi <= mng->blkCount());
		while (lo < hi) {
			uint64_t mid = lo + (hi - lo) / 2;
			if (getBlkSum(mid) < val) lo = mid + 1;
			else hi = mid;
		}
		if (lo == 0) return 0;
		lo--;
		assert(val > getBlkSum(lo));
		loadBlk(lo);
		ValueType ret = lo * BLKSIZE + blk.rank(val - getBlkSum(lo));
		return ret;
	}
};

}//namespace

//-------------------------------------------------------------

namespace mscds {



template<typename S>
void SDArrayBlockGBuilder<S>::buildBlk() {
	assert(vals.size() <= BLKSIZE);
	if (vals.size() == 0) return;
	setter.newblock();
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
	setter.width(width);
	setter.hints(select_hints);

	//lower bits
	for (size_t p = 0; p < vals.size(); ++p)
		setter.lower(p, vals[p] & ((1ull << width) - 1));

	//higer bits
	setter.upper_len((vals[vals.size() - 1] >> width) + vals.size() - 1);
	size_t j = 0;
	for (size_t p = 0; p < vals.size(); p++) {
		size_t pos = (vals[p] >> width) + p;
		setter.set_upper_pos(pos);
	}
	vals.clear();
	setter.finishblock();
}

template<typename G>
void SDArrayBlockG<G>::loadBlock(unsigned int id) {
	if (id != lastblk) {
		getter.loadBlk(id);
		width = getter.width();
		select_hints = getter.hints();
		upper_pos = getter.upper_start();
		lastblk = id;
	}
}

template<typename G>
uint64_t SDArrayBlockG<G>::lower(size_t off) const {
	return getter.lower(off);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::blk_sum(unsigned int b) const {
	return getter.sum(b);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::prefixsum(unsigned int p) const {
	if (p == 0) return 0;
	ValueType lo = (width > 0) ? lower(p-1) : 0;
	ValueType hi = select_hi(select_hints, p - 1) + 1 - p;
	return ((hi << width) | lo);
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::lookup(unsigned int off) const {
	uint64_t prev = 0;
	int64_t prehi = 0;
	if (off > 0) {
		ValueType prelo = lower(off - 1);
		prehi = select_hi(select_hints, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = lower(off);
	ValueType hi = prehi + bits->scan_next(upper_pos + prehi + off);
	ValueType cur = ((hi << width) | lo);
	return cur - prev;
}

template<typename G>
typename SDArrayBlockG<G>::ValueType SDArrayBlockG<G>::lookup(unsigned int off, ValueType &prev_sum) const {
	ValueType prev = 0;
	ValueType prehi = 0;
	if (off > 0) {
		ValueType prelo = lower(off - 1);
		prehi = select_hi(select_hints, off - 1) + 1 - off;
		prev = ((prehi << width) | prelo);
	}
	ValueType lo = lower(width);
	ValueType hi = prehi + bits->scan_next(upper_pos + prehi + off);
	ValueType cur = ((hi << width) | lo);
	prev_sum = prev;
	return cur - prev;
}

template<typename G>
unsigned int SDArrayBlockG<G>::rank(ValueType val) const {
	ValueType vlo = val & ((1ull << width) - 1);
	ValueType vhi = val >> width;
	uint32_t hipos = 0, rank = 0;
	if (vhi > 0) {
		hipos = select_zerohi(select_hints, vhi - 1) + 1;
		rank = hipos - vhi;
	}
	ValueType curlo = 0;
	while (rank < BLKSIZE && bits->bit(upper_pos + hipos)) {
		curlo = lower(rank);
		if (curlo >= vlo)
			return rank + 1;
		++rank;
		++hipos;
	}
	return rank + 1;
}

template<typename G>
unsigned int SDArrayBlockG<G>::select_hi(uint64_t hints, uint32_t off) const {
	unsigned int subblkpos = off / SUBB_SIZE;
	uint32_t res = off % SUBB_SIZE;
	if (res == SUBB_SIZE - 1)
		return getBits(hints, subblkpos * 10, 10);
	unsigned int gb = subblkpos > 0 ? getBits(hints, (subblkpos - 1) * 10, 10) + 1 : 0;
	return bits->scan_bits(upper_pos + gb, res) + gb;
}

template<typename G>
uint64_t SDArrayBlockG<G>::getBits(uint64_t x, uint64_t beg, uint64_t num) {
	return (x >> beg) & ((1ULL << num) - 1);
}

template<typename G>
unsigned int SDArrayBlockG<G>::select_zerohi(uint64_t hints, uint32_t off) const {
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
	return sbpos + bits->scan_zeros(upper_pos + sbpos, res);
}

}//namespace


void test_dsdd(const std::vector<unsigned int>& lst);
void test_dfsd(const std::vector<unsigned int>& lst);


