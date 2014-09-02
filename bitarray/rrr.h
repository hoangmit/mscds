#pragma once

#ifndef __RRR_H_
#define __RRR_H_

/** 
\file
RRR Compressed Rank/Select data structure 

Based on "" paper.

Implemented by Chai.

*/

#include "../framework/archive.h"
#include "bitarray.h"
#include "rankselect.h"

#include <cstdint>
#include <string>

namespace mscds {

class RRRBuilder;
class RRRHintSel;


/// RRR rank select data structure
class RRR : public RankSelect {
private:
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

	void loadp(InpArchive& ar, BitArray& b);
	void savep(OutArchive& ar) const;

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	typedef RRRBuilder BuilderTp;
private:
    uint64_t partialsum(uint64_t block) const;
    uint64_t positionS(uint64_t block) const;

	friend class RRRBuilder;
	friend class RRRHintSel;
	friend struct BlockIntIterator;
};

class RRRBuilder {
public:
	static void build(const BitArray& b, RRR * o);
	static void build(const BitArray& b, OutArchive& ar);
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
