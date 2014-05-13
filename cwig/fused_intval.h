#pragma once

#include "valrange.h"
#include "rlsum_int.h"

#include "fusion/blkgroup_array.h"
#include "fusion/fuse_blk_model.h"
#include "fusion/ps_access_blk.h"
#include "cwig/intv/nintv_fuse.h"
#include "intarray/sdarray.h"


#include <stdint.h>

namespace app_ds {

class IntValQuery;
class IntValBuilder {
public:
	typedef mscds::LiftStBuilder<NIntvInterBlkBuilder,
			mscds::CodeInterBlkBuilder,
			mscds::PtrInterBlkBd,
			mscds::PtrInterBlkBd
		> BD;
	typedef mscds::LiftStQuery<FuseNIntvInterBlock,
		mscds::CodeInterBlkQuery,
		mscds::PtrInterBlkQs,
		mscds::PtrInterBlkQs
	> QS;

	void add(unsigned int st, unsigned int ed, double val);
	void build(IntValQuery* qs);
	void build(mscds::OutArchive& ar);
	void build(const std::deque<ValRange>& all, IntValQuery* qs);
	typedef IntValQuery QueryTp;
private:
	void comp_transform(const std::deque<ValRange>& all);
	std::deque<ValRange> data;
	uint64_t factor;
	int64_t delta;
	mscds::SDArraySmlBuilder rvbd;
};

class IntValQuery : public RunLenSumArrIt<double> {
public:
	typedef IntValBuilder::QS QS;
	typedef IntValBuilder BuilderTp;
	IntValQuery();

	double sum(uint32_t pos) const;
	double sqrsum(uint32_t pos) const;

	void save(mscds::OutArchive& ar) const;
	void load(mscds::InpArchive& ar);

	unsigned int length() const;
	void clear();

	unsigned int range_start(unsigned int i) const;
	unsigned int range_len(unsigned int i) const;
	double range_value(unsigned int i) const;
	double range_psum(unsigned int i) const;
	
	unsigned int last_position() const { return length() > 0 ? range_start(length()-1) + range_len(length() - 1) : 0; }

	unsigned int count_range(unsigned int pos) const;

	/** \brief counts the number of non-zero numbers in the half-open range [0..p) */
	unsigned int countnz(unsigned int p) const;

	/** \brief returns the values of the integer at position `pos'. Note that
	sequence index starts at 0 */
	double access(unsigned int) const;

	/** \brief returns largest position that is less than or equal to the input
	and its value is non-zero (returns -1 if cannot find) */
	int prev(unsigned int) const;

	/** \brief returns the smallest position that is greater than the input
	and its value is non-zero (return -1 if cannot find) */
	int next(unsigned int) const;

	/** \brief finds the list of intervals [i..i'] that intersect with query range [st..ed] */
	std::pair<int, int> find_intervals(unsigned int st, unsigned int ed) const;

	struct IntervalInfo {
		IntervalInfo() {}
		IntervalInfo(unsigned int s, unsigned int e, double v): st(s), ed(e), val(v) {}
		IntervalInfo(const IntervalInfo& other): st(other.st), ed(other.ed), val(other.val) {}
		unsigned int st, ed;
		double val;
	};

	IntervalInfo range_at(unsigned int i) const;

	class Enum {
	public:
		bool hasNext();
		IntervalInfo next();
	private:
		friend class IntValQuery;
		const IntValQuery * ptr;
		size_t i;
	};
	void getEnum(unsigned int idx, Enum* e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;

private:
	double sum_intv(unsigned int idx, unsigned int leftpos = 0) const;
	double sqrSum_intv(unsigned int idx, unsigned int leftpos = 0) const;

	friend class IntValBuilder;
	static const unsigned int rate = 64;
	mscds::SDArraySml rankval;

	uint64_t factor;
	int64_t delta;
	uint64_t len;
	FuseNIntvInterBlock& itv;
	mscds::CodeInterBlkQuery& vals;
	mutable QS data;
};

}//namespace
