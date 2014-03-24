#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include "codec/vbyte.h"


#include <vector>

namespace mscds {

struct VByteArray {

	static void append(OBitStream& out, uint64_t val) {
		auto appendsummary = [&out](uint8_t v){ out.puts(v, 8); };
		coder::VByte::encode_f(val, appendsummary);
	}

	static uint64_t extract(IWBitStream& is) {
		auto getter = [&is]()->uint8_t { return is.get(8); };
		return coder::VByte::decode_f(getter);
	}
};

}//namespace