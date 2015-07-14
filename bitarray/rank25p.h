#pragma once

/** 
\file
Rank/Select auxiliary data structure that uses 25% of the input bit vector as the additional index.

Based on "..." paper.

Implemented by Hoang.

*/

#include "bitarray.h"
#include "bitop.h"
#include "rankselect.h"

namespace mscds {

class Rank25pBuilder;

/// Rank Auxiliary data structure that uses additional 25% of the the original input
class Rank25pAux: public RankSelectInterface {
public:
	uint64_t rank(uint64_t p) const;
	uint64_t select(uint64_t p) const;
	uint64_t one_count() const { return onecnt; }
	bool access(uint64_t pos) const { return bits->bit(pos); }
	uint64_t length() const { return bits->length(); }
	uint64_t selectzero(uint64_t r) const;
	typedef Rank25pBuilder BuilderTp;
	void clear() { onecnt = 0; inv.clear(); bits = nullptr; }

	void load_aux(InpArchive& ar, const BitArrayInterface * bits);
	void save_aux(OutArchive& ar) const;
private:
	unsigned int word_rank(size_t idx, unsigned int i) const;
	friend class Rank25pBuilder;
protected:
	BitArray inv;
	const BitArrayInterface * bits;
	uint64_t onecnt;
};


/// BitArray and auxiliary data structure
class Rank25p: public Rank25pAux {
public:
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
private:
	BitArray _own_bits;
	friend class Rank25pBuilder;
};

class Rank25pBuilder {
public:
	static void build(const BitArray& b, Rank25p * o);
	static void build_aux(const BitArrayInterface * b, Rank25pAux* o);
	typedef Rank25p QueryTP;
};


}//namespace
