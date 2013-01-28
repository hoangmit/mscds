#pragma once

#include <stdint.h>
#include <vector>
#include "archive.h"
#include "intarray/intarray.h"
#include "intarray/codearray.h"
#include "intarray/sdarray_sml.h"
#include "mem/filearchive.h"


namespace app_ds {

class PRSumArrInt: public mscds::SaveLoadInt {
public:
	virtual uint64_t access(size_t p) const = 0;
	virtual uint64_t sum(size_t p) const = 0;
	virtual uint64_t sqsum(size_t p) const = 0;
};

class PRSumArr;

class PRSumArrBuilder {
public:
	PRSumArrBuilder();

	void init(unsigned int method, unsigned int rate);

	void add(unsigned int v);
	void build(PRSumArr* out);
private:
	static const unsigned int CHECK_THRESHOLD = 10000;
	unsigned int method;
	uint64_t cnt;
	unsigned int rate;
	int lastval;
	std::vector<unsigned int> vals;

	mscds::SDArraySmlBuilder sdab;
	mscds::DeltaCodeArrBuilder dt1;
	mscds::DiffDeltaArrBuilder dt2;

	void resetbd();

	void addmethod(unsigned int val);
	void choosemethod();
};


class PRSumArr {
public:
	PRSumArr(): len(0) {}
	unsigned int sample_rate() { return rate; }
	uint64_t access(size_t p);
	void save(mscds::OArchive& ar) const;
	void load(mscds::IArchive& ar);
	void clear();
	size_t length() const { return len; }
	mscds::EnumeratorInt<uint64_t> * getEnum(size_t idx) const;
private:
	int storetype; // 1 - SDArray, 2-Delta, 3-DeltaWrap

	uint64_t len;
	unsigned int rate;
	mscds::SDArraySml sda;
	mscds::DeltaCodeArr dt1;
	mscds::DiffDeltaArr dt2;
	friend class PRSumArrBuilder;
};

}//namespace
