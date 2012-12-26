#pragma once

#include <stdint.h>
#include <vector>
#include <cassert>

namespace mscds {

class OBitStream {
public:
	OBitStream(): cur(0), j(0), bitlen(0) {}
	~OBitStream() { assert(j == 0); }
	void put0() { pushout(); }
	void put1() { cur |= (1ull << j);  pushout(); }
	void put(bool bit) { if (bit) put1(); else put0(); }
	void puts(uint64_t v, uint16_t len) {
		assert(len <= WORDLEN);
		if (len == 0) return ;
		v &= (~0ull) >> (WORDLEN - len);
		cur |= (v << j);
		if (j + len >= WORDLEN) {
			os.push_back(cur);
			cur = (v >> (WORDLEN - j));
			j -= WORDLEN - len;
		} else j += len;
		bitlen += len;
	}

	void puts(uint64_t v) {
		cur |= (v << j);
		os.push_back(cur);
		cur = (v >> (WORDLEN - j));
		bitlen += WORDLEN;
	}

	void clear() {
		os.clear();
		cur = 0;
		j = 0;
		bitlen = 0;
	}

	void close() {
		if (j > 0) {
			os.push_back(cur);
			bitlen += j;
			j = 0;
			cur = 0;	
		}
	}

	size_t length() const { return bitlen; }
	uint64_t* data() { return os.data(); }
private:
	const static uint16_t WORDLEN = 64;
	inline void pushout() {
		++j;
		++bitlen;
		if (j==WORDLEN) {
			os.push_back(cur);
			j = 0;
			cur = 0;
		}
	}
	
	uint64_t cur;
	size_t bitlen;
	uint16_t j;	
	std::vector<uint64_t> os;
};
//TODO: import code from extsds

}//namespace
