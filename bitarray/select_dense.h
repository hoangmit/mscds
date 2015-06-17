#pragma once

#include <stdint.h>
#include <vector>
#include <algorithm>
#include "bitarray.h"
#include "bitstream.h"
#include "bitop.h"

/** \file: suitable for bitarray with high density of 1-bit (e.g. 100% -- 25% density) */

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

/// Auxiliary data structure for select for dense input (100% to 25% 1-bit)
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

/// data structure to select 1-bit
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

/// data structure to select 0-bit
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


}//namespace
