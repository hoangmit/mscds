#pragma once

#ifndef __BITVECTOR_H_
#define __BITVECTOR_H_

/** 
\file
Implement BitArray and related classes based on memory framework.

  BitArray: represents a array of bits. The size is fixed at creation; however, 
    the values of the bit-element can be changed. This class also provide various
	ways to access the bit e.g. by word, by byte.

  BitArrayBuilder: set of functions to build BitArray.

  FixedWArray: Array of fixed width integers (based on BitArray).

  bsearch_hints: binary search

Implemented by Hoang
*/

#include "bitop.h"
#include "../framework/archive.h"
#include "../mem/local_mem.h"
#include "bitarray_generic.hpp"

#include <stdint.h>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <memory>
#include <deque>


namespace mscds {

class BitArray;

class BitArrayBuilder {
public:
	static BitArray create(size_t bitlen);
	static BitArray create(size_t bitlen, const char * ptr);
	static BitArray adopt(size_t bitlen, StaticMemRegionPtr p);
};

struct MemRegionWordAccess: public WordAccessInterface {
	MemRegionWordAccess() = default;
	MemRegionWordAccess(const MemRegionWordAccess& other) = default;
	MemRegionWordAccess(MemRegionWordAccess&& other): _data(std::move(other._data)) {};
	MemRegionWordAccess(const StaticMemRegionPtr& o): _data(o) {}
	MemRegionWordAccess(StaticMemRegionPtr&& o): _data(std::move(o)) {}
	MemRegionWordAccess& operator=(const MemRegionWordAccess& other) = default;
	MemRegionWordAccess& operator=(MemRegionWordAccess&& other) { _data = other._data; return *this; }

	MemRegionWordAccess& operator=(const StaticMemRegionPtr& other) { _data = other; return *this; }

	void load(InpArchive& ar) { _data = ar.load_mem_region(); }
	void save(OutArchive& ar) const { ar.save_mem(_data); }

	uint64_t word(size_t i) const { return _data.getword(i); }
	void setword(size_t i, uint64_t v) { _data.setword(i, v); }
	uint8_t byte(size_t i) const {
		/*uint64_t _word = this->getword(i / 8);
		return (uint8_t)((_word >> (8*(i % 8))) & 0xFF);*/
		return _data.getchar(i);
	}

	uint8_t popcntw(size_t i) const { return popcnt(word(i)); }
	StaticMemRegionPtr _data;
};


class BitArray: public BitArrayGeneric<MemRegionWordAccess> {
public:
	BitArray();
	BitArray(size_t bitlen);
    BitArray(const MemRegionWordAccess& mem, size_t bitlen);
    BitArray(const BitArray& other);
    BitArray& operator=(const BitArray& other);
	BitArray(BitArray&& mE) { _bitlen = mE._bitlen; _data = std::move(mE._data); }
	BitArray& operator=(BitArray&& mE) { _bitlen = mE._bitlen; _data = std::move(mE._data); return *this; }

	/** load the BitArray from InpArchive */
    void load(InpArchive& ar);
    void load_nocls(InpArchive& ar);

    /** save the BitArray to OutArchive */
    void save(OutArchive& ar) const;
    void save_nocls(OutArchive& ar) const;

	StaticMemRegionPtr data_ptr() const { return _data._data; }
	friend class BitArrayBuilder;
};


//------------------------------------------------------------------------
class FixedWArrayBuilder;

class FixedWArray {
public:
	FixedWArray(): width(0) {}
	FixedWArray(const FixedWArray& other): b(other.b), width(other.width) {}
	FixedWArray(const BitArray& bits, unsigned int width_): b(bits), width(width_) {}

	uint64_t operator[](size_t i) const { return b.bits(i*width, width); }
	void set(size_t i, uint64_t v) { b.setbits(i*width, v, width); }

	void fillzero() { b.fillzero(); }
	void clear() { b.clear(); width = 0; }
	InpArchive& load(InpArchive& ar);
	OutArchive& save(OutArchive& ar) const;
	size_t length() const { return b.length() / width; }
	unsigned int getWidth() const { return width; }
	const BitArray getArray() const { return b; }
	std::string to_str() const;
private:
	friend class FixedWArrayBuilder;
	BitArray b;
	unsigned int width;
};

class FixedWArrayBuilder {
public:
	template<typename Itr>
	static void build_s(Itr bg, Itr ed, FixedWArray* out) {
		unsigned width = 0;
		uint64_t len = 0;
		for (Itr it = bg; it != ed; ++it) {
			uint64_t v = *it;
			unsigned l = val_bit_len(v);
			if (width < l) width = l;
			++len;
		}
		out->b = BitArrayBuilder::create(len*width);
		out->width = width;
		uint64_t i = 0;
		uint64_t mask = (~0ull) >> (64-width);
		for (Itr it = bg; it != ed; ++it) {
			out->set(i, (*it) & mask);
			++i;
		}
	}
	static void build_s(const std::vector<unsigned int>& values, FixedWArray* out);

	static FixedWArray create(size_t len, unsigned int width) {
		return FixedWArray(BitArrayBuilder::create(len*width), width);
	}

	void add(uint64_t v) { vals.push_back(v);}
	void build(FixedWArray* out) {
		build_s(vals.begin(), vals.end(), out);
		vals.clear();
	}
	void clear() {
		vals.clear();
	}
private:
	std::deque<uint64_t> vals;
};


template<typename Iterator>
FixedWArray bsearch_hints(Iterator start, size_t arrlen, size_t rangelen, unsigned int lrate) {
	FixedWArray hints = FixedWArrayBuilder::create((rangelen >> lrate) + 2, ceillog2(arrlen + 1));
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
