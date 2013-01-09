
#pragma once

#include <iostream>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <stdexcept>
#include <cstdlib>
#include <limits>

namespace coder {

struct OutBitStream {
	typedef unsigned char ChrTp;
	typedef std::vector<ChrTp> VecTp;
	OutBitStream(VecTp& _v): v(_v), cache(0), mask(1), bitlen(0) {}
	~OutBitStream() {close();}

	void put_bit(bool bit) {
		bitlen++;
		if (bit) cache |= mask;
		if (mask != 0x80) mask <<= 1;
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
	size_t bitlen;
};

struct InBitStream {
	typedef unsigned char ChrTp;
	typedef std::vector<ChrTp> VecTp;

	InBitStream(const VecTp& _v, size_t _bitlen): v(_v), bitlen(_bitlen), cache(0), mask(1), pos(0) {
		if (v.size() > 0) cache = v[0]; 
		i = 0;
	}
	InBitStream(const OutBitStream &in): v(in.v), bitlen(in.bitlen), cache(0), mask(1), pos(0)  {
		if (v.size() > 0) cache = v[0];
		i = 0;
	}

	bool is_empty() {
		return pos == bitlen;
	}

	bool get_bit() {
		if (is_empty()) return false;
		pos++;
		bool ret = ((cache & mask) != 0);
		if (mask != 0x80) mask <<= 1;
		else {
			cache = v[++i];
			mask = 1;
		}
		return ret;
	}

	void close() {
	}

	const VecTp &v;
	ChrTp cache, mask;
	size_t bitlen;
	size_t pos, i;
};


struct AC32_EncState {
	OutBitStream * output;

	uint32_t lo, hi;
	size_t underflow;
	
	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);;
	static const uint32_t Q1  = 1ul << (sizeof(uint32_t)*8-2);
	//const uint32_t Q3  = 0xC0000000ul;
	static const uint32_t MAX_PROB_RESOLUTION = 1ul << (sizeof(uint32_t)*8-1);

	void init() {
		lo = 0;
		hi = std::numeric_limits<uint32_t>::max();;
		underflow = 0;
	}

	void update(uint32_t lo_count, uint32_t hi_count, uint32_t total) {
		assert(total < MAX_PROB_RESOLUTION);
		uint64_t rlen = (uint64_t) hi - lo + 1;
		hi = lo + ((rlen * hi_count / total) - 1);
		lo = lo + (rlen * lo_count / total);
		renormalize();
	}

	void renormalize() {
		for (;;) {
			if (hi < MSB) {
				set_bit(false);
			} else if (lo & MSB) { // lo >= MSB
				set_bit(true);
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

	void set_bit(bool bit) {
		output->put_bit(bit);
		while (underflow > 0) {
			output->put_bit(!bit);
			--underflow;
		}
	}

	void close() {
		underflow++;
		//set_bit(lo & Q1);
		if (lo & Q1) output->put_bit(true);// ignore the trailing 0
		else set_bit(false);
		output->close();
		underflow = 0;
	}
};


struct AC32_DecState {
	InBitStream * input;
	uint32_t lo, hi;
	uint32_t code;
	
	static const uint32_t MSB = 1ul << (sizeof(uint32_t)*8-1);
	static const uint32_t Q1  = 1ul << (sizeof(uint32_t)*8-2);
	
	void init() {
		lo = 0;
		hi = std::numeric_limits<uint32_t>::max();
		code = 0;
		for (int i = 0; i < sizeof(uint32_t)*8; ++i) {
			if (!input->is_empty())
				code = (code << 1) | (input->get_bit() ? 1 : 0);
			else code = (code << 1);
		}
	}

	unsigned int decode_count(uint32_t total) {
		uint64_t rlen = (uint64_t) hi - lo + 1;
		uint64_t v = (uint64_t(code) - lo + 1) * total - 1;
		uint32_t ret =  (uint32_t) (v / rlen);
		return ret;
	}

	void update(uint32_t lo_count, uint32_t hi_count, uint32_t total) {
		uint64_t rlen = (uint64_t) hi - lo + 1;
		hi = lo + ((rlen * hi_count / total) - 1);
		lo = lo + (rlen * lo_count / total);
		renomalize();
	}

	void renomalize() {
		for (;;) {
			if (hi < MSB || (lo & MSB)) {}
			else if ((lo & Q1) && hi < (MSB | Q1)) {
				lo ^= Q1;
				hi |= Q1;
				code ^= Q1;
			}else break;
			lo <<= 1;
			hi = (hi << 1) | 1;
			code = (code << 1) | (input->get_bit() ? 1ul : 0ul);
		}
	}

	void close() {
		input->close();
	}
};

}
