#pragma once

/**
\file
Implement BitSteam class. 

  OBitStream: output bit stream. It provides a bit stream that user can append more bits 
      at the end.

  IWBitStream: input bit stream. This class allows user to extract bits from its front.

  OByteStream: output byte stream.

Implemented by Hoang
*/

#include <stdint.h>
#include <vector>
#include <cassert>
#include <sstream>
#include "../framework/archive.h"
#include "../mem/local_mem.h"
#include "bitarray.h"


namespace mscds {

/// Output Bit Stream
class OBitStream {
public:
	OBitStream(): cur(0), j(0), bitlen(0) {}
	~OBitStream() { assert(j == 0); }

	/// append a 0-bit to the end
	void put0();
	void put1();
	/// append multiple 0-bits to the end
	void put0(uint16_t len);
	void put1(uint16_t len);

	void put(bool bit);
	void put_byte(uint8_t v) { puts(v,8); }
	void puts(uint64_t v, uint16_t len);
	void puts(const std::pair<uint64_t, uint16_t>& code);
	void puts(uint64_t v);
	void puts_c(const char* ptr, size_t byte_count);

	void clear();
	void close();

	bool is_accessiable() const;

	void append(const OBitStream& other);
	void append(const BitArray& ba);

	std::string to_str() const;

	size_t length() const { return bitlen; }
	size_t word_count() const { return os.size(); }

	void build(BitArray* out);
	StaticMemRegionPtr build();
private:
	friend class IWBitStream;
	
	bool getbit(uint64_t pos) const;
	const static uint16_t WORDLEN = 64;
	void pushout();
	
	uint64_t cur;
	size_t bitlen;
	uint16_t j;
	LocalDynamicMem os;
};

/// Input Bit Stream
class IWBitStream {
public:
	const static uint16_t WORDLEN = 64;
	IWBitStream(){ clear(); }
	IWBitStream(OBitStream& os) {
		LocalMemAllocator alloc;
		_own_data = alloc.move(os.os);
		init(&_own_data, os.length(), 0);
	}

	IWBitStream(const BitArray& b) {
		init(&b, b.length(), 0);
	}

	// important
	void init(const WordAccessInterface* _data, size_t blen, size_t start_idx = 0);

	void clear();

	// important
	void skipw(uint16_t len);
	void skipw_other(uint16_t len);

	bool getb();

	/** return the number of 0 bits before the next 1 bit or end of stream;
	    consume all the 0 bits and the last 1 bit */
	unsigned int scan_next1();

	/** return the number of 0 bits before the next 0 bit */
	unsigned int scan_next0();

	uint64_t get(uint16_t len);
	uint64_t get();
	uint64_t peek() const { return cur; }

	bool empty() const { return blen == 0; }
	size_t pos() const { return _init_len - blen; }
	void close() { clear(); }
private:
	uint64_t cur, nxt;
	size_t blen;
	size_t _init_len;
	uint16_t j;
	size_t ptr;
	const WordAccessInterface* data;
	MemRegionWordAccess _own_data;
};


/// Output Byte Stream
class OByteStream {
public:
	OByteStream() {}
	void put(char c) { os.append(c); }
	void puts(const std::string& str);
	void puts(const char* str, unsigned int len);
	size_t length() const { return os.size(); }
	void build(BitArray* out);
	void clear() { os.clear(); }
private:
	LocalDynamicMem os;
};

//-----------------------------------------------------------------------

inline void OBitStream::put0() { pushout(); }
inline void OBitStream::put1() { cur |= (1ull << j);  pushout(); }
inline void OBitStream::put(bool bit) { if (bit) put1(); else put0(); }

inline void OBitStream::put0(uint16_t len) {
	uint64_t v = 0;
	if (len < WORDLEN - j) puts(v, len);
	else {
		auto ln = WORDLEN - j;
		puts(v, ln);
		len -= ln;
		while (len >= WORDLEN) { puts(v); len -= WORDLEN; }
		puts(v, len);
	}
}

inline void OBitStream::put1(uint16_t len) {
	uint64_t v = ~0ull;
	if (len < WORDLEN - j) puts(v, len);
	else {
		auto ln = WORDLEN - j;
		puts(v, ln);
		len -= ln;
		while (len >= WORDLEN) { puts(v); len -= WORDLEN; }
		puts(v, len);
	}
}

inline void OBitStream::puts(uint64_t v, uint16_t len) {
	assert(len <= WORDLEN);
	assert((j < WORDLEN) && (j != 0 || cur == 0)); // invariant
	if (len == 0) return ;
	v &= (~0ull) >> (WORDLEN - len);
	cur |= (v << j);
	if (j + len >= WORDLEN) {
		os.append(cur);
		uint16_t rem = (WORDLEN - j);
		if (rem == WORDLEN) cur = 0;
		else cur = v >> rem;
		j -= WORDLEN - len;
	} else
		j += len;
	bitlen += len;
}

inline void OBitStream::puts(const std::pair<uint64_t, uint16_t>& code) {
	puts(code.first, code.second);
}

inline void OBitStream::puts(uint64_t v) {
	if (j > 0) {
		cur |= (v << j);
		os.append(cur);
		cur = (v >> (WORDLEN - j));
	} else {
		os.append(v);
		cur = 0;
	}
	bitlen += WORDLEN;
}

inline void OBitStream::puts_c(const char* ptr, size_t byte_count) {
	for (unsigned int i = 0; i < byte_count; ++i)
		this->puts(*(ptr + i), 8);
}

inline void OBitStream::clear() {
	os.clear();
	cur = 0;
	j = 0;
	bitlen = 0;
}

inline void OBitStream::close() {
	if (j > 0) {
		os.append(cur);
		j = 0;
		cur = 0;	
	}
}

inline bool OBitStream::is_accessiable() const { return (j == 0); }


inline void OBitStream::build(BitArray* out) {
	close();
	LocalMemAllocator alloc;
	*out = BitArrayBuilder::adopt(bitlen, alloc.move(os));
}

inline StaticMemRegionPtr OBitStream::build() {
	close();
	LocalMemAllocator alloc;
	return alloc.move(os);
}

inline void OBitStream::pushout() {
	++j;
	++bitlen;
	if (j==WORDLEN) {
		os.append(cur);
		j = 0;
		cur = 0;
	}
}

inline bool OBitStream::getbit(uint64_t pos) const { return (os.getword(pos / WORDLEN) & (1ull << (pos%WORDLEN))) > 0; }

//------------------------------------------------------------------------


inline void IWBitStream::init(const WordAccessInterface* _data, size_t blen, size_t start_idx) {
	data = _data;
	ptr = (start_idx / WORDLEN);
	this->blen = blen - start_idx + (start_idx % WORDLEN);
	if (blen > 0) {
		cur = data->word(ptr);
		++ptr;
	} else { cur = 0; }
	nxt = 0;
	j = 0;
	skipw(start_idx % WORDLEN);
	_init_len = blen;
}

inline void IWBitStream::clear() {
	j = 0;
	cur = nxt = 0;
	blen = 0;
	ptr = 0;
	//_extracted = 0;
	data = nullptr;
}

inline void IWBitStream::skipw(uint16_t len) {
	assert(len <= WORDLEN && blen >= len);
	if (len == 0) return ;
	if (len < WORDLEN) cur = (cur >> len) | (nxt << (WORDLEN - len));
	else cur = nxt;
	
	if (j >= len) {
		nxt >>= len;
		j -= len;
	} else {
		//fetch next word
		if (blen > WORDLEN + j) {
			nxt = data->word(ptr);
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
	//_extracted += len;
}

inline void IWBitStream::skipw_other(uint16_t len) {
	assert(len <= WORDLEN && blen >= len);
	if (len == 0) return;
	blen -= j;
	if (len < WORDLEN) cur = (cur >> len);
	else cur = 0;
	if (len > j) {
		cur |= nxt >> (len - j);
		len -= j;
		nxt = data->word(ptr);
		++ptr;
		j = WORDLEN;
	}
	assert(len <= j);
	cur |= nxt << (j - len);
	j -= len;
	//_extracted += len;
}


inline unsigned int IWBitStream::scan_next1() {
	unsigned int c = 0;
	while (j > 0) {
		if (!getb()) c++;
		else return c;
	}
	assert(j == 0);
	while (cur == 0 && blen > WORDLEN) {
		c += WORDLEN;
		skipw(WORDLEN);
	}
	while (!getb() && !empty()) c++;
	return c;
}

inline unsigned int IWBitStream::scan_next0() {
	unsigned int c = 0;
	while (getb() && !empty()) c++;
	return c;
}

inline bool IWBitStream::getb() {
	bool b = (cur & 1) > 0;
	skipw(1);
	return b;
}

inline uint64_t IWBitStream::get() {
	uint64_t v = cur;
	skipw(64);
	return v;
}

inline uint64_t IWBitStream::get(uint16_t len) {
	if (len == 0) return 0;
	uint64_t v = cur & ((~0ull) >> (WORDLEN - len));
	skipw(len);
	return v;
}

}//namespace
