#pragma once

/** 
\file

Coder + stream

*/

#include "bitarray/bitarray.h"
#include "bitarray/bitstream.h"

#include "codec/deltacoder.h"

#include "codec/vbyte.h"


#include <vector>

namespace mscds {

/// VByte Straem
struct VByteStream {

	static void append(OBitStream& out, uint64_t val) {
		auto appendsummary = [&out](uint8_t v){ out.puts(v, 8); };
		coder::VByte::encode_f(val, appendsummary);
	}

	static uint64_t extract(IWBitStream& is) {
		auto getter = [&is]()->uint8_t { return is.get(8); };
		return coder::VByte::decode_f(getter);
	}
	static uint64_t extract(const BitArray& ba, size_t& pos) {
		auto getter = [&ba,&pos]()->uint8_t {
			uint8_t v = ba.bits(pos, 8); pos += 8; return v;
		};
		return coder::VByte::decode_f(getter);
	}
};

struct DeltaCodeStream {
	static void append(OBitStream& out, uint64_t val) {
		unsigned l = msb_intr(val);
		out.puts(coder::GammaCoder::encode(l + 1));
		out.puts(val - (1ull << l), l);
	}

	static uint64_t extract(IWBitStream& is) {
		throw std::runtime_error("not implemented");
		return 0;
	}
	static uint64_t extract(const BitArray& ba, size_t start) {

	}

};

}//namespace