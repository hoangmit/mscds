
#pragma once

#include <stdint.h>
#include <cassert>

namespace coder {

// Modifty from
// Simple byte-aligned rANS encoder/decoder - public domain - Fabian 'ryg' Giesen 2014

typedef uint32_t RansState;
typedef uint32_t SymbolTp;

// L ('l' in the paper) is the lower bound of our normalization interval.
// Between this and our byte-aligned emission, we use 31 (not 32!) bits.
// This is done intentionally because exact reciprocals for 31-bit uints
// fit in 32-bit uints: this permits some optimizations during encoding.
#define RANS_BYTE_L (1u << 23) 


struct RansEncOp {
	// Initialize a rANS encoder.
	static RansState init() {
		return RANS_BYTE_L;
	}

	// Renormalize the encoder. Internal function.
	template<typename OStream>
	static RansState _renorm(RansState x, OStream* pptr, uint32_t freq, uint32_t scale_bits) {
		uint32_t x_max = ((RANS_BYTE_L >> scale_bits) << 8) * freq; // this turns into a shift.
		while (x >= x_max) {
			pptr->put((uint8_t) (x & 0xff));
			x >>= 8;
		}
		return x;
	}

    static RansState _update(RansState x, uint32_t start, uint32_t freq, uint32_t scale_bits) {
        return ((x / freq) << scale_bits) + (x % freq) + start;
    }
	
	// Encodes a single symbol with range start "start" and frequency "freq".
	// All frequencies are assumed to sum to "1 << scale_bits", and the
	// resulting bytes get written to ptr (which is updated).
	//
	// NOTE: With rANS, you need to encode symbols in *reverse order*, i.e. from
	// beginning to end! Likewise, the output bytestream is written *backwards*:
	template<typename OStream>
	static RansState put(RansState r, OStream* pptr, uint32_t start, uint32_t freq, uint32_t scale_bits) {
		// renormalize
		RansState x = _renorm(r, pptr, freq, scale_bits);
		// x = C(s,x)
        return _update(x,  start, freq, scale_bits);
	}
	
	// Flushes the rANS encoder.
	template<typename OStream>
	static RansState flush(RansState x, OStream* pptr) {
		pptr->put((uint8_t) (x));
		pptr->put((uint8_t) (x >> 8));
		pptr->put((uint8_t) (x >> 16));
		pptr->put((uint8_t) (x >> 24));
		return x;
	}
};


struct RansDecOp {
	
	// Initializes a rANS decoder.
	// Unlike the encoder, the decoder works forwards as you'd expect.
	template<typename IStream>
	static RansState init(IStream* pptr) {
		uint32_t x = pptr->get();
		x = (x << 8) | pptr->get();
		x = (x << 8) | pptr->get();
		x = (x << 8) | pptr->get();
		return x;
	}
	
	// Returns the current cumulative frequency (map it to a symbol yourself!)
	static SymbolTp get(uint32_t scale_bits, RansState* r) {
        uint32_t mask = (~0u) >> (32 - scale_bits);
        return *r & mask;
	}
	
	// Advances in the bit stream by "popping" a single symbol with range start
	// "start" and frequency "freq". All frequencies are assumed to sum to "1 << scale_bits",
	// and the resulting bytes get written to ptr (which is updated).
	template<typename IStream>
	static RansState advance(RansState r, IStream* pptr, uint32_t start, uint32_t freq, uint32_t scale_bits)
	{
		//uint32_t mask = (1u << scale_bits) - 1;
		uint32_t mask = (~0u) >> (32 - scale_bits);

		// s, x = D(x)
		uint32_t x = r;
		x = freq * (x >> scale_bits) + (x & mask) - start;

		// renormalize
		while (x < RANS_BYTE_L) {
			x = (x << 8) | pptr->get();
		}
		return x;
	}

	// Renormalize.
	template<typename IStream>
	static RansState renorm(RansState r, IStream* pptr) {
		// renormalize
		uint32_t x = r;
		while (x < RANS_BYTE_L) {
			x = (x << 8) | pptr->get();
		}
		return x;
	}
};

}//namespace
