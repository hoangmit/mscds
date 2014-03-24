#pragma once

#include <stdint.h>
#include <vector>
#include <cassert>
#include <sstream>
#include "framework/archive.h"
#include "mem/local_mem.h"
#include "bitarray.h"


namespace mscds {

class OBitStream {
public:
	OBitStream(): cur(0), j(0), bitlen(0) {}
	~OBitStream() { assert(j == 0); }

	void put0();
	void put1();
	void put0(uint16_t len);
	void put1(uint16_t len);
	void put(bool bit);
	void puts(uint64_t v, uint16_t len);
	void puts(const std::pair<uint64_t, uint16_t>& code);
	void puts(uint64_t v);
	void puts_c(const char* ptr, size_t byte_count);

	void clear();
	void close();

	bool is_accessiable() const;

	void append(OBitStream& other);

	std::string to_str() const;

	size_t length() const { return bitlen; }
	size_t word_count() const { return os.size(); }

	void build(BitArray* out);
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

class IWBitStream {
public:
	const static uint16_t WORDLEN = 64;
	IWBitStream(){ clear(); }
	IWBitStream(const OBitStream& os) {
		LocalMemModel alloc;
		init(alloc.convert(os.os), os.length(), 0);
	}

	IWBitStream(const BitArray& b) {
		init(b.data_ptr(), b.length(), 0);
	}

	// important
	void init(StaticMemRegionPtr _data, size_t blen, size_t start_idx = 0);

	void init(const BitArray& b, size_t start_idx = 0) {
		init(b.data_ptr(), b.length(), start_idx);
	}

	void clear();

	// important
	void skipw(uint16_t len);
	bool getb();

	/** return the number of 0 bits before the next 1 bit or end of stream;
	    consume all the 0 bits and the last 1 bit */
	unsigned int scan_next1();

	/** return the number of 0 bits before the next 0 bit */
	unsigned int scan_next0();

	uint64_t get(uint16_t len);
	uint64_t get();
	uint64_t peek() const { return cur; }

	//const uint64_t* current_ptr() { return ptr; }
	bool empty() const { return blen == 0; }
	size_t extracted() const { return _extracted; }
	void close() { clear(); }
private:
	uint64_t cur, nxt;
	size_t blen, _extracted;
	uint16_t j;
	size_t ptr;
	StaticMemRegionPtr data;
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
	if (len == 0) return ;
	v &= (~0ull) >> (WORDLEN - len);
	cur |= (v << j);
	if (j + len >= WORDLEN) {
		os.append(cur);
		cur = (v >> (WORDLEN - j));
		j -= WORDLEN - len;
	} else j += len;
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

inline void OBitStream::append(OBitStream &other) {
	if (!other.is_accessiable()) throw std::runtime_error("cannot extract");
	size_t px = 0, i = 0;
	while (px + WORDLEN < other.length()) {
		puts(other.os.getword(i));
		i++;
		px += WORDLEN;
	}
	if (px < other.length()) {
		uint64_t v = other.os.getword(i);
		puts(v, other.length() - px);
	}
}

inline void OBitStream::build(BitArray* out) {
	close();
	LocalMemModel alloc;
	*out = BitArrayBuilder::adopt(bitlen, alloc.convert(os));
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
		os.append(cur);
		j = 0;
		cur = 0;
	}
}

inline bool OBitStream::getbit(uint64_t pos) const { return (os.getword(pos / WORDLEN) & (1ull << (pos%WORDLEN))) > 0; }

//------------------------------------------------------------------------


inline void IWBitStream::init(StaticMemRegionPtr _data, size_t blen, size_t start_idx) {
	data = _data;
	ptr = (start_idx / WORDLEN);
	this->blen = blen - start_idx + (start_idx % WORDLEN);
	if (blen > 0) {
		cur = data.getword(ptr);
		++ptr;
	}
	nxt = 0;
	j = 0;
	skipw(start_idx % WORDLEN);
	_extracted = 0;
}

inline void IWBitStream::clear() {
	j = 0;
	cur = nxt = 0;
	blen = 0;
	ptr = 0;
	_extracted = 0;
	data.close();
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
			nxt = data.getword(ptr);
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
	_extracted += len;
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

inline uint64_t IWBitStream::get(uint16_t len) {
	if (len == 0) return 0;
	uint64_t v = cur & ((~0ull) >> (WORDLEN - len));
	skipw(len);
	return v;
}


//----------------------------------------------------------------

class OByteStream {
public:
	OByteStream() {}
	void put(char c) { os.append(c); }
	void puts(const std::string& str) {
		for (size_t i = 0; i < str.length(); ++i) os.append(str[i]);
	}
	void puts(const char* str, unsigned int len) {
		for (size_t i = 0; i < len; ++i) os.append(*(str + i));
	}
	size_t length() const { return os.size(); }
	void build(BitArray* out) {
		LocalMemModel alloc;
		*out = BitArrayBuilder::adopt(os.size() * 8, alloc.convert(os));
	}
	void clear() { os.clear(); }
private:
	LocalDynamicMem os;
};

}//namespace
