#pragma once

#include "bitarray/rrr3.h"
#include "bitarray/bitarray.h"

namespace mscds {

class SDArrayCompress {
public:
private:
	BitArray lower;
	RRR_BitArray upper;
};

}//namespace