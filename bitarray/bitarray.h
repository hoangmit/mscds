#pragma once

#ifndef __BITVECTOR_H_
#define __BITVECTOR_H_


#include "bitop.h"
#include "framework/archive.h"

#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <memory>
#include <vector>

namespace mscds {

class BitArraySeqBuilder {
public:
	BitArraySeqBuilder(size_t wordlen, OArchive& _ar);
	void addword(uint64_t v);
	void done();
private:
	OArchive & ar;
	size_t pos, wl;
};

class BitArray {
public:
	const static unsigned int WORDLEN = 64;

	void setbits(size_t bitindex, uint64_t value, unsigned int len);
	void setbit(size_t bitindex, bool value);

	uint64_t bits(size_t bitindex, unsigned int len) const;
	bool bit(size_t bitindex) const;
	bool operator[](size_t i) const { return bit(i); }
	uint8_t byte(size_t pos) const;

	uint64_t count_one() const;
	void fillzero();
	void fillone();

	uint64_t& word(size_t pos);
	const uint64_t& word(size_t pos) const;
	size_t length() const { return bitlen; }
	size_t word_count() const;

	//--------------------------------------------------
	BitArray();
	BitArray(size_t bit_len);
	BitArray(const BitArray& other);
	BitArray& operator=(const BitArray& other);
	BitArray(SharedPtr p, size_t bit_len);
	BitArray(uint64_t * ptr, size_t bit_len);

	const uint64_t* data_ptr() const;

	static BitArray create(size_t bitlen);
	static BitArray create(uint64_t * ptr, size_t bitlen);
	static BitArray create(const char * ptr, size_t bitlen);

	BitArray clone_mem() const;

	void clear();
	~BitArray();

	IArchive& load(IArchive& ar);
	OArchive& save(OArchive& ar) const;
	OArchive& save_nocls(OArchive& ar) const;
	IArchive& load_nocls(IArchive& ar);
	std::string to_str() const;

private:
	inline static uint64_t ceildiv(uint64_t a, uint64_t b) {
		/* return (a != 0 ? ((a - 1) / b) + 1 : 0); */
		return (a + b - 1) / b;
	}

	size_t bitlen;
	uint64_t * data;
	SharedPtr ptr;
};

//---------------------------------------------------------------------


inline BitArray::BitArray(): bitlen(0), data(NULL) {}

inline BitArray::BitArray(size_t bit_len): bitlen(0) {
	*this = create(bit_len);
}

inline BitArray::BitArray(const BitArray& other) {
	data = other.data;
	bitlen = other.bitlen;
	ptr = other.ptr;
}

inline BitArray& BitArray::operator=(const BitArray& other) {
	data = other.data;
	bitlen = other.bitlen;
	ptr = other.ptr;
	return *this;
}

inline BitArray::BitArray(SharedPtr p, size_t bit_len): ptr(p), bitlen(bit_len) {
	data = (uint64_t*) ptr.get();
}

inline BitArray::BitArray(uint64_t * ptr, size_t bit_len): bitlen(bit_len), data(ptr) {
	this->ptr.reset();
}

inline void BitArray::clear() {
	if (bitlen > 0) ptr.reset();
	bitlen = 0;
	data = NULL;
}

inline uint64_t& BitArray::word(size_t pos) { assert(pos < word_count()); return data[pos]; }
inline const uint64_t& BitArray::word(size_t pos) const { assert(pos < word_count()); return data[pos]; }
inline size_t BitArray::word_count() const { return ceildiv(bitlen, WORDLEN); }

inline const uint64_t* BitArray::data_ptr() const { return data; }

template<typename T>
struct CppArrDeleter {
	void operator()(void* p) const { delete[]((T*)p); }
};

inline SharedPtr createUI64Arr(size_t len) {
	return SharedPtr(new uint64_t[len], CppArrDeleter<uint64_t>());
}

inline BitArray BitArray::create(size_t bitlen) {
	BitArray v;
	if (bitlen == 0) return v;
	assert(bitlen > 0);
	size_t arrlen = (size_t)ceildiv(bitlen, WORDLEN);
	v.ptr = createUI64Arr(arrlen);
	v.data = (uint64_t*)v.ptr.get();
	v.bitlen = bitlen;
	if (arrlen > 0) v.data[arrlen - 1] = 0;
	return v;
}

inline BitArray BitArray::create(const char * ptr, size_t bitlen) {
	BitArray v = create(bitlen);
	size_t bytelen = (size_t)ceildiv(bitlen, 8);
	memcpy(v.data, ptr, bytelen);
	return v;
}

inline BitArray BitArray::create(uint64_t * ptr, size_t bitlen) {
	BitArray v = create(bitlen);
	size_t arrlen = (size_t)ceildiv(bitlen, WORDLEN);
	std::copy(ptr, ptr + arrlen, v.data);
	return v;
}

inline BitArray BitArray::clone_mem() const {
	BitArray v(this->bitlen);
	std::copy(data, data + word_count(), v.data);
	return v;
}

inline BitArray::~BitArray() { clear(); }

inline uint64_t BitArray::bits(size_t bitindex, unsigned int len) const {
	assert(len <= WORDLEN); // len > 0
	assert(bitindex + len <= bitlen);
	if (len==0) return 0;
	uint64_t i = bitindex / WORDLEN;
	unsigned int j = bitindex % WORDLEN;
	//uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull;
	uint64_t mask = ((~0ull) >> (WORDLEN - len));
	if (j + len <= WORDLEN)
		return (data[i] >> j) & mask;
	else
		return (data[i] >> j) | ((data[i + 1] << (WORDLEN - j)) & mask);
}

inline void BitArray::setbits(size_t bitindex, uint64_t value, unsigned int len) {
	assert(len <= WORDLEN && len > 0);
	assert(bitindex + len <= bitlen);
	uint64_t i = bitindex / WORDLEN;
	unsigned int j = bitindex % WORDLEN;
	//uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull; // & (~0ull >> (WORDLEN - len))
	uint64_t mask = ((~0ull) >> (WORDLEN - len));
	value = value & mask;
	data[i] = (data[i] & ~(mask << j)) | (value << j);
	if (j + len > WORDLEN)
		data[i+1] = (data[i+1] & ~ (mask >> (WORDLEN - j))) | (value >> (WORDLEN - j));
}

inline bool BitArray::bit(size_t bitindex) const {
	assert(bitindex < bitlen);
	return (data[bitindex / WORDLEN] & (1ULL << (bitindex % WORDLEN))) != 0;
}

inline void BitArray::setbit(size_t bitindex, bool value) {
	assert(bitindex < bitlen);
	if (value) data[bitindex / WORDLEN] |= (1ULL << (bitindex % WORDLEN));
	else data[bitindex / WORDLEN] &= ~(1ULL << (bitindex % WORDLEN));
}

//------------------------------------------------------------------------
class FixedWArray {
private:
	BitArray b;
	unsigned int width;
public:
	FixedWArray(): width(0) {}
	FixedWArray(const FixedWArray& other): b(other.b), width(other.width) {}
	FixedWArray(const BitArray& bits, unsigned int width_): b(bits), width(width_) {}
	static FixedWArray create(size_t len, unsigned int width) {
		return FixedWArray(BitArray::create(len*width), width);
	}
	uint64_t operator[](size_t i) const { return b.bits(i*width, width); }
	void set(size_t i, uint64_t v) { b.setbits(i*width, v, width); }

	void fillzero() { b.fillzero(); }
	void clear() { b.clear(); width = 0; }
	IArchive& load(IArchive& ar);
	OArchive& save(OArchive& ar) const;
	size_t length() const { return b.length() / width; }
	unsigned int getWidth() const { return width; }
	const BitArray getArray() const { return b; }
	std::string to_str() const;
};


template<typename Iterator>
FixedWArray bsearch_hints(Iterator start, size_t arrlen, size_t rangelen, unsigned int lrate) {
	FixedWArray hints = FixedWArray::create((rangelen >> lrate) + 2, ceillog2(arrlen + 1));
	uint64_t i = 0, j = 0, p = 0;
	do {
		hints.set(i, p);
		//assert(j == (1 << ranklrate)*i);
		//assert(A[p] >= j);
		++i;  j += (1ULL<<lrate);
		while (p < arrlen && *start < j) { ++p; ++start; }
	} while (j < rangelen);
	for (unsigned int k = i; k < hints.length(); k++)
		hints.set(k, arrlen);
	return hints;
}

} //namespace
#endif // __BITVECTOR_H_
