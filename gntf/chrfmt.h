#pragma once

#include <string>
#include "RLSum6.h"
#include "tree/RMQ_sct.h"
#include "stringarr.h"
#include "archive.h"
#include "valrange.h"

namespace app_ds {


typedef RunLenSumArray6 ChrNumValType;
typedef ChrNumValType::BuilderTp ChrNumValBuilderType;

class ChrNumThread;

enum minmaxop_t {NO_MINMAX= 0, MIN_OP=1, MAX_OP=2, ALL_OP=3};

class ChrNumThreadBuilder {
public:
	ChrNumThreadBuilder();
	void init(minmaxop_t option=NO_MINMAX, bool range_annotations = false);
	void add(unsigned int st, unsigned int ed, double val, const std::string& s = "");
	void build(mscds::OArchive& ar);
	void build(ChrNumThread* out);
private:
	std::deque<ValRange> ranges;
	bool setup_;
	minmaxop_t minmax_opt;

	bool has_annotation;
	StringArrBuilder annbd;
};

class ChrNumThread {
public:
	/** \brief returns the i-th range's annotation (if available) */
	const std::string range_annotation(unsigned int i) const;

	/** \brief returns the position of the next non-zero value */
	unsigned int next_nz(unsigned int) const;

	/** \brief returns the position of the previous non-zero value */
	unsigned int prev_nz(unsigned int) const;

	/** \brief returns the last position i.e. the length of the sequence */
	unsigned int last_position() const;


	/** \brief return the sum of the position from 0 to p */
	double sum(unsigned int p) const;

	std::vector<double> sum_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	double avg(unsigned int st, unsigned int ed) const;

	std::vector<double> avg_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief finds the list of intervals [i..i'] that intersect with query range [st..ed] */
	std::pair<unsigned int, unsigned int> find_intervals(unsigned int st, unsigned int ed) const;

	/** \brief counts the number of non-zero ranges that start from 0 to i (inclusive) */
	unsigned int count_intervals(unsigned int i) const;

	/** \brief returns the total number of intervals */
	unsigned int count_intervals() const;

	std::vector<unsigned int> count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief counts the number of non-zero position from 0 to i */
	unsigned int coverage(unsigned int) const;
	std::vector<unsigned int> coverage_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the minimum value in [st..ed) */
	double min_value(unsigned int st, unsigned int ed) const;

	std::vector<double> min_value_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the minimum value in [st..ed) */
	double max_value(unsigned int st, unsigned int ed) const;

	std::vector<double> max_value_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief returns the standdard deviation of the values in [st..ed) */
	double stdev(unsigned int st, unsigned int ed) const;
	std::vector<double> stdev_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	void clear();
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	void dump_bedgraph(std::ostream& fo) const;
	std::string name;
private:
	ChrNumValType vals;
	StringArr annotations;
	mscds::RMQ_sct min, max;

	bool has_annotation;
	minmaxop_t minmax_opt;
	friend class ChrNumThreadBuilder;
};


}//namespace

namespace app_ds {

inline double ChrNumThread::sum(unsigned int p) const {
	return vals.sum(p);
}

inline double ChrNumThread::avg(unsigned int st, unsigned ed) const {
	return (sum(ed) - sum(st)) / (coverage(ed) - coverage(st));
}

inline const std::string ChrNumThread::range_annotation(unsigned int i) const {
	if (has_annotation)
		return annotations.get(i);
	else return "";
}

inline unsigned int ChrNumThread::last_position() const { return vals.last_position(); }

inline unsigned int ChrNumThread::count_intervals(unsigned int i) const {
	return vals.count_range(i);
}

inline double ChrNumThread::min_value(unsigned int st, unsigned int ed) const {
	if (minmax_opt & MIN_OP) {
		auto ls = vals.find_intervals(st, ed - 1);
		if (ls.first < ls.second) {
			unsigned int i = min.m_idx(ls.first, ls.second);
			return vals.range_value(i);
		}else return 0;
	}else return 0;
}

inline double ChrNumThread::max_value(unsigned int st, unsigned int ed) const {
	if (minmax_opt & MAX_OP) {
		auto ls = vals.find_intervals(st, ed - 1);
		if (ls.first < ls.second) {
			unsigned int i = max.m_idx(ls.first, ls.second);
			return vals.range_value(i);
		}else return 0;
	}else return 0;
}

inline std::pair<unsigned int, unsigned int> ChrNumThread::find_intervals(unsigned int st, unsigned int ed) const {
	auto r = vals.find_intervals(st, ed);
	std::pair<unsigned int, unsigned int> ret(1,0);
	if (r.first <= r.second) {
		ret = r;
		return ret;
	} return ret;
}

inline unsigned int ChrNumThread::next_nz(unsigned int p) const {
	return vals.next(p);
}

inline unsigned int ChrNumThread::prev_nz(unsigned int p) const {
	return vals.prev(p);
}

inline unsigned int ChrNumThread::coverage(unsigned int p) const {
	return vals.countnz(p);
}

}//namespace
