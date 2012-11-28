#pragma once

#include "bitarray.h"
#include "archive.h"
#include "rankselect.h"
#include <cstdint>

namespace mscds {

class Rank6pBuilder;

class Rank6p : public RankSelect {
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
	
	bool bit(uint64_t p) const;

	void load(IArchive& ar, BitArray& b);
	void save(OArchive& ar) const;
	const BitArray& getBitArray() const { return bits; }
private:
	uint64_t blkrank(size_t blk) const;
	uint64_t subblkrank(size_t blk, unsigned int off) const;
	uint64_t blkrank0(size_t blk) const;
	uint64_t subblkrank0(size_t blk, unsigned int off) const;

	unsigned int word_rank(size_t idx, unsigned int i) const;
	unsigned int word_rank(size_t idx) const;
	unsigned int word_rank0(size_t idx) const;
	friend class Rank6pBuilder;
};

class Rank6pBuilder {
public:
	static void build(const BitArray& b, Rank6p * o);
	static void build(const BitArray& b, OArchive& ar);
private:
	static uint64_t Rank6pBuilder::getwordz(const BitArray& v, size_t idx);
};

}
