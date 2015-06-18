#pragma once

#include "bitarray/rrr3.h"
#include "bitarray/bitarray.h"
#include "intarray/_experiment/sdarray_blk2.h"

namespace mscds {

class SDArrayCompressBuilder;

class SDArrayCompress {
public:
	typedef uint64_t ValueTp;
	typedef SDArrayCompressBuilder BuilderTp;
	ValueTp prefixsum(unsigned int  p) const;
	ValueTp lookup(unsigned int p) const;
	ValueTp lookup(unsigned int p, ValueTp& prev_sum) const;
	unsigned int rank(ValueTp val) const;
	uint64_t length() const { return len; }

	void clear();

	uint64_t getBlkSum(size_t blk) const;

	uint64_t total_sum() const { return sum; }

	static const unsigned BLKSIZE = 512;
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
