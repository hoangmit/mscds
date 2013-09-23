
#pragma once

#include "archive.h"
#include "poly_vals.h"
#include "rank_vals.h"
#include "valrange.h"
#include "nintv.h"

#include <deque>

namespace app_ds {

class SampledSumQuery;

class SampledSumBuilder {
public:
	SampledSumBuilder();
	void init(unsigned int sample_rate = 64);
	void add(unsigned int st, unsigned int ed, double val);
	void add_all(std::deque<ValRange>* vals);
	void build(SampledSumQuery * out, NIntvQueryInt * posquery);
	void build(mscds::OArchive& ar);
	void clear();
private:

	mscds::SDArraySmlBuilder psbd, spsbd;
	PRValArrBuilder vdir;
	RankValArrBuilder vrank;
	std::deque<ValRange> svals, * ptr;
	uint64_t psum, sqpsum;
	int64_t lastv;
	unsigned int sample_rate;
	unsigned int lastst, cnt;
	unsigned int factor;
	int delta;
	unsigned int method;

	static unsigned int precision(double d);

	void addint(unsigned int st, unsigned int ed, unsigned int v);
	void comp_transform();
	void add_all_vals();
};

class SampledSumQuery {
public:
	SampledSumQuery();

	double access(unsigned int idx) const;
	unsigned int getSampleRate() const { return rate; }

	class Enum: public mscds::EnumeratorInt<double> {
	public:
		Enum(): e1(NULL), e2(NULL), type(0) {}
		~Enum();
		bool hasNext() const;
		double next();
		uint64_t next_int();
	private:
		friend class SampledSumQuery;
		PRValArr::Enum * e1;
		RankValArr::Enum * e2;
		unsigned int factor;
		int delta;
		unsigned char type;
	};

	/* return the sampled position */
	void getEnum(unsigned int idx, Enum * e) const;
	
	/* return the sum value at the nearest previous sampled value */
	double sum(unsigned int idx, unsigned int lefpos = 0) const;
	 
	double sqrSum(unsigned int idx, unsigned int lefpos = 0) const;
	
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
	unsigned int method; // 1--normal, 2--rank
	PRValArr vdir;
	RankValArr vrank;

};


}//namespace

