
#pragma once

/**
\file
Prototype implementation of Arithmetic code

Written by Hoang

*/

#include <iostream>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <limits>

namespace coder {


/// Output bit stream for arithmetic coder
struct OutBitStream {
	typedef unsigned char ChrTp;
	typedef std::vector<ChrTp> VecTp;
	OutBitStream(VecTp& _v): v(_v), cache(0), mask(1), bitlen(0) {}
	~OutBitStream() {close();}

	void put_bit(bool bit) {
		bitlen++;
		if (bit) cache |= mask;
		if (mask != MAXMASK) mask <<= 1;
		else {
			v.push_back(cache);
			cache = 0;
			mask = 1;
		}
	}

	template<bool bit>
	void put_bit_opt() {
		bitlen++;
		if (bit) cache |= mask;
		if (mask != MAXMASK) mask <<= 1;
		else {
			v.push_back(cache);
			cache = 0;
			mask = 1;
		}
	}

	void close() {
		if (mask > 1) {
			v.push_back(cache);
			mask = 1;
		}
	}

	VecTp & v;
	ChrTp cache, mask;
	static const ChrTp MAXMASK = 1u << (sizeof(ChrTp) * 8 - 1);
	size_t bitlen;
};


/// Input bit stream for arithmetic coder
struct InBitStream {
	typedef unsigned char ChrTp;
	typedef std::vector<ChrTp> VecTp;

	InBitStream(const VecTp& _v, size_t _bitlen): v(_v), bitlen(_bitlen), cache(0),
			mask(1), pos(0), i(0) {
		if (v.size() > 0) cache = v[0];
	}
	InBitStream(const OutBitStream &in): v(in.v), bitlen(in.bitlen), cache(0),
			mask(1), pos(0), i(0) {
		if (v.size() > 0) cache = v[0];
	}

	bool is_empty() { return pos == bitlen; }

	/// return one bit in the stream.
	/// This must return some bit, even overflow
	bool get_bit() {
		if (is_empty())
			return false;
		pos++;
		bool ret = ((cache & mask) != 0);
		if (mask != MAXMASK) mask <<= 1;
		else {
			if (i < v.size() - 1) cache = v[++i];
			else cache = 0;
			mask = 1;
		}
		return ret;
	}

	void close() {
	}

	static const ChrTp MAXMASK = 1u << (sizeof(ChrTp) * 8 - 1);
	const VecTp &v;
	ChrTp cache, mask;
	size_t bitlen;
	size_t pos, i;
};

/// Arithmetic encoder using 32-bit states
class AC32_EncState {
public:
	static const uint32_t MAX_PROB_RESOLUTION = 1ul << (sizeof(uint32_t)*8-1);
public:
	AC32_EncState(): output(NULL) {}

	void init(OutBitStream * out) {
		lo = 0;
		hi = std::numeric_limits<uint32_t>::max();;
		underflow = 0;
		output = out;
	}

	/// updates the state with the bounds of the symbol.
	/// Note that, the range is half open e.g. [lo_count, hi_count)
	void update(uint32_t lo_count, uint32_t hi_count, uint32_t total) {
		assert(total < MAX_PROB_RESOLUTION);
		uint64_t rlen = (uint64_t) hi - lo + 1;
		hi = lo + ((rlen * hi_count / total) - 1);
		lo = lo + (rlen * lo_count / total);
		renormalize();
	}

	void close() {
		underflow++;
		//set_bit(lo & Q1);
		if (lo & Q1) {
			//set_bit_opt<true>(); // alternative
			// ignore the trailing 0
			output->put_bit(true);
		} else set_bit_opt<false>();
		output->close();
		init(NULL);
	}

private:
	void renormalize() {
		for (;;) {
			if (hi < MSB) {
				set_bit_opt<false>();
			} else if (lo & MSB) { // lo >= MSB
				set_bit_opt<true>();
				/* //  the shifts at the end of this loop will erase the MSB
				lo ^= MSB; 
				hi ^= MSB; */
			} else if ((lo & Q1) && hi < (MSB | Q1)) {
				++underflow;
				//cout << " u ";
				lo ^= Q1;
				hi |= Q1; /* hi ^= MSB; */
			} else break;
			lo <<= 1;
			hi = (hi << 1) | 1;
		}
	}

	template<bool bit>
	void set_bit_opt() {
		output->put_bit_opt<bit>();
		while (underflow > 0) {
			output->put_bit_opt<!bit>();
			--underflow;
		}
	}

	void set_bit(bool bit) {
		output->put_bit(bit);
		while (underflow > 0) {
			output->put_bit(!bit);
			--underflow;
		}
	}

private:
	OutBitStream * output;

	uint32_t lo, hi;
	uint32_t underflow;

	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);;
	static const uint32_t Q1 = 1ul << (sizeof(uint32_t)*8-2);
	//const uint32_t Q3  = 0xC0000000ul;

};

/// Arithmetic dencoder using 32-bit states
class AC32_DecState {
public:
	AC32_DecState(): input(NULL) {}

	void init(InBitStream * inp) {
		input = inp;
		lo = 0;
		hi = std::numeric_limits<uint32_t>::max();
		code = 0;
		for (int i = 0; i < sizeof(uint32_t)*8; ++i) {
			if (!input->is_empty())
				code = (code << 1) | (input->get_bit() ? 1 : 0);
			else code = (code << 1);
		}
	}

	/// returns a number that fall between [lo_count, hi_count) 
	/// where 'lo_count' and 'hi_count' were parameter of the encode funtion
	unsigned int decode_count(uint32_t total) {
		uint64_t rlen = (uint64_t) hi - lo + 1;
		uint64_t v = (uint64_t(code) - lo + 1) * total - 1;
		uint32_t ret = (uint32_t) (v / rlen);
		return ret;
	}

	void update(uint32_t lo_count, uint32_t hi_count, uint32_t total) {
		uint64_t rlen = (uint64_t) hi - lo + 1;
		hi = lo + ((rlen * hi_count / total) - 1);
		lo = lo + (rlen * lo_count / total);
		renomalize();
	}

	void close() {
		input->close();
		input = NULL;
	}
private:
	void renomalize() {
		do {
			if ((hi < MSB) || (lo & MSB)) {}
			else if ((lo & Q1) && hi < (MSB | Q1)) {
				lo ^= Q1;
				hi |= Q1;
				code ^= Q1;
			} else break;
			lo <<= 1;
			hi = (hi << 1) | 1;
			code = (code << 1) | (input->get_bit() ? 1u : 0u);
		} while (true);
	}

private:
	InBitStream * input;
	uint32_t lo, hi;
	uint32_t code;

	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);
	static const uint32_t Q1 = 1ul << (sizeof(uint32_t)*8-2);

};

}
