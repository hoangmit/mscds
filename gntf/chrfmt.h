#pragma once

#include <string>
#include "sdarray/RLSum.h"
#include "tree/RMQ_sct.h"
#include "archive.h"

namespace app_ds {

struct ValRange {
	unsigned int st, ed;
	int val;
	ValRange() {}
	ValRange(unsigned int s, unsigned int e, int v):st(s), ed(e), val(v) {}
	bool operator<(const ValRange& e) const {
		return st < e.st;
	}
	bool operator==(const ValRange& e) const {
		return st == e.st && ed == e.ed && val == e.val;
	}
};


class ChrNumThread;

enum minmaxop_t {NO_MINMAX= 0, MIN_OP=1, MAX_OP=2, ALL_OP=3};

class ChrNumThreadBuilder {
public:
	
	ChrNumThreadBuilder();
	void init(minmaxop_t option=NO_MINMAX, unsigned int factor=1);
	void add(unsigned int st, unsigned int ed, int val);
	void build(mscds::OArchive& ar);
	void build(ChrNumThread* out);
private:
	std::deque<ValRange> ranges;
	bool setup_;
	unsigned int factor;
	minmaxop_t minmax_opt;
	int find_min();
};

class ChrNumThread {
public:
	int64_t sum(size_t p) const;
	unsigned int min_value(unsigned int st, unsigned int ed) const;
	unsigned int max_value(unsigned int st, unsigned int ed) const;

	void clear();
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;

	void dump_bedgraph(std::ostream& fo) const;
	std::string name;
private:
	mscds::RunLenSumArray vals;
	mscds::RMQ_sct min, max;

	minmaxop_t minmax_opt;
	unsigned int factor;
	int64_t delta;
	friend class ChrNumThreadBuilder;
};

}//namespace