#pragma once

#include "cwig/valrange.h"
#include "cwig/rlsum_int.h"

#include "fusion/blkgroup_array.h"
#include "fusion/fuse_blk_model.h"
#include "fusion/ps_access_blk.h"
#include "cwig2/intv/nintv_fuse.h"
#include "float_int_map.h"

#include "intarray/sdarray.h"

#include "fusedstorage.h"

#include <stdint.h>

namespace app_ds {

template<typename IVS> class IntValQueryG;

template <typename IVS>
class IntValBuilderG {
public:
	typedef typename IntValQueryG<IVS> QueryTp;
	void add(unsigned int st, unsigned int ed, double val) { bd.add(st, ed, val); }
	void build(QueryTp* qs) { bd.build(&(qs->data)); }
	void build(mscds::OutArchive& ar) {
		QueryTp qs;
		build(&qs);
		qs.save(ar);
	}
private:
	typedef typename IVS::BuilderTp BD;
	BD bd;

};

/*
struct IVS {
	int_start
	int_len
	int_startend
	find_cover
	coverage

	int_psrlen

	-------------

	ValEnum

	get_val(idx)
	getValEnum

	-------------

	get_sumq(idx)
};
*/



template<typename IVS>
class IntValQueryG: public RunLenSumArrIt<double> {
public:	
	IntValQueryG();

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
		friend class IntValQueryG;
		const IntValQueryG * ptr;
		size_t i;
	};
	void getEnum(unsigned int idx, Enum* e) const;
	//void inspect(const std::string& cmd, std::ostream& out) const;
	typedef typename IntValBuilderG<IVS> BuilderTp;
private:
	double sum_intv(unsigned int idx, unsigned int leftpos = 0) const;
	double sqrSum_intv(unsigned int idx, unsigned int leftpos = 0) const;

	friend class IntValBuilder;
	static const unsigned int rate = IVS::SUM_GAP; // = 64

	friend class BuilderTp;

	IVS data;
};

template<typename IVS>
void IntValQueryG<IVS>::save(mscds::OutArchive &ar) const {
	ar.startclass("IntVal", 1);
	data.save(ar.var("data"));
	ar.endclass();
}

template<typename IVS>
void IntValQueryG<IVS>::load(mscds::InpArchive &ar) {
	int class_version = ar.loadclass("IntVal");	
	data.load(ar.var("data"));
	ar.endclass();
}

template<typename IVS>
unsigned int IntValQueryG<IVS>::length() const {
	return data.length();
}

template<typename IVS>
void IntValQueryG<IVS>::clear() {
	data.clear();
}

template<typename IVS>
unsigned int IntValQueryG<IVS>::range_start(unsigned int i) const {
	return data.itv.int_start(i);
}

template<typename IVS>
unsigned int IntValQueryG<IVS>::range_len(unsigned int i) const {
	return data.itv.int_len(i);
}

template<typename IVS>
double IntValQueryG<IVS>::range_psum(unsigned int i) const {
	return sum_intv(i, 0);
}

template<typename IVS>
typename IntValQueryG<IVS>::IntervalInfo IntValQueryG<IVS>::range_at(unsigned int i) const {
	auto x = data.itv.int_startend(i);
	return IntValQueryG<IVS>::IntervalInfo(x.first, x.second, range_value(i));
}

template<typename IVS>
unsigned int IntValQueryG<IVS>::count_range(unsigned int pos) const {
	auto res = data.itv.find_cover(pos);
	if (res.first == 0 && res.second == 0) return 0;
	else return res.first + 1;
}

template<typename IVS>
unsigned int IntValQueryG<IVS>::countnz(unsigned int p) const {
	return data.itv.coverage(p);
}

template<typename IVS>
double IntValQueryG<IVS>::access(unsigned int p) const {
	auto res = data.itv.find_cover(p);
	if (res.second != 0) return range_value(res.first);
	else return 0;
}

template<typename IVS>
int IntValQueryG<IVS>::prev(unsigned int p) const {
	auto res = data.itv.find_cover(p);
	if (res.second > 0) return p;
	else if (res.first > 0) return data.itv.int_end(res.first - 1) - 1;
	else return -1;
}

template<typename IVS>
int IntValQueryG<IVS>::next(unsigned int p) const {
	auto res = data.itv.find_cover(p);
	if (res.second > 0) return p;
	else if (res.first < length()) return data.itv.int_start(res.first);
	else return -1;
}

template<typename IVS>
std::pair<int, int> IntValQueryG<IVS>::find_intervals(unsigned int st, unsigned int ed) const {
	assert(st < ed);
	std::pair<unsigned int, unsigned int> ret;
	auto r1 = data.itv.find_cover(st);
	decltype(r1) r2;
	if (st + 1 < ed) r2 = data.find_cover(ed - 1);
	else r2 = r1; // one position
	if (r1.second > 0) ret.first = r1.first;
	else ret.first = r1.first + 1;
	if (r2.second > 0) ret.second = r2.first + 1;
	else ret.second = r2.first;
	return ret;
}

template<typename IVS>
double IntValQueryG<IVS>::sum(uint32_t pos) const {
	if (pos == 0) return 0;
	auto res = data.itv.find_cover(pos - 1);
	if (res.first == 0 && res.second == 0) return 0;
	return sum_intv(res.first, res.second);
}

template<typename IVS>
double IntValQueryG<IVS>::sqrsum(uint32_t pos) const {
	throw std::runtime_error("not implemented");
	return 0;
}

template<typename IVS>
double IntValQueryG<IVS>::range_value(unsigned int idx) const {
	return data.get_val(idx);
}

template<typename IVS>
double IntValQueryG<IVS>::sum_intv(unsigned int idx, unsigned int leftpos) const {
	size_t r = idx % rate;
	size_t p = idx / rate;
	int64_t tlen = data.itv.int_psrlen(idx - r);
	double cpsum = data.get_sumq(p);
	size_t base = p * rate;
	typename IVS::Enum e;
	if (r > 0 || leftpos > 0) {
		data.getEnum(base, &e);
		for (size_t i = 0; i < r; ++i) {
			double v = e.next();
			cpsum += data.itv.int_len(base + i) * v;
		}
	}
	if (leftpos > 0)
		cpsum += e.next() * leftpos;
	return cpsum;
}

template<typename IVS>
double IntValQueryG<IVS>::sqrSum_intv(unsigned int idx, unsigned int leftpos) const {
	throw std::runtime_error("not implemented");
	return 0;
}

template<typename IVS>
IntValQueryG<IVS>::IntValQueryG() {}


template<typename IVS>
void IntValQueryG<IVS>::getEnum(unsigned int idx, typename IntValQueryG<IVS>::Enum *e) const {
	e->ptr = this;
	e->i = idx;
}

template<typename IVS>
bool app_ds::IntValQueryG<IVS>::Enum::hasNext() {
	return i < ptr->length();
}

template<typename IVS>
typename IntValQueryG<IVS>::IntervalInfo IntValQueryG<IVS>::Enum::next() {
	return ptr->range_at(i++);
}

typedef IntValBuilderG<Storage> IntValBuilder2;
typedef IntValQueryG<Storage> IntValQuery2;

}//namespace
