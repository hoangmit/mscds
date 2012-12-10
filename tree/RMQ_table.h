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
	uint64_t get_tb(uint64_t d, uint64_t i) const {
		if (d == 0) return i;
		uint64_t start = len*(d-1) - (1ULL<<(d-1)) + 1;
		return table[start + i];
	}

public:
	RMQ_table() {}
	uint64_t m_idx(uint64_t st, uint64_t ed) const {
		assert(st < ed && ed <= len);
		if (ed - st == 1) return st;
		unsigned int d = msb_intr(ed - st);
		uint64_t start = len*(d-1) - (1ULL<<(d-1)) + 1;
		uint64_t i1 = table[start + st];
		if (ed - (1ULL<<d) != st) {
			uint64_t i2 = table[start + ed - (1ULL<<d)];
			if (_min_structre != (r_val(i1) > r_val(i2))) return i1;
			else return i2;
		}else return i1;
	}

	uint64_t r_val(uint64_t idx) const {
		return rval[idx];
	}

	bool is_min() const { return _min_structre; }
	void clear() { len = 0; table.clear(); rval.clear(); }

	std::string to_str() const {
		std::ostringstream ss;
		if (_min_structre) ss << "min_structure\n";
		else ss << "max_structure\n";
		if (len == 0) return "";
		ss << '{' << r_val(0);
		for (size_t i = 1; i < len; ++i) 
			ss << ',' << r_val(i);
		ss << "}\n-\n";
		size_t md = nlayers();
		for (size_t d = 0; d < md; d++) {
			ss << '{' << get_tb(d, 0);
			uint64_t span = d == 0 ? 0 : 1ULL << (d-1);
			for (size_t i = 1; i < (len - span); ++i) {
				ss << "," << get_tb(d, i);
			}
			ss << "}\n";
		}
		return ss.str();
	}

	void save(OArchive& ar) const {
		ar.startclass("rmq_table", 1);
		ar.var("len").save(len);
		ar.var("max_val").save(max_val);
		uint32_t mins = _min_structre ? 1 : 0;
		ar.var("min_structure").save(mins);
		ar.var("values");
		rval.save(ar);
		ar.var("table");
		table.save(ar);
		ar.endclass();
	}

	void load(IArchive& ar) {
		clear();
		ar.loadclass("rmq_table");
		ar.var("len").load(len);
		ar.var("max_val").load(max_val);
		uint32_t mins;
		ar.var("min_structure").load(mins);
		_min_structre = mins != 0;
		ar.var("values");
		rval.load(ar);
		ar.var("table");
		table.load(ar);
		ar.endclass();
		//init_width();
	}

private:
	FixedWArray table; // idx_bwidth
	FixedWArray rval; // valbw
	//unsigned int val_bwidth, idx_bwidth;

	bool _min_structre;
	uint64_t len, max_val;
	

	friend class RMQ_table_builder;
	/*void init_width() {
		val_bwidth = ceillog2(max_val);
		idx_bwidth = ceillog2(len);
	}*/

	size_t nlayers() const {
		return msb_intr(len) + 1;
	}
};

}//namespace


#include <map>

namespace mscds {
using namespace std;

inline void RMQ_table_builder::pack(bool pack_values, const std::vector<uint64_t>& lst, std::vector<uint64_t>& nlst, uint64_t& maxval) {
	if (pack_values) {
		nlst.resize(lst.size());
		std::map<uint64_t, uint64_t> mapper;
		for (auto it = lst.begin(); it != lst.end(); ++it)
			mapper[*it] = 1;
		uint64_t idx = 0;
		for (auto it = mapper.begin(); it != mapper.end(); ++it) {
			it->second = idx;
			idx++;
		}
		maxval = idx;
		for (uint64_t i = 0; i < lst.size(); ++i)
			nlst[i] = mapper[lst[i]];
	} else {
		nlst = lst;
		maxval = *std::max_element(nlst.begin(), nlst.end());
	}
}

template<typename Vec>
inline uint64_t choose(bool ismin, const Vec& v, uint64_t i1, uint64_t i2) {
	if (ismin != (v[i1] > v[i2])) return i1;
	else return i2;
}

inline void RMQ_table_builder::build(const std::vector<uint64_t>& lst, RMQ_table * t,  bool min_structre, bool pack_values) {
	assert(lst.size() > 1);
	t->clear();
	t->len = lst.size();
	t->_min_structre = min_structre;
	std::vector<uint64_t> nlst;
	pack(pack_values, lst, nlst, t->max_val); // set max_val
	//t->init_width(); // initialize idx_bwidth and val_bwidth
	unsigned int idx_bwidth = ceillog2(t->len);
	unsigned int val_bwidth = ceillog2(t->max_val);
	t->rval = FixedWArray::create(nlst.size(), val_bwidth);
	t->rval.fillzero();
	for (size_t i = 0; i < nlst.size(); ++i) 
		t->rval.set(i, nlst[i]);
	size_t px = 0;
	uint64_t nl = t->nlayers();
	vector<uint64_t> last(t->len);
	for (size_t i = 0; i < t->len; ++i) last[i] = i;
	assert(nl < 64);
	t->table = FixedWArray::create((t->len*(nl-1) - (1ULL<<(nl-1)) + 1), idx_bwidth);
	for (size_t d = 1; d < nl; ++d) {
		uint64_t span = 1ULL << (d-1);
		for (size_t i = 0; i < t->len - span; i++) {
			uint64_t v = choose(min_structre, nlst, last[i], last[i+span]);
			t->table.set(px, v);
			px++;
			last[i] = v;
		}
	}
	//assert(px == t->table.length() / t->idx_bwidth);
}

}//namespace
