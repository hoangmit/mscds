#pragma once

#ifndef __BITVECTOR_H_
#define __BITVECTOR_H_




#include "bitop.h"
#include "archive.h"

#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <memory>
#include <algorithm>


namespace mscds {

class BitArraySeqBuilder {
	OArchive & ar;
	size_t pos, wl;
public:
	BitArraySeqBuilder(size_t wordlen, OArchive& _ar): ar(_ar), pos(0) {
		ar.startclass("Bitvector", 1);
		ar.var("bit_len").save(wordlen * 64);
		wl = wordlen;
	}

	void addword(uint64_t v) {
		ar.save_bin(&v, sizeof(v));
		pos++;
	}

	void done() {
		ar.endclass();
		assert(wl == pos);
	}
};

class BitArray {
public:
	const static unsigned int WORDLEN = 64;

	uint64_t bits(size_t bitindex, unsigned int len) const {
		assert(len <= WORDLEN && len > 0);
		assert(bitindex + len <= bitlen);
		uint64_t i = bitindex / WORDLEN;
		unsigned int j = bitindex % WORDLEN; 
		//uint64_t mask = ((1ull << (len % WORDLEN)) - 1); // (~0ull >> (WORDLEN - len));
		uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull;
		if (j + len <= WORDLEN) 
			return (data[i] >> j) & mask;
		else
			return (data[i] >> j) | ((data[i + 1] << (WORDLEN - j)) & mask);
	}

	void setbits(uint64_t bitindex, uint64_t value, unsigned int len) {
		assert(len <= WORDLEN && len > 0);
		assert(bitindex + len <= bitlen);
		uint64_t i = bitindex / WORDLEN;
		unsigned int j = bitindex % WORDLEN; 
		//uint64_t mask = ((1ull << (len % WORDLEN)) - 1);
		uint64_t mask = (len < WORDLEN) ? ((1ull << len) - 1) : ~0ull; // & (~0ull >> (WORDLEN - len))
		value = value & mask;
		if (j + len <= WORDLEN) {
			data[i] = (data[i] & ~(mask << j)) | (value << j);
		} else {
			data[i] = (data[i] & ~(mask << j)) | (value << j);
			data[i+1] = data[i+1] & ~ (mask >> (WORDLEN - j)) &  value >> (WORDLEN - j);
		}
	}

	bool bit(size_t bitindex) const {
		assert(bitindex < bitlen);
		return (data[bitindex / WORDLEN] & (1ULL << (bitindex % WORDLEN))) != 0;
	}

	void setbit(size_t bitindex, bool value) {
		assert(bitindex < bitlen);
		if (value) data[bitindex / WORDLEN] |= (1ULL << (bitindex % WORDLEN));
		else data[bitindex / WORDLEN] &= ~(1ULL << (bitindex % WORDLEN));
	}
	
	uint64_t& word(size_t pos) { return data[pos]; }
	const uint64_t& word(size_t pos) const { return data[pos]; }
	size_t length() const { return bitlen; }
	size_t word_count() const { return ceildiv(bitlen, WORDLEN); }

	void fillzero() {
		std::fill(data, data + word_count(), 0ull);
	}

	void fillone() {
		std::fill(data, data + word_count(), ~0ull);
	}

//--------------------------------------------------
	BitArray(): bitlen(0), data(NULL) {}

	BitArray(size_t bit_len): bitlen(0) {
		*this = create(bit_len);
	}

	BitArray(const BitArray& other) {
		data = other.data;
		bitlen = other.bitlen;
		ptr = other.ptr;
	}

	BitArray(SharedPtr p, size_t bit_len): ptr(p), bitlen(bit_len) {
		data = (uint64_t*) ptr.get();
	}

	BitArray(uint64_t * ptr, size_t bit_len): bitlen(bit_len), data(ptr) {
		this->ptr.reset();
	}

	void clear() {
		bitlen = 0;
		data = NULL;
		ptr.reset();
	}

	static BitArray create(size_t bitlen) {
		BitArray v;
		assert(bitlen > 0);
		size_t arrlen = (size_t) ceildiv(bitlen, WORDLEN);
		v.data = new uint64_t[arrlen];
		v.ptr = SharedPtr(v.data);
		v.bitlen = bitlen;
		return v;
	}

	static BitArray create(uint64_t * ptr, size_t bitlen) {
		BitArray v = create(bitlen);
		size_t arrlen = (size_t) ceildiv(bitlen, WORDLEN);
		std::copy(ptr, ptr + arrlen, v.data);
		return v;
	}

	BitArray clone_mem() const {
		BitArray v(this->bitlen);
		std::copy(data, data + word_count(), v.data);
		return v;
	}

	~BitArray() { clear(); }

	inline static uint64_t ceildiv(uint64_t a, uint64_t b) {
		return (a != 0 ? ((a - 1) / b) + 1 : 0);
	}
	
	IArchive& load(IArchive& ar) {
		ar.loadclass("Bitvector");
		ar.var("bit_len").load(bitlen);
		ptr = ar.var("bits").load_mem(0, sizeof(uint64_t) * word_count());
		data = (uint64_t*) ptr.get();
		ar.endclass();
		return ar;
	}

	OArchive& save(OArchive& ar) const {
		ar.startclass("Bitvector", 1);
		ar.var("bit_len").save(bitlen);
		ar.var("bits").save_bin(data, sizeof(uint64_t) * word_count());
		ar.endclass();
		return ar;
	}

	std::string to_str() const {
		std::string s;
		for (unsigned int i = 0; i < bitlen; ++i)
			if (bit(i)) s += '1';
			else s += '0';
		return s;
	}

private:
	size_t bitlen;
	uint64_t * data;
private:
	SharedPtr ptr;
};

} //namespace
#endif // __BITVECTOR_H_
