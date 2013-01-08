#pragma once

#include <string>
#include "sdarray/RLSum2.h"
#include "tree/RMQ_sct.h"
#include "stringarr.h"
#include "archive.h"

namespace app_ds {

struct ValRange {
	unsigned int st, ed;
	int val;
	std::string annotation;
	ValRange() {}
	ValRange(unsigned int s, unsigned int e, int v):st(s), ed(e), val(v) {}
	ValRange(unsigned int s, unsigned int e, int v, const std::string& ann):st(s), ed(e), val(v), annotation(ann) {}
	bool operator<(const ValRange& e) const { return st < e.st; }
	bool operator==(const ValRange& e) const {
		return st == e.st && ed == e.ed && val == e.val;
	}
};

class ChrNumThread;

enum minmaxop_t {NO_MINMAX= 0, MIN_OP=1, MAX_OP=2, ALL_OP=3};

class ChrNumThreadBuilder {
public:
	ChrNumThreadBuilder();
	void init(minmaxop_t option=NO_MINMAX, unsigned int factor=100, bool range_annotations = false);
	void add(unsigned int st, unsigned int ed, int val, const std::string& s = "");
	void build(mscds::OArchive& ar);
	void build(ChrNumThread* out);
private:
	std::deque<ValRange> ranges;
	bool setup_;
	unsigned int factor;
	minmaxop_t minmax_opt;
	int find_min();

	bool has_annotation;
	StringArrBuilder annbd;
};

class ChrNumThread {
public:
	/** \brief return the sum of the position from 0 to p */
	int64_t sum(size_t p) const;

	/** \brief returns the i-th range's annotation (if available) */
	const std::string range_annotation(unsigned int i) const;
	
	/** \brief counts the number of non-zero ranges that start from 0 to i (inclusive) */
	unsigned int count_range(unsigned int i) const;

	/** \brief returns the minimum value in [st..ed) */
	unsigned int min_value(unsigned int st, unsigned int ed) const;

	/** \brief returns the minimum value in [st..ed) */
	unsigned int max_value(unsigned int st, unsigned int ed) const;

	/** \brief returns the position of the next non-zero value */
	unsigned int next_nz(unsigned int) const;

	/** \brief returns the position of the previous non-zero value */
	unsigned int prev_nz(unsigned int) const;

	/** \brief counts the number of non-zero position from 0 to i */
	unsigned int count_nz(unsigned int) const;

	void clear();
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	void dump_bedgraph(std::ostream& fo) const;
	std::string name;
private:
	mscds::RunLenSumArray2 vals;
	StringArr annotations;
	mscds::RMQ_sct min, max;

	bool has_annotation;
	minmaxop_t minmax_opt;
	unsigned int factor;
	int64_t delta;
	friend class ChrNumThreadBuilder;
};


}//namespace

namespace app_ds {

inline int64_t ChrNumThread::sum(size_t p) const {
	return vals.sum_delta(p, delta);
}

inline const std::string ChrNumThread::range_annotation(unsigned int i) const {
	if (has_annotation)
		return annotations.get(i);
	else return "";
}

inline unsigned int ChrNumThread::count_range(unsigned int i) const {
	return vals.count_range(i);
}

inline unsigned int ChrNumThread::min_value(unsigned int st, unsigned int ed) const {
	if (minmax_opt & MIN_OP) {
		unsigned int i = (unsigned int) min.m_idx(st, ed);
		return (int)(vals.range_value(i) + delta);
	}else return 0;
}

inline unsigned int ChrNumThread::max_value(unsigned int st, unsigned int ed) const {
	if (minmax_opt & MAX_OP) {
		unsigned int i = (unsigned int) min.m_idx(st, ed);
		return (int)(vals.range_value(i) + delta);
	}else return 0;
}

inline unsigned int ChrNumThread::next_nz(unsigned int p) const {
	return vals.next(p);
}

inline unsigned int ChrNumThread::prev_nz(unsigned int p) const {
	return vals.prev(p);
}

inline unsigned int ChrNumThread::count_nz(unsigned int p) const {
	return vals.countnz(p);
}

}//namespace
