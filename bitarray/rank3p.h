#pragma once

#ifndef __RANK_3P_H_
#define __RANK_3P_H_

#include "bitarray.h"
#include "framework/archive.h"
#include "rankselect.h"
#include <cstdint>
#include <string>



namespace mscds {

class Rank3pBuilder;
class Rank3pHintSel;

class Rank3p : public RankSelect {
private:
	BitArray bits;
    BitArray l0, l1_l2, sampling;
	uint64_t onecnt;
public:
	uint64_t rank(uint64_t p) const;
	uint64_t rankzero(uint64_t p) const;
	uint64_t select(uint64_t r) const;
	uint64_t selectzero(uint64_t r) const;
	uint64_t one_count() const { return onecnt; }
	uint64_t length() const { return bits.length(); }
	bool access(uint64_t pos) const { return bits[pos]; }
	void clear();
	
	bool bit(uint64_t p) const;
	std::string to_str() const { return bits.to_str(); }

	void loadp(IArchive& ar, BitArray& b);
	void savep(OArchive& ar) const;

	void load(IArchive& ar);
	void save(OArchive& ar) const;
	const BitArray& getBitArray() const { return bits; }
	typedef Rank3pBuilder BuilderTp;
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

class Rank3pBuilder {
public:
	static void build(const BitArray& b, Rank3p * o);
	static void build(const BitArray& b, OArchive& ar);
	typedef Rank3p QueryTp;
private:
	static uint64_t getwordz(const BitArray& v, size_t idx);
};


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
