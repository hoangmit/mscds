#pragma once

#ifndef __RRR2_H_
#define __RRR2_H_

/**
\file
RRR Compressed Rank/Select data structure. (This data structures does not use pre-computed look up table.)

Based on "" paper.

Implemented by Chai.

*/

#include "../framework/archive.h"
#include "bitarray.h"
#include "rankselect.h"
#include <cstdint>
#include <string>

namespace mscds {

class RRR2Builder;
class RRR2HintSel;

/// RRR2 data structure
class RRR2 : public RankSelect {
public:
	RRR2();
	uint64_t rank(uint64_t p) const;
	uint64_t rankzero(uint64_t p) const;
	uint64_t select(uint64_t r) const;
	uint64_t selectzero(uint64_t r) const;
	uint64_t one_count() const { return onecnt; }
	uint64_t length() const { return len; }
	bool access(uint64_t pos) const { return (rank(pos+1) - rank(pos))==1; }
	void clear();
	
	bool bit(uint64_t p) const;
	//std::string to_str() const { return bits.to_str(); }

	void loadp(InpArchive& ar, BitArray& b);
	void savep(OutArchive& ar) const;

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	//const BitArray& getBitArray() const { return bits; }
	typedef RRR2Builder BuilderTp;
private:
	BitArray R, S, sumR, posS;
	uint64_t onecnt, len;
	unsigned bits_per_sumR, bits_per_posS;

	static uint64_t nCk[2048];
	static uint8_t code_len[64];
	static void _init_nCk();
	void _init_variables();
private:
    uint64_t partialsum(uint64_t block) const;
    uint64_t positionS(uint64_t block) const;
    uint64_t decode(uint64_t offset, unsigned int k) const;

	friend class RRR2Builder;
	friend class RRR2HintSel;
	friend struct BlockIntIterator;
};

class RRR2Builder {
public:
	static void init_table();
    static uint64_t encode(uint64_t w, unsigned int k);
	static void build(const BitArray& b, RRR2 * o);
	static void build(const BitArray& b, OutArchive& ar);
	typedef RRR2 QueryTp;
private:
	static uint64_t getwordz(const BitArray& v, size_t idx);
};


class RRR2HintSel {
	RRR2 rankst;
	FixedWArray hints;
public:
	void init(RRR2& r);
	void init(BitArray& b);

	uint64_t select(uint64_t r) const;
	uint64_t rank(uint64_t p) const {
		return rankst.rank(p);
	}
	void clear();
private:
	void init();
};

}


#endif //__RRR2_H_
