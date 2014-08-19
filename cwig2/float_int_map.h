#pragma once

// Similar to a Minimal perfect hash function from 32-bit float to integer

#include "bitarray/bitstream.h"
#include "intarray/sdarray_sml.h"
#include <cassert>
#include <set>

namespace app_ds {

class FloatIntMapBuilder;


/// Map floating point numbers to unsigned integer space
/**
	3 levels of codes:
		float:   original, can do all operations
		integer: scaled, can only do addition (minus?)
		symbol : cannot do  arithmetic, but can do equal comparison
*/
class FloatIntMapQuery {
public:
	//map a float to symbol
	uint64_t map_fs(double d) const;
	//unmap a symbol to float value
	double unmap_sf(uint64_t val) const;

	// partial unmap/map from symbol to integer
	int64_t unmap_si(uint64_t val) const;
	uint64_t map_is(int64_t val) const;

	// unmap from integer to float
	double unmap_if(int64_t v) const;
	int64_t map_fi(double d) const;

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);
	void clear();

	void inspect(const std::string &cmd, std::ostream &out) const;

private:
	friend class FloatIntMapBuilder;
	mscds::SDArraySml rankval;
	int64_t delta, factor;
};

class FloatIntMapBuilder {
public:
	void add(double d);
	void build(FloatIntMapQuery* out);


private:
	void comp_transform();

	int64_t delta, factor;

	std::set<double> vals;
	mscds::SDArraySmlBuilder ptrbd;
};


}//namespace
