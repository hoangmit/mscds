#pragma once

#include "bitarray/bitarray.h"
#include "bitarray/select_dense.h"

namespace mscds {

class SDArrayTH;

class SDArrayTHBuilder {
public:
	SDArrayTHBuilder();
	void add(uint64_t val);
	void add_inc(uint64_t pos);

	void build(SDArrayTH* out);
private:
	std::vector<uint64_t> vals;
	uint64_t last;
};


/// SDArray with more linear structures
class SDArrayTH {
public:
	/** returns the value of A[i] */
	uint64_t lookup(const uint64_t i) const;

	/** returns the value of A[i] and  prev_sum=prefix_sum(i) */
	uint64_t lookup(const uint64_t i, uint64_t& prev_sum) const;

	/** return the value of prefix_sum(i) */
	uint64_t prefixsum(size_t i) const;

	/** return the value of rank(p) */
	uint64_t rank(uint64_t p) const;

	/** clear the array (length becomes 0) */
	void clear();

	/** save and load functions */
	void save(OutArchive& ar) const;
	void load(InpArchive& ar);

	/** to string for debug */
	std::string to_str(bool psum) const;

	/** returns the sum of all the elements in the array */
	uint64_t total() const ;

	/** counts the number of elements in the array */
	uint64_t length() const ;

	void inspect(const std::string&, std::ostream&) const;
private:
	uint64_t select_hi(uint64_t r) const;
	size_t len, sum;
	unsigned int width;
	BitArray lower;

	BitArray upper;
	SelectDenseAux saux0, saux1;
	friend class SDArrayTHBuilder;
};

}//namespace
