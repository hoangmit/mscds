#pragma once

#include "bitarray/bitop.h"
#include "bitarray/bitarray.h"
#include "framework/archive.h"

#include "bitarray/rank6p.h"
#include "RMQ_index_table.h"

#include <stdint.h>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <utility>
#include <sstream>


namespace mscds {

int word_excess(uint64_t x);
std::pair<int8_t, uint8_t> max_excess_word_slow(uint64_t x, uint8_t st = 0, uint8_t ed = 64);
std::pair<int8_t, uint8_t> min_excess_word_slow(uint64_t x, uint8_t st = 0, uint8_t ed = 64);
std::pair<int8_t, uint8_t> max_excess_word(uint64_t x);
std::pair<int8_t, uint8_t> min_excess_word(uint64_t x);
std::pair<int8_t, uint8_t> min_excess_word(uint64_t x, uint8_t st, uint8_t ed);
std::pair<int8_t, uint8_t> max_excess_word(uint64_t x, uint8_t st, uint8_t ed);
void print_max_excess_8_table();


class RMQ_pm1 {
public:
	static void build(BitArray b, unsigned int blksize, bool _min_struct, RMQ_pm1* out);
	static const unsigned int WORDSIZE = 64;

	size_t m_idx(size_t st, size_t ed) const;
	int excess(size_t i) const;

	//std::string to_str() const;

	void save_aux(OArchive& ar) const;
	void load_aux(IArchive& ar, Rank6p& rs);
	void clear() { bits.clear(); blks.clear(); }

private:
	size_t getpos(unsigned int wpos, uint8_t st, uint8_t ed) const;
	void checkpos(const std::vector<unsigned int>& blkpos, std::vector<unsigned int>* out) const;
	size_t validate(std::vector<unsigned int>& pos) const;

private:
	bool _min_struct;
	RMQ_index_blk blks;
	Rank6p bits;
	friend class RMQ_pm1_builder;
};

class RMQ_pm1_minmax {
public:
	static void build(BitArray b, unsigned int blksize, RMQ_pm1_minmax* out);
	std::pair<int, size_t> find_max(size_t st, size_t ed) const;
	std::pair<int, size_t> find_min(size_t st, size_t ed) const;

	void save_aux(OArchive& ar) const;
	void load_aux(IArchive& ar, Rank6p& rs);

private:
	RMQ_pm1 minidx, maxidx;
};

//---------------------------------------------------------------------------------

inline size_t RMQ_pm1::m_idx(size_t st, size_t ed) const {
	if (st >= ed) return st;
	assert(ed > st);
	ed--;
	unsigned int stb = (unsigned int)st / WORDSIZE;
	uint8_t stp = (uint8_t)(st % WORDSIZE);
	unsigned int edb = (unsigned int)ed / WORDSIZE;
	uint8_t edp = (uint8_t)(ed % WORDSIZE);
	if (stb == edb) {
		return stb * WORDSIZE + getpos(stb, stp, edp + 1);
	} else {
		std::vector<unsigned int> poslst;
		poslst.reserve(8);
		if (stp > 0) {
			poslst.push_back((unsigned int)(stb*WORDSIZE + getpos(stb, stp, WORDSIZE)));
			stb += 1;
		}
		if (edp == WORDSIZE - 1)
			edb += 1;
		auto lst = blks.m_idx(stb, edb);
		checkpos(lst, &poslst);
		if (edp != WORDSIZE - 1)
			poslst.push_back((unsigned int)(edb * WORDSIZE + getpos(edb, 0, edp + 1)));
		return validate(poslst);
	}
}

inline int RMQ_pm1::excess(size_t i) const { return int(2 * bits.rank(i + 1)) - (int)(i + 1); }

inline size_t RMQ_pm1::getpos(unsigned int wpos, uint8_t st, uint8_t ed) const {
	uint64_t w = bits.getBitArray().word(wpos);
	if (_min_struct)
		return min_excess_word(w, st, ed).second;
	else
		return max_excess_word(w, st, ed).second;
}

inline void RMQ_pm1::checkpos(const std::vector<unsigned int> &blkpos, std::vector<unsigned int> *out) const {
	if (_min_struct) {
		for (auto blkp : blkpos)
			out->push_back(blkp * WORDSIZE + min_excess_word(bits.getBitArray().word(blkp)).second);
	} else {
		for (auto blkp : blkpos)
			out->push_back(blkp * WORDSIZE + max_excess_word(bits.getBitArray().word(blkp)).second);
	}
}

inline size_t RMQ_pm1::validate(std::vector<unsigned int> &pos) const {
	size_t px = -1;
	assert(pos.size() > 0);
	if (pos.size() == 1) return pos[0];
	if (_min_struct) {
		int mx = std::numeric_limits<int>::max();
		for (auto p : pos) {
			auto e = excess(p);
			if (mx > e) { mx = e; px = p; }
		}
	} else {
		int mx = std::numeric_limits<int>::min();
		for (auto p : pos) {
			auto e = excess(p);
			if (mx < e) { mx = e; px = p; }
		}
	}
	return px;
}


}//namespace

