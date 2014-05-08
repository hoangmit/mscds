#pragma once

/** 
Implement VByte coding
*/

#include <stdint.h>

namespace coder {
struct VByte {
	template<typename OutputIterator>
	static void encode(uint64_t val, OutputIterator output) {
		while (val >= 128) {
			uint8_t c = (val & 127);
			*output++ = c | 128;
			val >>= 7;
		}
		*output++ = val;
	}
	template<typename InputIterator>
	static uint64_t decode(InputIterator input) {
		uint64_t val = 0;	
		for (unsigned shift = 0; shift < 64; shift += 7) {
			uint8_t temp = *input++;
			val |= ((temp&127) << shift);
			if (temp < 128) break;
		}
		return val;
	}
	//
	template<typename OutFunc>
	static void encode_f(uint64_t val, OutFunc fx) {
		while (val >= 128) {
			uint8_t c = (val & 127);
			fx(c | 128);
			val >>= 7;
		}
		fx(val);
	}
	template<typename InpFunc>
	static uint64_t decode_f(InpFunc fx) {
		uint64_t val = 0;
		for (unsigned shift = 0; shift < 64; shift += 7) {
			uint8_t temp = fx();
			val |= ((temp & 127) << shift);
			if (temp < 128) break;
		}
		return val;
	}
};

}//namespace
