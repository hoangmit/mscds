#include "RMQ_pm1.h"

#include <map>
#include <vector>
#include <algorithm>

namespace mscds {
using namespace std;


//---------------------------------------------------------------------------------------------

uint8_t max_excess_tbl[256] =
	{ 56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 76, 92, 76, 92, 92, 108,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 69, 85, 85, 101, 69, 85, 85, 101, 85, 101, 101, 117,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 76, 92, 76, 92, 92, 108,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 78, 94, 78, 94, 94, 110,
	56, 72, 78, 94, 78, 94, 94, 110, 78, 94, 94, 110, 94, 110, 110, 126,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 76, 92, 76, 92, 92, 108,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 69, 85, 85, 101, 69, 85, 85, 101, 85, 101, 101, 117,
	56, 72, 65, 81, 56, 72, 74, 90, 56, 72, 65, 81, 67, 83, 83, 99,
	56, 72, 65, 81, 71, 87, 87, 103, 71, 87, 87, 103, 87, 103, 103, 119,
	56, 72, 65, 81, 71, 87, 87, 103, 71, 87, 87, 103, 87, 103, 103, 119,
	71, 87, 87, 103, 87, 103, 103, 119, 87, 103, 103, 119, 103, 119, 119, 135 };


std::pair<int8_t, uint8_t> max_excess_8_slow(uint8_t x) {
	std::pair<int8_t, uint8_t> ret(-127, 0), cur(0, 0);
	for (cur.second = 0; cur.second < 8; ++(cur.second)) {
		if (x & 1) cur.first += 1;
		else cur.first -= 1;
		x >>= 1;
		if (ret.first < cur.first) ret = cur;
	}
	return ret;
}

void print_max_excess_8_table() {
	for (unsigned int i = 0; i < 256; ++i) {
		auto ret = max_excess_8_slow((uint8_t)i);
		assert(ret.second < 8);
		uint8_t encode = ((ret.first + 8) << 3) | (ret.second);
		if (i % 16 == 0 && i > 0) cout << endl;
		cout << (unsigned int)encode << ',';
	}
	cout << endl;
}

int word_excess(uint64_t x) {
	return (int)(2 * popcnt(x)) - 64;
}

std::pair<int8_t, uint8_t> max_excess_8_lookup(uint8_t v) {
	uint8_t encode = max_excess_tbl[v];
	return std::pair<int8_t, uint8_t>(((int8_t)(encode >> 3)) - 8, encode & 7);
}

std::pair<int8_t, uint8_t> min_excess_8_lookup(uint8_t v) {
	uint8_t encode = max_excess_tbl[uint8_t(~v)];
	return std::pair<int8_t, uint8_t>(8 - ((int8_t)(encode >> 3)), encode & 7);
}

std::pair<int8_t, uint8_t> min_excess_word_slow(uint64_t x, uint8_t st, uint8_t ed) {
	x >>= st;
	std::pair<int8_t, uint8_t> ret(127, 0), cur(0, 0);
	for (cur.second = st; cur.second < ed; ++(cur.second)) {
		if (x & 1) cur.first += 1;
		else cur.first -= 1;
		x >>= 1;
		if (ret.first > cur.first) ret = cur;
	}
	return ret;
}

std::pair<int8_t, uint8_t> max_excess_word_slow(uint64_t x, uint8_t st, uint8_t ed) {
	x >>= st;
	std::pair<int8_t, uint8_t> ret(-127, 0), cur(0, 0);
	for (cur.second = st; cur.second < ed; ++(cur.second)) {
		if (x & 1) cur.first += 1;
		else cur.first -= 1;
		x >>= 1;
		if (ret.first < cur.first) ret = cur;
	}
	return ret;
}

std::pair<int8_t, uint8_t> max_excess_word(uint64_t x) {
	register uint64_t byte_excess = x - ((x & 0xAAAAAAAAAAAAAAAAull) >> 1);
	byte_excess = (byte_excess & 0x3333333333333333ull) + ((byte_excess >> 2) & 0x3333333333333333ull);
	byte_excess = (byte_excess + (byte_excess >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	byte_excess *= 0x0101010101010101ull;
	byte_excess <<= 1;
	byte_excess = (((byte_excess | 0x8080808080808080ull) - 0x4038302820181008ull) ^ 0x8080808080808080ull);
	std::pair<int8_t, uint8_t> ret(-127, 0), cur(0, 0);
	int8_t last = 0;
	for (unsigned int i = 0; i < 8; ++i) {
		auto p = max_excess_8_lookup((uint8_t)(x & 0xFF));
		x >>= 8;
		cur.first = last + p.first;
		cur.second = (i << 3) + p.second;
		if (ret.first < cur.first) ret = cur;
		last = (int8_t)(byte_excess & 0xFFu);
		byte_excess >>= 8;
	}
	return ret;
}

std::pair<int8_t, uint8_t> min_excess_word(uint64_t x) {
	register uint64_t byte_excess = x - ((x & 0xAAAAAAAAAAAAAAAAull) >> 1);
	byte_excess = (byte_excess & 0x3333333333333333ull) + ((byte_excess >> 2) & 0x3333333333333333ull);
	byte_excess = (byte_excess + (byte_excess >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	byte_excess *= 0x0101010101010101ull;
	byte_excess <<= 1;
	byte_excess = (((byte_excess | 0x8080808080808080ull) - 0x4038302820181008ull) ^ 0x8080808080808080ull);
	std::pair<int8_t, uint8_t> ret(127, 0), cur(0, 0);
	int8_t last = 0;
	for (unsigned int i = 0; i < 8; ++i) {
		auto p = min_excess_8_lookup((uint8_t)(x & 0xFF));
		x >>= 8;
		cur.first = last + p.first;
		cur.second = (i << 3) + p.second;
		if (ret > cur) ret = cur;
		last = (int8_t)(byte_excess & 0xFFu);
		byte_excess >>= 8;
	}
	return ret;
}

std::pair<int8_t, uint8_t> min_excess_word(uint64_t x, uint8_t st, uint8_t ed) {
	x >>= st;
	ed -= st;
	if (ed < 64) x |= ~((1ull << ed) - 1);
	auto ret = min_excess_word(x);
	ret.second += st;
	return ret;
}

std::pair<int8_t, uint8_t> max_excess_word(uint64_t x, uint8_t st, uint8_t ed) {
	x >>= st;
	ed -= st;
	if (ed < 64) x &= ((1ull << ed) - 1);
	auto ret = max_excess_word(x);
	ret.second += st;
	return ret;
}

void RMQ_pm1::build(BitArray b, unsigned int blksize, bool _min_struct, RMQ_pm1 *out) {
	Rank6pBuilder::build(b, &(out->bits));
	out->_min_struct = _min_struct;
	auto nc = b.word_count();
	std::vector<int> mwex(nc);
	int val = 0;
	for (size_t i = 0; i < nc - 1; ++i) {
		uint64_t w = b.word(i);
		int mx;
		if (_min_struct) mx = val + min_excess_word(w).first;
		else mx = val + max_excess_word(w).first;
		mwex[i] = mx;
		val += word_excess(w);
	}
	{//last one, may not be a whole word
		uint64_t w = b.word(nc - 1);
		unsigned int lastlen = b.length() % WORDSIZE;
		if (lastlen == 0) lastlen = WORDSIZE;
		int mx;
		if (_min_struct) mx = val + min_excess_word(w, 0, lastlen).first;
		else mx = val + max_excess_word(w, 0, lastlen).first;
		mwex[nc - 1] = mx;
	}
	RMQ_index_blk::build(mwex, blksize, _min_struct, &(out->blks));
}

void RMQ_pm1::save_aux(OArchive &ar) const {
	ar.startclass("RMQ_pm1_auxiliary");
	uint32_t v = _min_struct ? 1 : 0;
	ar.var("is_min_structure").save(v);
	blks.save(ar.var("word_rmq"));
	//bits.save_aux(ar.var("rank_aux"));
	ar.endclass();
}

void RMQ_pm1::load_aux(IArchive &ar, Rank6p& rs) {
	ar.loadclass("RMQ_pm1_auxiliary");
	uint32_t v = 0;
	ar.var("is_min_structure").load(v);
	_min_struct = v != 0 ? true : false;
	blks.load(ar.var("word_rmq"));
	bits = rs;
	//bits.load_aux(ar.var("rank_aux"), b);
	ar.endclass();
}

void RMQ_pm1_minmax::build(BitArray b, unsigned int blksize, RMQ_pm1_minmax *out) {
	RMQ_pm1::build(b, blksize, true, &(out->minidx));
	RMQ_pm1::build(b, blksize, true, &(out->maxidx));
}

void RMQ_pm1_minmax::save_aux(OArchive &ar) const {
	ar.startclass("minmax");
	minidx.save_aux(ar.var("minidx"));
	maxidx.save_aux(ar.var("maxidx"));
	ar.endclass();
}

void RMQ_pm1_minmax::load_aux(IArchive &ar, Rank6p& rs) {
	ar.loadclass("minmax");
	minidx.load_aux(ar.var("minidx"), rs);
	maxidx.load_aux(ar.var("maxidx"), rs);
	ar.endclass();
}


//----------------------------------------------------------------------------------------------




}
