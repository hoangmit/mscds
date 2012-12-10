#pragma once

#include <stdint.h>
#include "bitarray/bitarray.h"
#include "bitarray/rank6p.h"
#include "bitarray/sdarray.h"

namespace mscds {

class BP_aux {
public:
	uint64_t excess(uint64_t i);
	uint64_t find_close(uint64_t);
	uint64_t find_open(uint64_t);
	uint64_t enclose(uint64_t);
private:
	BitArray bp_bits;
	Rank6p bitrank;
	unsigned int rec_lvl;
	static const unsigned int blksize = 128;
};


inline BitArray pioneer_map(const BitArray& bp, size_t blksize);


} //namespace
