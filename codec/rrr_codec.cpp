#include "rrr_codec.h"

#include <cstdlib>
#include <cstring>
namespace coder {

static bool _table_ready = false;
uint64_t RRR_Codec::_nCk[65*32 + 1];
uint8_t RRR_Codec::_code_len[65*64 + 1];

RRR_Codec::RRR_Codec() { _init_tables(); }

RRR_Codec::CodeTp RRR_Codec::encode(uint8_t n, uint8_t k, RRR_Codec::CodeTp val) {
	CodeTp r = 0;
	if (k == 0 || n == k) return 0;
	for (int j = 0; k > 0; ++j) {
		if (val & 1) {
			int hold = k > (n - j - 1) / 2 ? n - j - 1 - k : k;
			if (n - j == k) break;
			r += _nCk[(n - j - 1) * 32 + hold]; //Increment r by (n - j - 1) C k
			--k;
		}
		val >>= 1;
	}
	return r;
}

RRR_Codec::CodeTp RRR_Codec::decode(uint8_t n, uint8_t k, RRR_Codec::CodeTp offset) {
	if (k == 0) return 0;
	CodeTp mask = (~0ull) >> (64-n);
	if (k == n) return mask;
	CodeTp word = 0;
	unsigned int j = 0;

	while (k > 0) {
		int hold = k > (n - j - 1) / 2 ? n - j - 1 - k : k;
		if (n - j == k) {
			word |= ~((1ull << j) - 1) & mask;
			break;
		}
		if (offset >= _nCk[(n - j - 1) * 32 + hold]) {
			word |= (1ull << j);
			offset -= _nCk[(n - j - 1) * 32 + hold];
			--k;
		}
		++j;
	}
	return word;
}

uint8_t RRR_Codec::offset_len(uint8_t n, uint8_t k) {
	return _code_len[n*64+k];
}

uint64_t RRR_Codec::nCk_val(unsigned n, unsigned k) {
	assert(n <= 64 && k <= n);
	if (k <= n/2) return _nCk[n*32 + k];
	else return _nCk[n*32 + n - k];
}

void RRR_Codec::_init_tables() {
	if (_table_ready) return;
	memset(_nCk, 0, sizeof(_nCk));
	memset(_code_len, 0, sizeof(_code_len));
	_nCk[0] = 1;
	for (unsigned n = 1; n <= 64; ++n) {
		_nCk[n*32 + 0] = 1;
		for (unsigned k = 1; k <= n/2; ++k) {
			_nCk[n * 32 + k] = _nCk[(n-1)*32 + k] + _nCk[(n-1)*32 + k - 1];
		}
		if (n % 2 == 0) _nCk[n*32 + n/2] += _nCk[(n-1)*32 + n/2 - 1];
	}
	_nCk[64*32 + 32] -= 1; // wrap around table
	for (unsigned n = 0; n <= 64; ++n) {
		for (unsigned k = 0; k <= n/2; ++k) {
			_code_len[n*64+k] = mscds::ceillog2(_nCk[n*32+k]);
		}
		for (unsigned k = n/2 + 1; k <= n; ++k) {
			_code_len[n*64+k] = _code_len[n*64 + n - k];
		}
	}
	_table_ready = true;
}

}//namespace