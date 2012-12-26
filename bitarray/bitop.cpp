#include "bitop.h"

namespace mscds {
	bool lmsb_init_flag = false;
	unsigned int _msb_table[256], _lsb_table[256];
	

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
};
