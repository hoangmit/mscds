#pragma once

#ifndef __RANK_6P_H_
#define __RANK_6P_H_

/**
\file
Rank/Select auxiliary data structure that uses 6% of the input bit vector as the additional index.

Based on "..." paper.

Implemented by Hoang

*/

#include "bitarray.h"
#include "rankselect.h"
#include "../framework/archive.h"
#include <cstdint>
#include <string>


namespace mscds {

class Rank6pBuilder;
class Rank6pHintSel;

/// Rank Auxiliary data structure that uses additional 6.25% of the the original input
class Rank6p: public RankSelectInterface {
public:
	/** counts the number of 1 in the range from 1 to (p-1) */
	uint64_t rank(uint64_t p) const;
	/** counts the number of 0 in the range from 1 to (p-1) */
	uint64_t rankzero(uint64_t p) const;
	/** the position of the (r+1)-th 1-value (from left to right) */
	uint64_t select(uint64_t r) const;
	/** the position of the (r+1)-th 0-value (from left to right) */
	uint64_t selectzero(uint64_t r) const;
	/** returns the number of 1 in the whole array */
	uint64_t one_count() const { return onecnt; }
	/** returns the length of the array */
	uint64_t length() const { return bits.length(); }
	
	/** returns the value p-th bit in the bit array */
	bool access(uint64_t pos) const { return bits[pos]; }
	bool bit(uint64_t p) const;

	void clear();
	
	std::string to_str() const { return bits.to_str(); }

	void load_aux(InpArchive& ar, BitArray& b);
	void save_aux(OutArchive& ar) const;

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	const BitArray& getBitArray() const { return bits; }
	typedef Rank6pBuilder BuilderTp;
private:
	BitArray bits;
	BitArray inv;
	uint64_t onecnt;
private:
	uint64_t blkrank(size_t blk) const;
	uint64_t subblkrank(size_t blk, unsigned int off) const;
	uint64_t blkrank0(size_t blk) const;
	uint64_t subblkrank0(size_t blk, unsigned int off) const;

	uint64_t selectblock(uint64_t blk, uint64_t d) const;
	uint64_t selectblock0(uint64_t blk, uint64_t d) const;

	unsigned int word_rank(size_t idx, unsigned int i) const;
	friend class Rank6pBuilder;
	friend class Rank6pHintSel;
	friend struct BlockIntIterator;
};


/// Builder class for Rank6p
class Rank6pBuilder {
public:
	static void build(const BitArray& b, Rank6p * o);
	//static void build(const BitArray& b, OutArchive& ar);
	typedef Rank6p QueryTp;
private:
	static uint64_t getwordz(const BitArray& v, size_t idx);
};

/// Rank6p adds select hints
class Rank6pHintSel {
public:
	void init(Rank6p& r);
	void init(BitArray& b);

	uint64_t select(uint64_t r) const;
	uint64_t rank(uint64_t p) const {
		return rankst.rank(p);
	}
	void clear();
private:
	Rank6p rankst;
	FixedWArray hints;
private:
	void init();
};

}//namespace


#endif //__RANK_6P_H_