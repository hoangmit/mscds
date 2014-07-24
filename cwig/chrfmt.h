#pragma once

#include <string>
#include <stdexcept>

#include "tree/RMQ_sct.h"
#include "string/stringarr.h"
#include "framework/archive.h"
#include "valrange.h"

#include "RLSum6.h"
#include "../cwig2/fused_intval2.h"

namespace app_ds {

//typedef RunLenSumArray6 ChrNumValType;
//typedef ChrNumValType::BuilderTp ChrNumValBuilderType;

//typedef IntValQuery2 ChrNumValType;
//typedef IntValQuery2::BuilderTp ChrNumValBuilderType;

typedef IntValQuery3 ChrNumValType;
typedef IntValQuery3::BuilderTp ChrNumValBuilderType;

class ChrNumData;

enum minmaxop_t {NO_MINMAX= 0, MIN_OP=1, MAX_OP=2, ALL_OP=3};

class ChrNumDataBuilder {
public:
	ChrNumDataBuilder();
	void init(minmaxop_t option=NO_MINMAX, bool range_annotations = false);
	void add(unsigned int st, unsigned int ed, double val, const std::string& s = "");
	void build(mscds::OutArchive& ar);
	void build(ChrNumData* out);
private:
	std::deque<ValRange> ranges;
	bool setup_;
	minmaxop_t minmax_opt;

	bool has_annotation;
	mscds::StringArrBuilder annbd;
};

class ChrNumData {
public:
	ChrNumData();
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
	double sum(unsigned int st, unsigned int ed) const;
	double sqrsum(unsigned int st, unsigned int ed) const;

	std::vector<double> sum_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	double avg(unsigned int st, unsigned int ed) const;

	std::vector<double> avg_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief finds the list of intervals [i..i'] that intersect with query range [st..ed] */
	std::pair<unsigned int, unsigned int> find_intervals(unsigned int st, unsigned int ed) const;
	void getEnum(unsigned int i, ChrNumValType::Enum* e) const;

	/** \brief counts the number of non-zero ranges that start from 0 to i (inclusive) */
	unsigned int count_intervals(unsigned int i) const;

	/** \brief returns the total number of intervals */
	unsigned int count_intervals() const;

	std::vector<unsigned int> count_intervals_batch(unsigned int st, unsigned int ed, unsigned int n) const;

	/** \brief counts the number of non-zero position from 0 to i */
	unsigned int coverage(unsigned int) const;
	unsigned int coverage(unsigned int st, unsigned int ed) const;
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

	/** \brief returns the values of bases from st to ed */
	std::vector<double> base_value_map(unsigned int st, unsigned int ed) const;

	void inspect(const std::string& cmd, std::ostream& out) const;

	void clear();
	void load(mscds::InpArchive& ar);
	void save(mscds::OutArchive& ar) const;
	void dump_bedgraph(std::ostream& fo) const;
	std::string name;

private:
	ChrNumValType vals;
	mscds::StringArr annotations;
	mscds::RMQ_sct min, max;

	bool has_annotation;
	minmaxop_t minmax_opt;
	friend class ChrNumDataBuilder;
};


}//namespace

namespace app_ds {

inline double ChrNumData::sum(unsigned int p) const { return vals.sum(p); }

inline double ChrNumData::sum(unsigned int st, unsigned int ed) const {
	if (st >= ed) throw std::runtime_error("invalid input interval");
	return sum(ed) - sum(st);
}

inline double ChrNumData::avg(unsigned int st, unsigned ed) const {
	if (st >= ed) throw std::runtime_error("invalid input interval");
	return (sum(st, ed)) / coverage(st, ed);
}

inline const std::string ChrNumData::range_annotation(unsigned int i) const {
	if (has_annotation) return annotations.get_str(i);
	else return "";
}

inline unsigned int ChrNumData::last_position() const { return vals.last_position(); }

inline unsigned int ChrNumData::count_intervals(unsigned int i) const {
	return vals.count_range(i);
}

inline std::pair<unsigned int, unsigned int> ChrNumData::find_intervals(unsigned int st, unsigned int ed) const {
	if (st >= ed) throw std::runtime_error("invalid input interval");
	return vals.find_intervals(st, ed);
}

inline unsigned int ChrNumData::next_nz(unsigned int p) const { return vals.next(p); }

inline unsigned int ChrNumData::prev_nz(unsigned int p) const { return vals.prev(p); }

inline unsigned int ChrNumData::coverage(unsigned int p) const { return vals.countnz(p); }

inline unsigned int ChrNumData::coverage(unsigned int st, unsigned int ed) const {
	return coverage(ed) - coverage(st);
}

inline double ChrNumData::sqrsum(unsigned int st, unsigned int ed) const {
	return vals.sqrsum(ed) - vals.sqrsum(st);
}

}//namespace
