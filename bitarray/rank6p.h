#pragma once

#ifndef __RANK_6P_H_
#define __RANK_6P_H_

#include "bitarray.h"
#include "archive.h"
#include "rankselect.h"
#include <cstdint>



namespace mscds {

class Rank6pBuilder;
class Rank6pHintSel;

class Rank6p : public RankSelect {
private:
	BitArray bits;
	BitArray inv;
	uint64_t onecnt;
public:
	uint64_t rank(uint64_t p) const;
	uint64_t rankzero(uint64_t p) const;
	uint64_t select(uint64_t r) const;
	uint64_t selectzero(uint64_t r) const;
	uint64_t one_count() const { return onecnt; }
	uint64_t length() const { return bits.length(); }
	void clear();
	
	bool bit(uint64_t p) const;

	void load(IArchive& ar, BitArray& b);
	void save(OArchive& ar) const;
	const BitArray& getBitArray() const { return bits; }
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
};

class Rank6pBuilder {
public:
	static void build(const BitArray& b, Rank6p * o);
	static void build(const BitArray& b, OArchive& ar);
private:
	static uint64_t getwordz(const BitArray& v, size_t idx);
};


class Rank6pHintSel {
	Rank6p rankst;
	BitArray hints;
public:
	void init(Rank6p& r);
	void init(BitArray& b);

	uint64_t select(uint64_t r) const;
	void clear();
private:
	void init();
};

}


#endif //__RANK_6P_H_