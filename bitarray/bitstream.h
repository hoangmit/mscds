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

	void put0();
	void put1();
	void put(bool bit);
	void puts(uint64_t v, uint16_t len);
	void puts(const std::pair<uint64_t, uint16_t>& code);
	void puts(uint64_t v);

	void clear();
	void close();
	std::string to_str() const;

	size_t length() const { return bitlen; }
	size_t word_count() const { return os.size(); }
	uint64_t* data_ptr() { return os.data(); }
private:
	bool getbit(uint64_t pos) const { return (os[pos/WORDLEN] & (1ull << (pos%WORDLEN))) > 0; }
	const static uint16_t WORDLEN = 64;
	void pushout();
	
	uint64_t cur;
	size_t bitlen;
	uint16_t j;	
	std::vector<uint64_t> os;
};

inline void OBitStream::put0() { pushout(); }
inline void OBitStream::put1() { cur |= (1ull << j);  pushout(); }
inline void OBitStream::put(bool bit) { if (bit) put1(); else put0(); }

inline void OBitStream::puts(uint64_t v, uint16_t len) {
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

inline void OBitStream::puts(const std::pair<uint64_t, uint16_t>& code) {
	puts(code.first, code.second);
}


inline void OBitStream::puts(uint64_t v) {
	cur |= (v << j);
	os.push_back(cur);
	cur = (v >> (WORDLEN - j));
	bitlen += WORDLEN;
}

inline void OBitStream::clear() {
	os.clear();
	cur = 0;
	j = 0;
	bitlen = 0;
}

inline void OBitStream::close() {
	if (j > 0) {
		os.push_back(cur);
		j = 0;
		cur = 0;	
	}
}

inline std::string OBitStream::to_str() const {
	std::ostringstream ss;
	for (int i = 0; i < bitlen; ++i)
		ss << ((getbit(i)) ? 1 : 0);
	return ss.str();
}

inline void OBitStream::pushout() {
	++j;
	++bitlen;
	if (j==WORDLEN) {
		os.push_back(cur);
		j = 0;
		cur = 0;
	}
}


//------------------------------------------------------------------------

class IWBitStream {
public:
	const static uint16_t WORDLEN = 64;
	IWBitStream(){ clear(); }

	void init(const uint64_t * _ptr, size_t blen, size_t idx = 0);

	IWBitStream(const uint64_t * _ptr, size_t blen, size_t idx = 0) {
		init(_ptr, blen, idx);
	}

	IWBitStream(SharedPtr p, size_t idx, size_t blen) {
		uint64_t * ptr = (uint64_t*) p.get();
		handle = p;
		init(ptr, blen, idx);
	}

	void clear();

	void skipw(uint16_t len);

	bool getb();
	uint64_t get(uint16_t len);
	uint64_t get();
	uint64_t peek() const { return cur; }

	const uint64_t* current_ptr() { return ptr; }
	bool empty() { return blen == 0; }
	void close() { clear(); }
private:
	uint64_t cur, nxt;
	size_t blen;
	uint16_t j;

	SharedPtr handle;
	const uint64_t *ptr;

	void init_(const uint64_t * _ptr, size_t blen);
};


inline void IWBitStream::init(const uint64_t * _ptr, size_t blen, size_t idx) {
	init_(_ptr + (idx / WORDLEN), blen - idx + (idx % WORDLEN));
	skipw(idx % WORDLEN);
}


inline void IWBitStream::init_(const uint64_t * _ptr, size_t blen) {
	this->ptr = _ptr;
	this->blen = blen;
	if (blen > 0) {
		cur = *ptr;
		++ptr;
	}
	nxt = 0;
	j = 0;
}

inline void IWBitStream::clear() {
	j = 0;
	cur = nxt = 0;
	blen = 0;
	ptr = NULL;
	handle.reset();
}

inline void IWBitStream::skipw(uint16_t len) {
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

inline bool IWBitStream::getb() {
	bool b = (cur & 1) > 0;
	skipw(1);
	return b;
}

inline uint64_t IWBitStream::get(uint16_t len) {
	if (len == 0) return 0;
	uint64_t v = cur & ((~0ull) >> (WORDLEN - len));
	skipw(len);
	return v;
}

}//namespace
