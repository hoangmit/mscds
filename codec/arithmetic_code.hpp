
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
#include <queue>
#include "bitarray/bitop.h"

namespace coder {


/// Output bit stream for arithmetic coder
struct OutBitStream {
	typedef uint16_t ChrTp;
	typedef std::deque<ChrTp> VecTp;
	OutBitStream(VecTp* _v): v(_v), cache(0), mask(1), bitlen(0) {}
	~OutBitStream() {close();}

	void put_bit(bool bit) {
		bitlen++;
		if (bit) cache |= mask;
		if (mask != MAXMASK) mask <<= 1;
		else {
			v->push_back(cache);
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
			v->push_back(cache);
			cache = 0;
			mask = 1;
		}
	}

	void close() {
		if (mask > 1) {
			v->push_back(cache);
			mask = 1;
		}
	}

	VecTp * v;
	ChrTp cache, mask;
	static const ChrTp MAXMASK = 1u << (sizeof(ChrTp) * 8 - 1);
	size_t bitlen;
};


/// Input bit stream for arithmetic coder
struct InBitStream {
	typedef uint16_t ChrTp;
	typedef std::deque<ChrTp> VecTp;

	InBitStream(const VecTp* _v, size_t _bitlen) {
		init(_v, _bitlen);
	}

	InBitStream(const OutBitStream & in) {
		init(in.v, in.bitlen);
	}

	void init(const VecTp* _v, size_t _bitlen) {
		v = _v;
		bitlen = _bitlen;
		cache = 0;
		clen = 0;
		pos = 0;
		i = 0;
	}

	bool is_empty() { return pos >= bitlen; }

	/// return one bit in the stream.
	/// This must return some bit, even overflow
	bool get_bit() {
		if (is_empty()) return false;
		pos++;
		if (clen == 0) {
			cache = (*v)[i++];
			clen = sizeof(ChrTp)*8;
		}
		bool ret = (cache & 1) != 0;
		cache >>= 1;
		clen -= 1;
		return ret;
	}

	static unsigned revbitL(unsigned v, uint8_t len) {
		unsigned ret = 0;
		while (len > 8) {
			ret = (ret << 8) | mscds::revbits_table8(v & 0xFF);
			v = v >> 8;
			len -= 8;
		}
		// does not reserve bits [len+1:]
		if (len > 0) {
			v = mscds::revbits_table8(v & 0xFF) >> (8-len);
			ret = (ret << len) | v;
		}
		return ret;
	}

	unsigned get_bits(uint8_t len) {
		// must have the same resut as:
		//unsigned code = 0;
		//for (unsigned i = 0; i < len; ++i)
		//	code = (code << 1) | (input->get_bit() ? 1u : 0u);
		//return code;
		if (is_empty()) 
			return 0;
		if (len == 1) return get_bit();
		pos += len;
		unsigned ret = 0;
		uint8_t shift = 0;
		while (len>0) {
			if (clen == 0) {
				if (i < v->size()) cache = (*v)[i++];
				else cache = 0;
				clen = sizeof(ChrTp)*8;
			}
			uint8_t dt = std::min<uint8_t>(clen, len);

			ret |= (cache << shift);
			cache >>= dt;
			len -= dt;
			clen -= dt;
			shift += dt;
		}
		ret = revbitL(ret, shift);
		return ret;
	}

	void close() {
	}

	static const ChrTp MAXMASK = 1u << (sizeof(ChrTp) * 8 - 1);
	const VecTp * v;
	ChrTp cache;
	uint8_t clen;
	size_t bitlen;
	size_t pos, i;
};

/// Arithmetic encoder using 32-bit states
class AC32_EncState {
public:
	static const uint32_t MAX_PROB_RESOLUTION = 1ul << (sizeof(uint32_t)*8-2);
public:
	AC32_EncState(): output(NULL) {}
	AC32_EncState(OutBitStream * out) { init(out); }

	void init(OutBitStream * out) {
		lo = 0;
		hi = ~lo;
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
		if (lo & Q1) {
			set_bit_opt<true>(); // alternative
			// ignore the trailing 0
			//output->put_bit(true);
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
			} else if ((lo & Q1) && hi < Q3) {
				++underflow;
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

	size_t underflow;
	uint32_t lo, hi;

	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);;
	static const uint32_t Q1 = 1ul << (sizeof(uint32_t)*8-2);
	static const uint32_t Q3 = Q1 | MSB;
	//const uint32_t Q3  = 0xC0000000ul;
};

/// Arithmetic dencoder using 32-bit states
class AC32_DecState {
public:
	AC32_DecState(): input(NULL) {}
	AC32_DecState(InBitStream * inp) { init(inp); }

	void init(InBitStream * inp) {
		input = inp;
		lo = 0;
		hi = ~lo;
		code = 0;
		for (int i = 0; i < sizeof(uint32_t)*8; ++i) {
			if (!input->is_empty())
				code = (code << 1) | (input->get_bit() ? 1 : 0);
			else code = (code << 1);
		}
	}

	/// returns a number that fall between [lo_count, hi_count) 
	/// where 'lo_count' and 'hi_count' were the parameters of the encode funtion
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
		fast_renomalize();
	}

	void close() {
		input->close();
		input = NULL;
	}
private:
	void renomalize() {
		for (;;) {
			if ((hi < MSB) || (lo & MSB)) {}
			else if ((lo & Q1) && hi < Q3) {
				lo ^= Q1;
				hi |= Q1;
				code ^= Q1;
			} else break;
			lo <<= 1;
			hi = (hi << 1) | 1;
			code = (code << 1) | (input->get_bit() ? 1u : 0u);
		}
	}

	void fast_renomalize() {
		if ((hi < MSB) || (lo & MSB)) {
			uint32_t k = (hi ^ lo);
			uint8_t shift = sizeof(hi)*8 - mscds::msb_intr(k) - 1;
			code = (code << shift) | input->get_bits(shift);
			hi <<= shift;
			hi |= ~((~0u) << shift);
			lo <<= shift;
		}
		if ((lo & Q1) && hi < Q3) {
			//underflow
			uint32_t k = ((hi - lo) >> 1) | (~(hi ^ lo));
			uint8_t shift = sizeof(hi)*8 - mscds::msb_intr(k) - 2;
			code = (code << shift) | input->get_bits(shift);

			code ^= MSB;
			hi <<= shift;
			hi |= ~((~0u) << shift);
			hi |= MSB;
			lo <<= shift;
			lo &= (MSB-1);
		}
	}

private:
	InBitStream * input;
	uint32_t lo, hi;
	uint32_t code;

	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);
	static const uint32_t Q1 = 1ul << (sizeof(uint32_t)*8-2);
	static const uint32_t Q3 = (Q1 | MSB);
};

}
