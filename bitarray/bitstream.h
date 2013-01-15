#pragma once

#include <stdint.h>
#include <vector>
#include <cassert>
#include <sstream>
#include "archive.h"

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

	void puts(const std::pair<uint64_t, uint16_t>& code) {
		puts(code.first, code.second);
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
			j = 0;
			cur = 0;	
		}
	}

	std::string to_str() const {
		std::ostringstream ss;
		for (int i = 0; i < bitlen; ++i)
			ss << ((getbit(i)) ? 1 : 0);
		return ss.str();
	}

	size_t length() const { return bitlen; }
	size_t word_count() const { return os.size(); }
	uint64_t* data_ptr() { return os.data(); }
private:
	bool getbit(uint64_t pos) const { return (os[pos/WORDLEN] & (1ull << (pos%WORDLEN))) > 0; }
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


class IWBitStream {
private:
	uint64_t cur, nxt;
	size_t blen;
	uint16_t j;
	
	void init(const uint64_t * _ptr, size_t blen) {
		this->ptr = _ptr;
		this->blen = blen;
		if (blen > 0) {
			cur = *ptr;
			++ptr;
		}
		nxt = 0;
		j = 0;
	}

	SharedPtr handle;
	const uint64_t *ptr;
public:
	const static uint16_t WORDLEN = 64;
	IWBitStream(){ clear(); }

	IWBitStream(const uint64_t * _ptr, size_t idx, size_t blen) {
		init(_ptr + (idx / WORDLEN), blen + (idx % WORDLEN));
		skipw(idx % WORDLEN);
	}

	IWBitStream(SharedPtr p, size_t idx, size_t blen) {
		uint64_t * ptr = (uint64_t*) p.get();
		handle = p;
		init(ptr + (idx / WORDLEN), blen + (idx % WORDLEN));
		skipw(idx% WORDLEN);
	}

	void clear() {
		j = 0;
		cur = nxt = 0;
		blen = 0;
		ptr = NULL;
	}

	inline void skipw(uint16_t len) {
		assert(len <= WORDLEN && blen >= len);
		if (len == 0) return ;
		if (len < WORDLEN)  cur = (cur >> len) | (nxt << (WORDLEN - len));
		else cur = nxt;
		if (j >= len) {
			nxt >>= len;
			j -= len;
		} else {
			//fetch next word
			if (blen > WORDLEN + j) {
				nxt = *ptr;
				++ptr;
			} else nxt = 0;
			j = WORDLEN + j - len;
			//avoid shifting 64
			if (j > 0) {
				cur |= nxt << j;
				nxt >>= (WORDLEN - j);
			} else {
				cur = nxt;
				nxt = 0;
			}
		}
		blen -= len;
	}

	bool getb() {
		bool b = (cur & 1) > 0;
		skipw(1);
		return b;
	}

	uint64_t get(uint16_t len) {
		if (len == 0) return 0;
		uint64_t v = cur & ((~0ull) >> (WORDLEN - len));
		skipw(len);
		return v;
	}

	uint64_t get() {
		uint64_t v = cur;
		skipw(WORDLEN);
		return v;
	}

	uint64_t peek() {
		return cur;
	}

	const uint64_t* current_ptr() { return ptr; }

	bool empty() { return blen == 0; }

	void close() { clear(); }
};

}//namespace
