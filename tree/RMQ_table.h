#pragma once

#include "bitarray/bitop.h"
#include "bitarray/bitarray.h"
#include "archive.h"

#include <iostream>
#include <stdint.h>
#include <vector>
#include <sstream>

namespace mscds {

//depends on bitarray, msb_intr, ceillog2

class RMQ_table;

class RMQ_table_builder {
public:
	void build(const std::vector<uint64_t>& lst, RMQ_table * t,  bool min_structre = true, bool pack_values = true);
private:
	void pack(bool pack_values, const std::vector<uint64_t>& lst, std::vector<uint64_t>& nlst, uint64_t& maxval);
};

class RMQ_table {
private:
	uint64_t get_tb(uint64_t d, uint64_t i) const;

public:
	RMQ_table() {}
	uint64_t m_idx(uint64_t st, uint64_t ed) const;
	uint64_t r_val(uint64_t idx) const { return rval[idx];}

	bool is_min() const { return _min_structre; }
	void clear() { len = 0; table.clear(); rval.clear(); }

	std::string to_str() const;
	void save(OArchive& ar) const;
	void load(IArchive& ar);

private:
	FixedWArray table; // idx_bwidth
	FixedWArray rval; // valbw
	//unsigned int val_bwidth, idx_bwidth;

	bool _min_structre;
	uint64_t len, max_val;
	
	friend class RMQ_table_builder;
	size_t nlayers() const { return msb_intr(len) + 1; }
};

}//namespace

