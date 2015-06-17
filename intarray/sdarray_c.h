#pragma once

#include "bitarray/rrr3.h"
#include "bitarray/bitarray.h"
#include "sdarray_blk.h"

namespace mscds {

class SDArrayCompressBuilder;

class SDArrayCompress {
public:
	typedef SDArrayCompressBuilder BuilderTp;
private:
	friend class SDArrayCompressBuilder;
	SDArrayBlock blk;
	BitArray header;
	RRR_BitArray bits;
};

class SDArrayCompressBuilder {
public:
	void add(unsigned int v) {

	}

	void add_inc(unsigned int v) {

	}
	void build(SDArrayCompress* out) {
		BitArray ba;

		RRR_BitArrayBuilder::build_array(ba, &out->bits);
	}
private:
	SDArrayBlock blk;
	RRR_WordAccessBuilder bd;
};

}//namespace