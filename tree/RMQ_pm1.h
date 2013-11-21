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

//depends on bitarray, msb_intr, ceillog2

int word_excess(uint64_t x);
std::pair<int8_t, uint8_t> max_excess_word_slow(uint64_t x, uint8_t st = 0, uint8_t ed = 64);
std::pair<int8_t, uint8_t> min_excess_word_slow(uint64_t x, uint8_t st = 0, uint8_t ed = 64);
std::pair<int8_t, uint8_t> max_excess_word(uint64_t x);
std::pair<int8_t, uint8_t> min_excess_word(uint64_t x);
std::pair<int8_t, uint8_t> min_excess_word(uint64_t x, uint8_t st, uint8_t ed);
std::pair<int8_t, uint8_t> max_excess_word(uint64_t x, uint8_t st, uint8_t ed);
void print_max_excess_8_table();


class RMQ_index_pm1 {
public:
	static void build(BitArray b, unsigned int blksize, bool _min_struct, RMQ_index_pm1* out) {
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
		{//last one, it is not a whole word
			uint64_t w = b.word(nc - 1);
			int mx;
			if (_min_struct) mx = val + min_excess_word(w, 0, b.length() % WORDSIZE).first;
			else mx = val + max_excess_word(w, 0, b.length() % WORDSIZE).first;
			mwex[nc - 1] = mx;
		}
		RMQ_index_blk::build(mwex, blksize, _min_struct, &(out->blks));
	}

	RMQ_index_pm1() {}
	static const unsigned int WORDSIZE = 64;
	size_t m_idx(size_t st, size_t ed) const {
		assert(ed > st);
		ed--;
		unsigned int stb = (unsigned int)st / WORDSIZE;
		uint8_t stp = (uint8_t)(st % WORDSIZE);
		unsigned int edb = (unsigned int)ed / WORDSIZE;
		uint8_t edp = (uint8_t)(ed % WORDSIZE);
		if (stb == edb) {
			return st + getpos(stb, stp, edp + 1);
		} else {
			std::vector<unsigned int> poslst;
			if (stp > 0) {
				poslst.push_back(st + getpos(stb, stp, WORDSIZE));
				stb += 1;
			}
			if (edp == WORDSIZE - 1) edb += 1;
			auto lst = blks.m_idx(stb, edb);
			checkpos(lst, &poslst);
			if (edp != WORDSIZE - 1)
				poslst.push_back(edb * WORDSIZE + getpos(edb, 0, edp + 1));
			return validate(poslst);
		}
	}
	int excess(size_t i) const { return int(2 * bits.rank(i + 1)) - (int)(i + 1); }

	void clear();

	std::string to_str() const;
	void save_aux(OArchive& ar) const;
	void load_aux(IArchive& ar, const BitArray& b);

private:
	inline size_t getpos(unsigned int wpos, uint8_t st, uint8_t ed) const {
		if (_min_struct) {
			return min_excess_word(bits.getBitArray().word(wpos), st, ed).second;
		} else {
			max_excess_word(bits.getBitArray().word(wpos), st, ed).second;
		}
	}

	inline void checkpos(const std::vector<unsigned int>& blkpos, std::vector<unsigned int>* out) const {
		if (_min_struct) {
			for (auto blkp : blkpos) 
				out->push_back(blkp * WORDSIZE + min_excess_word(bits.getBitArray().word(blkp)).second);
		} else {
			for (auto blkp : blkpos)
				out->push_back(blkp * WORDSIZE + max_excess_word(bits.getBitArray().word(blkp)).second);
		}
	}

	inline size_t validate(std::vector<unsigned int>& pos) const {
		if (_min_struct) {
			int mx = std::numeric_limits<int>::max();
			for (auto p : pos) {
				auto e = excess(p);
				if (mx > e) e = mx;
			}
			return mx;
		} else {
			int mx = std::numeric_limits<int>::min();
			for (auto p : pos) {
				auto e = excess(p);
				if (mx < e) e = mx;
			}
			return mx;
		}
	}


	bool _min_struct;
	RMQ_index_blk blks;
	Rank6p bits;
	friend class RMQ_pm1_builder;
};

//----------------------------------------------------------------


}//namespace

