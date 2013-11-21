#pragma once

#ifndef __RRR_H_
#define __RRR_H_

#include "bitarray.h"
#include "framework/archive.h"
#include "rankselect.h"
#include <cstdint>
#include <string>



namespace mscds {

class RRRBuilder;
class RRRHintSel;

class RRR : public RankSelect {
private:
	//BitArray bits;
	//BitArray inv;
    BitArray E, R, S, sumR, posS;
	uint64_t onecnt, len;
public:
	uint64_t rank(uint64_t p) const;
	uint64_t rankzero(uint64_t p) const;
	uint64_t select(uint64_t r) const;
	uint64_t selectzero(uint64_t r) const;
	uint64_t one_count() const { return onecnt; }
	uint64_t length() const { /*return bits.length();*/ return len; }
	bool access(uint64_t pos) const { return (rank(pos+1) - rank(pos))==1; }
	void clear();
	
	bool bit(uint64_t p) const;
	//std::string to_str() const { return bits.to_str(); }

	void loadp(IArchive& ar, BitArray& b);
	void savep(OArchive& ar) const;

	void load(IArchive& ar);
	void save(OArchive& ar) const;
	//const BitArray& getBitArray() const { return bits; }
	typedef RRRBuilder BuilderTp;
private:
    uint64_t partialsum(uint64_t block) const;
    uint64_t positionS(uint64_t block) const;
//	uint64_t blkrank(size_t blk) const;
/*	uint64_t subblkrank(size_t blk, unsigned int off) const;
	uint64_t blkrank0(size_t blk) const;
	uint64_t subblkrank0(size_t blk, unsigned int off) const;

	uint64_t selectblock(uint64_t blk, uint64_t d) const;
	uint64_t selectblock0(uint64_t blk, uint64_t d) const;

	unsigned int word_rank(size_t idx, unsigned int i) const;*/
	friend class RRRBuilder;
	friend class RRRHintSel;
	friend struct BlockIntIterator;
};

class RRRBuilder {
public:
	static void build(const BitArray& b, RRR * o);
	static void build(const BitArray& b, OArchive& ar);
	typedef RRR QueryTp;
private:
	static uint64_t getwordz(const BitArray& v, size_t idx);
};


class RRRHintSel {
	RRR rankst;
	FixedWArray hints;
public:
	void init(RRR& r);
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


#endif //__RRR_H_
