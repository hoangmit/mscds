#pragma once

#ifndef __RANK_3P_H_
#define __RANK_3P_H_

/**
\file
Rank/Select auxiliary data structure that uses 3% of the input bit vector as the additional index.

Based on "..." paper.

Implemented by Chai
*/

#include "bitarray.h"
#include "rankselect.h"
#include "../framework/archive.h"
#include <cstdint>
#include <string>

namespace mscds {

class Rank3pBuilder;
class Rank3pHintSel;

/// Rank Auxiliary data structure that uses additional 3% of the the original input
class Rank3pAux: public RankSelectInterface {
public:
	uint64_t rank(uint64_t p) const;
	uint64_t rankzero(uint64_t p) const;
	uint64_t select(uint64_t r) const;
	uint64_t selectzero(uint64_t r) const;
	uint64_t one_count() const { return onecnt; }
	uint64_t length() const { return bits->length(); }
	bool access(uint64_t pos) const { return bits->bit(pos); }
	void clear();
	
	bool bit(uint64_t p) const;
	std::string to_str() const { return bits->to_str(); }

	void load_aux(InpArchive& ar, const BitArrayInterface* b);
	void save_aux(OutArchive& ar) const;

	const BitArrayInterface* getBitArray() const { return bits; }
	typedef Rank3pBuilder BuilderTp;
protected:
	const BitArrayInterface* bits;
	BitArray l0, l1_l2, sampling;
	uint64_t onecnt;
private:
	uint64_t l0rank(uint64_t blk) const;
	uint64_t l1rank(uint64_t blk) const;
	uint64_t basicblkcount(uint64_t blk) const;
	uint64_t countone(uint64_t start, uint64_t end) const;
	uint64_t l1binarysearch(uint64_t j, uint64_t start, uint64_t end) const;
	uint64_t l0binarysearch(uint64_t i) const;

	friend class Rank3pBuilder;
	friend class Rank3pHintSel;
	friend struct BlockIntIterator;
};

class Rank3p: public Rank3pAux {
public:
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
private:
	friend class Rank3pBuilder;
	BitArray _own_bits;
};

/// Builder class for Rank3p
class Rank3pBuilder {
public:
	static void build_aux(const BitArrayInterface* b, Rank3pAux * o);
	static void build(const BitArray& b, Rank3p * o);
	typedef Rank3pAux QueryTp;
private:
	static uint64_t getwordz(const BitArrayInterface* v, size_t idx);
};


/// Rank3p with select hints
class Rank3pHintSel {
	Rank3p rankst;
	FixedWArray hints;
public:
	void init(Rank3p& r);
	void init(BitArray& b);

	uint64_t select(uint64_t r) const;
	uint64_t rank(uint64_t p) const { return rankst.rank(p); }
	void clear();
private:
	void init();
};

}


#endif //__RANK_3P_H_
