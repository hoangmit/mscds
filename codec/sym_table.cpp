

#include "sym_table.h"

namespace coder {

void RansEncSymbolOp::init(RansEncSymbol *s, uint32_t start, uint32_t freq, uint32_t scale_bits) {
	assert(scale_bits <= 16);
	assert(start <= (1u << scale_bits));
	assert(freq <= (1u << scale_bits) - start);

	// Say M := 1 << scale_bits.
	//
	// The original encoder does:
	//   x_new = (x/freq)*M + start + (x%freq)
	//
	// The fast encoder does (schematically):
	//   q     = mul_hi(x, rcp_freq) >> rcp_shift   (division)
	//   r     = x - q*freq                         (remainder)
	//   x_new = q*M + bias + r                     (new x)
	// plugging in r into x_new yields:
	//   x_new = bias + x + q*(M - freq)
	//        =: bias + x + q*cmpl_freq             (*)
	//
	// and we can just precompute cmpl_freq. Now we just need to
	// set up our parameters such that the original encoder and
	// the fast encoder agree.

	s->x_max = ((RANS_BYTE_L >> scale_bits) << 8) * freq;
	s->cmpl_freq = (uint16_t) ((1 << scale_bits) - freq);
	if (freq < 2) {
		// freq=0 symbols are never valid to encode, so it doesn't matter what
		// we set our values to.
		//
		// freq=1 is tricky, since the reciprocal of 1 is 1; unfortunately,
		// our fixed-point reciprocal approximation can only multiply by values
		// smaller than 1.
		//
		// So we use the "next best thing": rcp_freq=0xffffffff, rcp_shift=0.
		// This gives:
		//   q = mul_hi(x, rcp_freq) >> rcp_shift
		//     = mul_hi(x, (1<<32) - 1)) >> 0
		//     = floor(x - x/(2^32))
		//     = x - 1 if 1 <= x < 2^32
		// and we know that x>0 (x=0 is never in a valid normalization interval).
		//
		// So we now need to choose the other parameters such that
		//   x_new = x*M + start
		// plug it in:
		//     x*M + start                   (desired result)
		//   = bias + x + q*cmpl_freq        (*)
		//   = bias + x + (x - 1)*(M - 1)    (plug in q=x-1, cmpl_freq)
		//   = bias + 1 + (x - 1)*M
		//   = x*M + (bias + 1 - M)
		//
		// so we have start = bias + 1 - M, or equivalently
		//   bias = start + M - 1.
		s->rcp_freq = ~0u;
		s->rcp_shift = 0;
		s->bias = start + (1 << scale_bits) - 1;
	} else {
		// Alverson, "Integer Division using reciprocals"
		// shift=ceil(log2(freq))
		uint32_t shift = 0;
		while (freq > (1u << shift))
			shift++;

		s->rcp_freq = (uint32_t) (((1ull << (shift + 31)) + freq-1) / freq);
		s->rcp_shift = shift - 1;

		// With these values, 'q' is the correct quotient, so we
		// have bias=start.
		s->bias = start;
	}
}


void RansDecSymbolOp::init(RansDecSymbol *s, uint32_t start, uint32_t freq) {
	assert(start <= (1 << 16));
	assert(freq <= (1 << 16) - start);
	s->start = (uint16_t) start;
	s->freq = (uint16_t) freq;
}


//----------------------------------------------------------------------



void NormSymbolStats::calc_cum_freqs() {
	cum_freqs[0] = 0;
	for (unsigned i = 0; i < NSYMS; i++)
		cum_freqs[i+1] = cum_freqs[i] + freqs[i];
}

void NormSymbolStats::normalize_freqs(uint32_t target_total) {
	assert(target_total >= NSYMS);

	calc_cum_freqs();
	uint32_t cur_total = cum_freqs[NSYMS];

	// resample distribution based on cumulative freqs
	for (int i = 1; i <= NSYMS; i++)
		cum_freqs[i] = ((uint64_t)target_total * cum_freqs[i])/cur_total;

	// if we nuked any non-0 frequency symbol to 0, we need to steal
	// the range to make the frequency nonzero from elsewhere.
	//
	// this is not at all optimal, i'm just doing the first thing that comes to mind.
	for (unsigned i = 0; i < NSYMS; i++) {
		if (freqs[i] && cum_freqs[i+1] == cum_freqs[i]) {
			// symbol i was set to zero freq

			// find best symbol to steal frequency from (try to steal from low-freq ones)
			uint32_t best_freq = ~0u;
			int best_steal = -1;
			for (unsigned j = 0; j < NSYMS; j++) {
				uint32_t freq = cum_freqs[j+1] - cum_freqs[j];
				if (freq > 1 && freq < best_freq) {
					best_freq = freq;
					best_steal = j;
				}
			}
			assert(best_steal != -1);

			// and steal from it!
			if (best_steal < i) {
				for (int j = best_steal + 1; j <= i; j++)
					cum_freqs[j]--;
			} else {
				assert(best_steal > i);
				for (int j = i + 1; j <= best_steal; j++)
					cum_freqs[j]++;
			}
		}
	}

	// calculate updated freqs and make sure we didn't screw anything up
	assert(cum_freqs[0] == 0 && cum_freqs[NSYMS] == target_total);
	for (unsigned i = 0; i < NSYMS; i++) {
		if (freqs[i] == 0)
			assert(cum_freqs[i+1] == cum_freqs[i]);
		else
			assert(cum_freqs[i+1] > cum_freqs[i]);

		// calc updated freq
		freqs[i] = cum_freqs[i+1] - cum_freqs[i];
	}
}

}//namespace
