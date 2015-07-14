#pragma once

/** 
\file
Compressed SDArray (using RRR bitvector)
*/

#include "sdarray_interface.h"
#include "bitarray/rrr3.h"
#include "bitarray/bitarray.h"
#include "intarray/_experiment/sdarray_blk2.h"

namespace mscds {

class SDArrayCompressBuilder;

/// Compressed SDArray
class SDArrayCompress: public SDArrayInterface {
public:
	typedef uint64_t ValueTp;
	typedef SDArrayCompressBuilder BuilderTp;
	ValueTp prefixsum(ValueTp p) const;
	ValueTp lookup(ValueTp p) const;
	ValueTp lookup(ValueTp p, ValueTp& prev_sum) const;
	ValueTp rank(ValueTp val) const;
	ValueTp rank2(ValueTp p, ValueTp& select) const;
	uint64_t length() const { return len; }

	void clear();

	uint64_t total() const { return sum; }
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	static const unsigned BLKSIZE = 1024;
private:
	ValueTp _getBlkSum(unsigned blk) const;
	ValueTp _getBlkStartPos(unsigned blk) const;
	void _loadBlk(unsigned blk) const;
private:
	friend class SDArrayCompressBuilder;
	mutable SDArrayBlock2 blk;
	unsigned w1, w2;
	BitArray header;
	RRR_BitArray bits;
	size_t sum, len;
};

class SDArrayCompressBuilder {
public:
	SDArrayCompressBuilder();
	void init();

	void add(unsigned int v);

	void add_inc(unsigned int s);
	void build(SDArrayCompress* out);
private:
	void _build_blk();
	void _finalize();
private:
	std::deque<uint64_t> csum, blkpos;
	static const unsigned BLKSIZE = SDArrayCompress::BLKSIZE;
	unsigned i;
	unsigned int w1, w2;

	size_t sum, last;
	SDArrayBlock2 blk;
	OBitStream obits, header;
};

}//namespace
