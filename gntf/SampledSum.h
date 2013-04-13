
#pragma once

#include "archive.h"
#include "poly_vals.h"
#include "valrange.h"
#include "poly_vals.h"
#include "nintv.h"

#include <deque>

namespace app_ds {

class SampledSumQuery;

class SampledSumBuilder {
public:
	SampledSumBuilder() { init(); }
	void init(unsigned int sample_rate = 64);
	void add(unsigned int st, unsigned int ed, double val);
	void add_all(std::deque<ValRange>* vals);
	void build(SampledSumQuery * out);
	void build(mscds::OArchive& ar);
	void clear();
private:

	mscds::SDArraySmlBuilder psbd, spsbd;
	PRValArrBuilder vals;
	std::deque<ValRange> svals, * ptr;
	uint64_t psum, sqpsum;
	int64_t lastv;
	unsigned int sample_rate;
	unsigned int lastst, cnt;
	unsigned int factor;
	int delta;

	static unsigned int precision(double d);

	void addint(unsigned int st, unsigned int ed, unsigned int v);
	void comp_transform();
	void add_all_vals();
};

class SampledSumQuery {
public:
	SampledSumQuery(): pq(NULL) {}
	typedef PRValArr::Enum Enum;
	double getValue(unsigned int idx) const;
	unsigned int getSampleRate() const { return rate; }

	/* return the sampled position */
	unsigned int getEnum(unsigned int idx, Enum * e) const;
	
	/* return the sum value at the nearest previous sampled value */
	double sum(unsigned int idx, unsigned int lefpos = 0) const;
	 
	double sqrSum(unsigned int pos, unsigned int lefpos = 0) const;
	
	void load(mscds::IArchive& ar, NIntvQueryInt * posquery);
	void save(mscds::OArchive& ar) const;
	void clear();
private:
	NIntvQueryInt * pq;
	friend class SampledSumBuilder;
	mscds::SDArraySml psum, sqrsum;
	unsigned int len, rate;
	unsigned int factor;
	int delta;
	PRValArr vals;
};


}//namespace

