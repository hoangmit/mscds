#pragma once

#include <cassert>
#include "stdint.h"

#if defined (_MSC_VER)
#include <intrin.h>
//http://msdn.microsoft.com/en-us/library/fbxyd7zd%28v=VS.80%29.aspx

namespace utils{
#if defined (_WIN64)
	#pragma intrinsic(_BitScanReverse64)
	inline unsigned int find_MSB_intr(unsigned long long number) {
		unsigned long index;
		_BitScanReverse64(&index, number);
		return index;
	}

	#pragma intrinsic(_BitScanForward64)
	inline unsigned int find_LSB_intr(unsigned long long number) {
		unsigned long index;
		_BitScanForward64(&index, number);
		return index;
	}

	#pragma intrinsic(__popcnt64)
	inline unsigned int popcount(unsigned long long number) {
		return (unsigned int) __popcnt64(number);
	}

#else
	// use _BitScanReverse64, _BitScanForward64 and __popcnt64 for 64x compiler
	#pragma intrinsic(_BitScanReverse)
	inline unsigned int find_MSB_intr(unsigned long long number) {
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
	inline unsigned int find_LSB_intr(unsigned long long number) {
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
	inline unsigned int popcount(unsigned long long number) {
		return __popcnt(number >> 32) + __popcnt(number & 0xFFFFFFFFull);
	}
#endif
}


#else

#if defined(__GNUC__)

namespace utils {
	inline unsigned int find_MSB_intr(unsigned long long number) {
		return sizeof(number) * 8 -  __builtin_clzll(number) - 1;
	}

	inline unsigned int find_LSB_intr(unsigned long long number) {
		return __builtin_ctzll(number);
	}
	inline unsigned int popcount(unsigned long long number) {
		return __builtin_popcountll(number);
	}
}
#endif

#endif

namespace utils {

	/** \brief returns the value of ceiling of log_2(n) */
	inline uint64_t ceillog2(uint64_t n) {
		if (n == 0) return 0;
		return find_MSB_intr(n) + 1;
	}
}
