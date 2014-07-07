#pragma once

// Similar to a Minimal perfect hash function from 32-bit float to integer

#include "bitarray/bitstream.h"
#include "intarray/sdarray_sml.h"
#include <cassert>
#include <set>
#include <algorithm>


#include "cwig/float_precision.h"

namespace app_ds {

class FloatIntMapBuilder;

class FloatIntMapQuery {
public:
	uint64_t map(double d) {
		uint64_t valt = (int64_t)(d * factor) - delta;
		valt = rankval.rank(valt);
		return valt;
	}

	double unmap(uint64_t val) {
		return rankval.prefixsum(val);
	}
private:
	friend class FloatIntMapBuilder;
	mscds::SDArraySml rankval;
	int64_t delta, factor;
};

class FloatIntMapBuilder {
public:

	void add(double d) {
		vals.insert(d);
	}
	
	void build(FloatIntMapQuery* out) {
		comp_transform();
		out->delta = delta;
		out->factor = factor;
		ptrbd.build(&(out->rankval));
	}

private:
	void comp_transform() {
		unsigned int pc = 0;

		for (auto it = vals.cbegin(); it != vals.cend(); ++it)
			pc = std::max<unsigned int>(fprecision(*it), pc);
		factor = 1;
		if (pc > 5) pc = 5;

		for (unsigned int i = 0; i < pc; ++i) factor *= 10;
		int64_t minr = std::numeric_limits<int64_t>::max();
		for (auto it = vals.cbegin(); it != vals.cend(); ++it)
			minr = std::min<int64_t>(minr, (*it)*factor);
		delta = minr; // 1 - minr
		int64_t last = -1;
		for (auto it = vals.cbegin(); it != vals.cend(); ++it) {
			auto v = (int64_t)((*it) * factor) - delta;
			if (v != last) {
				ptrbd.add_inc(v);
				last = v;
			}
		}
	}

	int64_t delta, factor;

	std::set<double> vals;
	mscds::SDArraySmlBuilder ptrbd;
};


}//namespace