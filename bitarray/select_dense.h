#pragma once

#include <stdint.h>
#include <vector>
#include <algorithm>
#include "bitarray.h"
#include "bitstream.h"
#include "bitop.h"

// suitable for bitarray with high density of 1-bit (e.g. 100% -- 25% density)

namespace mscds {

class SelectDenseAux;
class SelectDense;
class Select0Dense;

class SelectDenseBuilder {
public:
	static void build_aux(const BitArray& b, SelectDenseAux * o);
	static void build0_aux(const BitArray& b, SelectDenseAux * o);

	static void build(const BitArray& b, SelectDense * o);
	static void build0(const BitArray& b, Select0Dense * o);
};

class SelectDenseAux {
public:
	std::pair<uint64_t, uint32_t> pre_select(uint64_t r) const;
	void load_aux(InpArchive& ar, BitArray& b);
	void save_aux(OutArchive& ar) const;
	void clear();

	size_t cnt, len;
private:
	BitArray ptrs, overflow;
	friend class SelectDenseBuilder;
};

class SelectDense  {
public:
	/** the position of the (r+1)-th 1-value (from left to right) */
	uint64_t select(uint64_t r) const {
		auto p = aux.pre_select(r);
		return p.first + bits.scan_bits(p.first, p.second);
	}
	const BitArray& getBitArray() const { return bits; }
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
public:
	SelectDenseAux aux;
	BitArray bits;
	friend class SelectDenseBuilder;
};

class Select0Dense {
public:
	uint64_t select0(uint64_t r) const {
		auto p = aux.pre_select(r);
		return p.first + bits.scan_zeros(p.first, p.second);
	}
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
public:
	SelectDenseAux aux;
	BitArray bits;
	friend class SelectDenseBuilder;
};

//------------------------------------------------------------
//for internal use only
struct Block {
	static const unsigned int BLK_COUNT = 512;
	struct Header {
		uint64_t v1;
		uint64_t v2;
	};

	Header h;

	void build(const std::vector<unsigned int>& _inp, 
		OBitStream& overflow, uint64_t start_val, unsigned int start_flow);

	uint64_t blk_ptr() const;

	unsigned int get_span() const;
	unsigned int get_sublen() const;
	unsigned int get_casex(unsigned int pos,
		const BitArray& v, unsigned int start) const;

	// private functions
	static const uint64_t SUBMASK = (1ull << 11) - 1;
	static const uint64_t MASK_1 = 0x00FFFFFFFFFFFFFFull;

	unsigned int _get_case0(unsigned int pos) const;
	void _write_case(unsigned int z, unsigned int w, const std::vector<unsigned int>& vals,
		OBitStream& overflow, unsigned int start_flow) ;
	// the number of bits to store number p
	static unsigned int _numbit(unsigned int p);
	static bool _check_span(unsigned int span, const std::vector<unsigned int>& inp,
		const unsigned int max_diff);
	static std::vector<unsigned int> _make_span(unsigned int span,
		const std::vector<unsigned int>& inp, unsigned int& width);
};


}//namespace
