#pragma once

/**

Data structure for Range Minimum/Maximum Query.

This is structure only store the values of the min/max.

*/


#include "bitarray/bitop.h"
#include "bitarray/bitarray.h"
#include "framework/archive.h"

#include <iostream>
#include <stdint.h>
#include <vector>
#include <sstream>

namespace mscds {

//depends on bitarray, msb_intr, ceillog2

class RMQ_table;

class RMQ_table_builder {
public:
	static void build(const std::vector<uint64_t>& lst, RMQ_table * t, bool min_structure = true, bool pack_values = true);
private:
	static void pack(bool pack_values, const std::vector<uint64_t>& lst, std::vector<uint64_t>& nlst, uint64_t& maxval);
};

/// Single level RMQ table (uses O(n^2) space)
class RMQ_table {
public:
	RMQ_table() {}
	size_t m_idx(size_t st, size_t ed) const;
	size_t r_val(size_t idx) const { return rval[idx]; }

	bool is_min() const { return _min_structure; }
	void clear() { len = 0; table.clear(); rval.clear(); }

	std::string to_str() const;
	void save(OutArchive& ar) const;
	void load(InpArchive& ar);

private:
	size_t get_tb(size_t d, size_t i) const;

	FixedWArray table; // idx_bwidth
	FixedWArray rval; // valbw
	//unsigned int val_bwidth, idx_bwidth;

	bool _min_structure;
	uint64_t len, max_val;
	
	friend class RMQ_table_builder;
	size_t nlayers() const { return msb_intr(len) + 1; }
};

}//namespace

