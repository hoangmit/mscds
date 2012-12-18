#pragma once

#include <deque>
#include <stdint.h>
#include "archive.h"

#include "bitarray/sdarray.h"

namespace mscds {

class RunLenSumArray;

class RunLenSumArrayBuilder {
public:
	RunLenSumArrayBuilder(): len(0), lastst(0) {}
	void add(unsigned int st, unsigned int ed, unsigned int v);
	void build(RunLenSumArray* arr);
	void build(OArchive& ar);
	void clear();
private:
	unsigned int len, lastst;
	SDArrayBuilder psbd, rlenbd;
	SDRankSelect stbd;
	std::vector<unsigned int> stpos;
};

class RunLenSumArray {
public:
	RunLenSumArray();
	~RunLenSumArray() { clear(); }
	size_t load(std::istream& fi);
	void load(IArchive& ar);
	void save(OArchive& ar) const;

	uint64_t sum(uint32_t pos);

	unsigned int length() const;
	void clear();

	uint64_t range_start(unsigned int i);
	uint64_t range_psum(unsigned int i);
	unsigned int range_len(unsigned int i);
private:
	unsigned int len;
	SDRankSelect start;
	SDArrayQuery psum, rlen;
	friend class RunLenSumArrayBuilder;
};

}//namespace
