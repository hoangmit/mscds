#pragma once

#include "bitarray/bitop.h"
#include <stdint.h>

namespace coder{

struct RRR_Codec {
	typedef uint64_t CodeTp;
	RRR_Codec();
	CodeTp encode(uint8_t n, uint8_t k, CodeTp val) const;
	CodeTp decode(uint8_t n, uint8_t k, CodeTp offset) const;

	uint8_t offset_len(uint8_t n, uint8_t k) const;

	uint8_t rank(uint8_t n, uint8_t k, CodeTp val) const;
	uint8_t select(uint8_t n, uint8_t k, CodeTp val) const;

	static uint64_t nCk_val(unsigned n, unsigned k);
	static void _init_tables();
	static uint64_t _nCk[65*32 + 1];
	static uint8_t _code_len[65*64 + 1];
};

}//namespace
