#pragma once

#ifndef __BIT_OP_H_
#define __BIT_OP_H_

#include <stdint.h>

namespace mscds {

inline unsigned int popcnt(uint64_t x) {
	x = x - ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
	return ((x * 0x0101010101010101ULL) >> 56) & 0x7FULL;
}

#define ONES_STEP_4 ( 0x1111111111111111ULL )
#define ONES_STEP_8 ( 0x0101010101010101ULL )
#define MSBS_STEP_8 ( 0x80ULL * ONES_STEP_8 )
#define INCR_STEP_8 ( 0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 | 0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1 )

#define LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( (x) & ~MSBS_STEP_8 ) ) ^ (x) ^ (y) ) & MSBS_STEP_8 ) >> 7 )

#define ZCOMPARE_STEP_8(x) ( ( ( x | ( ( x | MSBS_STEP_8 ) - ONES_STEP_8 ) ) & MSBS_STEP_8 ) >> 7 )

inline uint64_t selectwrd(uint64_t x, uint64_t r) {
	register uint64_t byte_sums = x - ( ( x & 0xa * ONES_STEP_4 ) >> 1 );
	byte_sums = ( byte_sums & 3 * ONES_STEP_4 ) + ( ( byte_sums >> 2 ) & 3 * ONES_STEP_4 );
	byte_sums = ( byte_sums + ( byte_sums >> 4 ) ) & 0x0f * ONES_STEP_8;
	byte_sums *= ONES_STEP_8;

	// Phase 2: compare each byte sum with k
	const uint64_t k_step_8 = r * ONES_STEP_8;
	const uint64_t place = (LEQ_STEP_8( byte_sums, k_step_8) * ONES_STEP_8 >> 53) & ~0x7;

	// Phase 3: Locate the relevant byte and make 8 copies with incrental masks
	const int byte_rank = r - (((byte_sums << 8) >> place) & 0xFF);

	const uint64_t spread_bits = ( x >> place & 0xFF ) * ONES_STEP_8 & INCR_STEP_8;
	const uint64_t bit_sums = ZCOMPARE_STEP_8(spread_bits) * ONES_STEP_8;

	// Compute the inside-byte location and return the sum
	const uint64_t byte_rank_step_8 = byte_rank * ONES_STEP_8;

	return place + ( LEQ_STEP_8( bit_sums, byte_rank_step_8 ) * ONES_STEP_8 >> 56 );

}

}//namespace
#endif //__BIT_OP_H_