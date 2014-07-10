#include "float_int_map.h"
#include "cwig/float_precision.h"

#include <algorithm>

namespace app_ds {

uint64_t FloatIntMapQuery::map_fs(double d) const {
	uint64_t valt = (int64_t)(d * factor) - delta;
	valt = rankval.rank(valt);
	return valt;
}

double FloatIntMapQuery::unmap_sf(uint64_t val) const {
	return ((int64_t)rankval.prefixsum(val) + delta) * 1.0 / factor;
}

uint64_t FloatIntMapQuery::map_is(int64_t val) const {
	return rankval.rank(val - delta);
}

int64_t FloatIntMapQuery::unmap_si(uint64_t val) const {
	return (int64_t)rankval.prefixsum(val) + delta;
}

int64_t FloatIntMapQuery::map_fi(double d) const {
	return (int64_t)(d * factor);
}

double FloatIntMapQuery::unmap_if(int64_t v) const {
	return v  * 1.0 / factor;
}

void FloatIntMapQuery::save(mscds::OutArchive &ar) const {
	ar.startclass("FloatIntMap", 1);
	ar.var("delta").save(delta);
	ar.var("factor").save(factor);
	rankval.save(ar.var("rankval"));
	ar.endclass();
}

void FloatIntMapQuery::load(mscds::InpArchive &ar) {
	ar.loadclass("FloatIntMap");
	ar.var("delta").load(delta);
	ar.var("factor").load(factor);
	rankval.load(ar.var("rankval"));
	ar.endclass();
}

void FloatIntMapQuery::clear() {
	delta = 0;
	factor = 0;
	rankval.clear();
}

void FloatIntMapQuery::inspect(const std::string &cmd, std::ostream &out) const {
	out << '{';
	out << '"' << "delta" << "\": " << delta << ",";
	out << '"' << "factor" << "\": " << factor << ",";
	out << '"' << "n_distinct_values" << "\": " << rankval.length() << ", ";
	out << "\"block_data\": ";
	out << '}';
}

void FloatIntMapBuilder::add(double d) {
	vals.insert(d);
}

void FloatIntMapBuilder::build(FloatIntMapQuery *out) {
	comp_transform();
	out->delta = delta;
	out->factor = factor;
	ptrbd.build(&(out->rankval));
	vals.clear();
}


void FloatIntMapBuilder::comp_transform() {
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

}//namespace
