#pragma once

#ifndef __BIT_OP_H_
#define __BIT_OP_H_

/**
\file
Implement commmon bit operations within a word

Defines the following functions:

    popcnt, popcnt_comp: count number of 1-bit in a word

    msb_intr, lsb_intr, lsb_table, msb_table: position of "most significant"/"least significant"
	    bit in a word

    revbits: reverse bit order in a word

    selectword, selectword_v2: find position of the k-th 1-bit in a word

    ceillog2: ceil(log2(x))

Written and collected from various sources by Hoang
*/

#include <stdint.h>
#include <cassert>

/// mscds
namespace mscds {

#define ONES_STEP_4 ( 0x1111111111111111ULL )
#define ONES_STEP_8 ( 0x0101010101010101ULL )
#define MSBS_STEP_8 ( 0x80ULL * ONES_STEP_8 )
#define INCR_STEP_8 ( 0x80ULL << 56 | 0x40ULL << 48 | 0x20ULL << 40 | 0x10ULL << 32 | 0x8ULL << 24 | 0x4ULL << 16 | 0x2ULL << 8 | 0x1 )

#define LEQ_STEP_8(x,y) ( ( ( ( ( (y) | MSBS_STEP_8 ) - ( (x) & ~MSBS_STEP_8 ) ) ^ (x) ^ (y) ) & MSBS_STEP_8 ) >> 7 )

#define ZCOMPARE_STEP_8(x) ( ( ( x | ( ( x | MSBS_STEP_8 ) - ONES_STEP_8 ) ) & MSBS_STEP_8 ) >> 7 )

// from "Sebastiano Vigna"  at http://vigna.di.unimi.it/
inline uint64_t selectword_v2(uint64_t x, uint64_t r) {
	uint64_t byte_sums = x - ( ( x & 0xa * ONES_STEP_4 ) >> 1 );
	byte_sums = ( byte_sums & 3 * ONES_STEP_4 ) + ( ( byte_sums >> 2 ) & 3 * ONES_STEP_4 );
	byte_sums = ( byte_sums + ( byte_sums >> 4 ) ) & 0x0f * ONES_STEP_8;
	byte_sums *= ONES_STEP_8;

	// Phase 2: compare each byte sum with k
	const uint64_t k_step_8 = r * ONES_STEP_8;
	const uint64_t place = (LEQ_STEP_8( byte_sums, k_step_8) * ONES_STEP_8 >> 53) & ~0x7;

	// Phase 3: Locate the relevant byte and make 8 copies with incrental masks
	const unsigned int byte_rank = (unsigned int)(r - (((byte_sums << 8) >> place) & 0xFF));

	const uint64_t spread_bits = ( x >> place & 0xFF ) * ONES_STEP_8 & INCR_STEP_8;
	const uint64_t bit_sums = ZCOMPARE_STEP_8(spread_bits) * ONES_STEP_8;

	// Compute the inside-byte location and return the sum
	const uint64_t byte_rank_step_8 = byte_rank * ONES_STEP_8;

	return place + ( LEQ_STEP_8( bit_sums, byte_rank_step_8 ) * ONES_STEP_8 >> 56 );
}

inline uint64_t selectword(uint64_t x, uint64_t r){
	r += 1;
	uint64_t x1 = x - ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
	uint64_t x2 = (x1 & 0x3333333333333333ULL) + ((x1 >> 2) & 0x3333333333333333ULL);
	uint64_t x3 = (x2 + (x2 >> 4)) & 0x0F0F0F0F0F0F0F0FULL;

	uint64_t pos = 0;
	for (;;  pos += 8){
		uint64_t b = (x3 >> pos) & 0xFFULL;
		if (r <= b) break;
		r -= b;
	}

	uint64_t v2 = (x2 >> pos) & 0xFULL;
	if (r > v2) {
		r -= v2;
		pos += 4;
	}

	uint64_t v1 = (x1 >> pos) & 0x3ULL;
	if (r > v1){
		r -= v1;
		pos += 2;
	}

	uint64_t v0  = (x >> pos) & 0x1ULL;
	if (v0 < r){
		r -= v0;
		pos += 1;
	}

	return pos;
}
}//namespace


#if defined (_MSC_VER)
#include <intrin.h>
//http://msdn.microsoft.com/en-us/library/fbxyd7zd%28v=VS.80%29.aspx

namespace mscds{
#if defined (_WIN64)
	#pragma intrinsic(_BitScanReverse64)
	inline unsigned int msb_intr(unsigned long long int number) {
		unsigned long index;
		_BitScanReverse64(&index, number);
		return index;
	}

	#pragma intrinsic(_BitScanForward64)
	inline unsigned int lsb_intr(unsigned long long int number) {
		unsigned long index;
		_BitScanForward64(&index, number);
		return index;
	}

	#pragma intrinsic(__popcnt64)
	inline unsigned int popcnt(unsigned long long int number) {
		return (unsigned int) __popcnt64(number);
	}

	#pragma intrinsic(_BitScanReverse)
	inline unsigned int msb_intr32(unsigned int number) {
		unsigned long index;
		_BitScanReverse(&index, number);
		return index;
	}

	#pragma intrinsic(_BitScanForward)
	inline unsigned int lsb_intr32(unsigned int number) {
		unsigned long index;
		_BitScanForward(&index, number);
		return index;
	}

	#pragma intrinsic(__popcnt)
	inline unsigned int popcnt32(unsigned int number) {
		return (unsigned int) __popcnt(number);
	}

#else
	// use _BitScanReverse64, _BitScanForward64 and __popcnt64 for 64x compiler
	#pragma intrinsic(_BitScanReverse)
	inline unsigned int msb_intr(unsigned long long int number) {
		unsigned long index;
		//unsigned char isNonzero;

		assert(number != 0);
		unsigned int hi = number >> 32;
		unsigned int lo = number & 0xFFFFFFFFull;
		if (hi == 0) {
			_BitScanReverse(&index, lo);
		}
		else {
			_BitScanReverse(&index, hi);
			index += 32;
		}
		return index;
	}

	#pragma intrinsic(_BitScanForward)
	inline unsigned int lsb_intr(unsigned long long int number) {
		unsigned long index;
		//unsigned char isNonzero;
		assert(number != 0);
		unsigned int hi = number >> 32;
		unsigned int lo = number & 0xFFFFFFFFull;
		if (lo == 0) {
			_BitScanForward(&index, hi);
			index += 32;
		}else
			_BitScanForward(&index, lo);
		return index;
	}

	#pragma intrinsic(__popcnt)
	inline unsigned int popcnt(unsigned long long int number) {
		return __popcnt(number >> 32) + __popcnt(number & 0xFFFFFFFFull);
	}

	#pragma intrinsic(_BitScanReverse)
	inline unsigned int msb_intr32(unsigned int number) {
		unsigned long index;
		_BitScanReverse(&index, number);
		return index;
	}

	#pragma intrinsic(_BitScanForward)
	inline unsigned int lsb_intr32(unsigned int number) {
		unsigned long index;
		_BitScanForward(&index, number);
		return index;
	}

	#pragma intrinsic(__popcnt)
	inline unsigned int popcnt32(unsigned int number) {
		return (unsigned int)__popcnt(number);
	}
#endif
}


#endif // MSVC

#if defined(__GNUC__)

namespace mscds {
	inline unsigned int msb_intr(unsigned long long int number) {
		return sizeof(number) * 8 -  __builtin_clzll(number) - 1;
	}

	inline unsigned int lsb_intr(unsigned long long int number) {
		return __builtin_ctzll(number);
	}

	inline unsigned int popcnt(unsigned long long int number) {
		return __builtin_popcountll(number);
	}

	inline unsigned int msb_intr32(unsigned int number) {
		return sizeof(number) * 8 -  __builtin_clz(number) - 1;
	}

	inline unsigned int lsb_intr32(unsigned int number) {
		return __builtin_ctz(number);
	}

	inline unsigned int popcnt32(unsigned int number) {
		return __builtin_popcount(number);
	}
}
#endif // GNUC

namespace mscds {
	inline uint8_t revbits(uint8_t c) {
		return (uint8_t)((c * 0x0202020202ULL & 0x010884422010ULL) % 1023);
	}

	inline uint32_t revbits(uint32_t v) {
		uint32_t c;
		unsigned char * p = (unsigned char *) &v;
		unsigned char * q = (unsigned char *) &c;
		q[3] = revbits(p[0]);
		q[2] = revbits(p[1]);
		q[1] = revbits(p[2]);
		q[0] = revbits(p[3]);
		return c;
	}

	inline unsigned int popcnt_comp(uint64_t x) {
		x = x - ((x & 0xAAAAAAAAAAAAAAAAULL) >> 1);
		x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
		x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
		return ((x * 0x0101010101010101ULL) >> 56) & 0x7FULL;
	}

	inline unsigned int popcnt_comp32(uint32_t x) {
		/*
		x = x  - ((x >> 1)  & 0x33333333333u)
			- ((x >> 2)  & 0x11111111111u);
		x = (x +  (x >> 3)) & 0x30707070707u;
		return x % 63;
		*/		
		x = x - ((x >> 1) & 0x55555555u);
		x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
		return (((x + (x >> 4)) & 0x0F0F0F0Fu) * 0x01010101u) >> 24;
	}
	unsigned int lsb_table(uint64_t number);
	unsigned int msb_table(uint64_t number);
	unsigned int lsb_table32(uint32_t number);
	unsigned int msb_table32(uint32_t number);

	
}//namespace


namespace mscds {

	/** \brief returns the value of ceiling of log_2(n) */
	inline unsigned int ceillog2(uint64_t n) {
		if (n == 0) return 0;
		return msb_intr(n) + (n&(n-1) ? 1 : 0);
	}

	inline unsigned int ceillog2_32(uint32_t n) {
		if (n == 0) return 0;
		return msb_intr32((unsigned int) n) + (n&(n-1) ? 1 : 0);
	}

	/// returns floor(log2(n))
	inline unsigned int floorlog2(uint64_t n) {
		if (n == 0) return 0;
		return msb_intr(n);
	}

	inline unsigned int floorlog2_32(uint32_t n) {
		if (n == 0) return 0;
		return msb_intr32(n);
	}

	inline unsigned int val_bit_len(uint64_t n) {
		if (n == 0) return 0;
		else return msb_intr(n) + 1;
	}

	inline unsigned int val_bit_len32(uint32_t n) {
		if (n == 0) return 0;
		else return msb_intr32(n) + 1;
	}
}


#endif //__BIT_OP_H_