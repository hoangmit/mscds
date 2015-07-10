

#include "rans_byte.hpp"
#include <stdlib.h>

namespace coder {

// Encoder symbol description
// This (admittedly odd) selection of parameters was chosen to make
// RansEncPutSymbol as cheap as possible.
struct RansEncSymbol {
    uint32_t x_max;     // (Exclusive) upper bound of pre-normalization interval
    uint32_t rcp_freq;  // Fixed-point reciprocal frequency
    uint32_t bias;      // Bias
    uint16_t cmpl_freq; // Complement of frequency: (1 << scale_bits) - freq
    uint16_t rcp_shift; // Reciprocal shift};
};

struct RansDecSymbol {
    uint16_t start;     // Start of range.
    uint16_t freq;      // Symbol frequency.
};

struct RansEncSymbolOp {
    static void init(RansEncSymbol* s, uint32_t start, uint32_t freq, uint32_t scale_bits);
	
	template<typename OStream>
	static RansState put(RansEncSymbol const * s, RansState r, OStream* pptr) {
		assert(sym->x_max != 0); // can't encode symbol with freq=0

		// renormalize
		uint32_t x = r;
		while (x >= s->x_max) {
            pptr->put((uint8_t) (x & 0xff));
			x >>= 8;
		}

		// x = C(s,x)
		// NOTE: written this way so we get a 32-bit "multiply high" when
		// available. If you're on a 64-bit platform with cheap multiplies
		// (e.g. x64), just bake the +32 into rcp_shift.
        uint32_t q = (uint32_t) (((uint64_t)x * s->rcp_freq) >> 32) >> s->rcp_shift;
        return x + s->bias + q * s->cmpl_freq;
	} 
};


struct RansDecSymbolOp {
	// Initialize a decoder symbol to start "start" and frequency "freq"
    static void init(RansDecSymbol* s, uint32_t start, uint32_t freq);
	
	// Equivalent to advance that takes a symbol.
	template<typename IStream>
	static RansState advance(RansDecSymbol const* sym, RansState r, IStream* pptr, uint32_t scale_bits) {
		return RansDecOp::advance(r, pptr, sym->start, sym->freq, scale_bits);
	}
	
	// Advances in the bit stream by "popping" a single symbol with range start
	// "start" and frequency "freq". All frequencies are assumed to sum to "1 << scale_bits".
	// No renormalization or output happens.
	template<typename IStream>
	static RansState _advanceStep(RansState r, uint32_t start, uint32_t freq, uint32_t scale_bits) {
		uint32_t mask = (~0u) >> (32 - scale_bits);

		// s, x = D(x)
		uint32_t x = r;
		return freq * (x >> scale_bits) + (x & mask) - start;
	}
	
	// Equivalent to advanceStep that takes a symbol.
	template<typename IStream>
	static RansState advanceSymbolStep(RansDecSymbol const* sym, RansState r, uint32_t scale_bits) {
		return _advanceStep<IStream>(r, sym->start, sym->freq, scale_bits);
	}
	
};


struct NormSymbolStats {
    static const unsigned LOG2NSYMS = 8;
    static const unsigned NSYMS = 1 << LOG2NSYMS;

    uint32_t freqs[NSYMS];
    uint32_t cum_freqs[NSYMS + 1];

    template<typename IStream>
    void count_freqs(IStream * in, size_t nbytes) {
        for (unsigned i=0; i < NSYMS; i++) freqs[i] = 0;
        while (in->hasNext()) ++freqs[in->get()];
    }
    void calc_cum_freqs();
    void normalize_freqs(uint32_t target_total);
};

}//namespace
