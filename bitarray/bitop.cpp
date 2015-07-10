#include "bitop.h"

namespace mscds {
	static bool lmsb_init_flag = false;
	static unsigned int _msb_table[256], _lsb_table[256];
	

	void lmsb_init_tables(void)
	{
		int i, j, lsb, msb, mask;
		j = 1;
		msb = -1;
		for (i=1; i < 256; i++) {
			if (i == j) {
				j <<= 1;
				msb++;
			}
			_msb_table[i] = msb;

			for (lsb=0,mask=1; !(mask & i); lsb++,mask<<=1) ;
			_lsb_table[i] = lsb;
		}
		lmsb_init_flag = true;
	}

	unsigned int lsb_table32(uint32_t number) {
		//if (!lmsb_init_flag) init_tables();
		if (number & 0xff) return _lsb_table[number & 0xff];
		number >>= 8;
		if (number & 0xff) return 8 + _lsb_table[number & 0xff];
		number >>= 8;
		if (number & 0xff) return 16 + _lsb_table[number & 0xff];
		number >>= 8;
		if (number & 0xff) return 24 + _lsb_table[number & 0xff];
		return 64;
	}

	unsigned int msb_table32(uint32_t number) {
		//if (!lmsb_init_flag) init_tables();
		if (number & (0xff << 24)) return 24 + _msb_table[number >> 24];
		if (number & (0xff << 16)) return 16 + _msb_table[number >> 16];
		if (number & (0xff << 8)) return 8 + _msb_table[number >> 8];
		if (number & 0xff) return _msb_table[number];
		return 64;
	}

	unsigned int lsb_table(uint64_t number) {
		if (!lmsb_init_flag) lmsb_init_tables();
		if (number & 0xffffffffULL) return lsb_table32(number & 0xffffffffULL);
		else return lsb_table32(number >> 32);
	}
	
	unsigned int msb_table(uint64_t number) {
		if (!lmsb_init_flag) lmsb_init_tables();
		if (number & 0xffffffff00000000ULL) return 32 + msb_table32(number >> 32);
		else return msb_table32(number);
	}

	unsigned int msb_debruijn32(uint32_t v) {
		static const uint8_t DeBruijnClz[32] = {0, 9, 1, 10, 13, 21, 2, 29, 11, 14,
			16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26,
			5, 4, 31};
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		return DeBruijnClz[(uint32_t)(v * 0x07C4ACDDU) >> 27];
	}

	unsigned int lsb_debruijn32(uint32_t v) {
		static const uint8_t DeBruijnCtz[32] = {
			0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
			31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
		};
		return DeBruijnCtz[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
	}

	uint8_t revbits_table8(uint8_t b) {
		static const unsigned char BitReverseTable256[256] =
		{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
			R6(0), R6(2), R6(1), R6(3)
		};
		return BitReverseTable256[b];
	}
};
