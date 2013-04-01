#pragma once

#include <deque>
#include <cmath>
#include <stdint.h>
#include "archive.h"

#include "rlsum_int.h"
#include "poly_sum.h"

#include "nintv.h"
#include "valrange.h"

#include "intarray/sdarray_sml.h"
#include "codec/deltacoder.h"


namespace app_ds {

class RunLenSumArray6;

class RunLenSumArrayBuilder6 {
public:
	RunLenSumArrayBuilder6(): len(0), lastst(0), psum(0), sqpsum(0), lastv(0), cnt(0), ptr(&svals) {}
	void add(unsigned int st, unsigned int ed, double v);

	void build(RunLenSumArray6* arr);
	void build(mscds::OArchive& ar);
	void clear();
	typedef RunLenSumArray6 QueryTp;

	/* only use the method if you know what you are doing (avoid doubling the input the data) */
	void set_all(std::deque<ValRange> * vals) {ptr = vals;}
private:
	
	void comp_transform();
	void add_all_vals();

	void addint(unsigned int st, unsigned int ed, unsigned int val);
	unsigned int len, lastst, cnt;
	PNIntvBuilder itvb;

	mscds::SDArraySmlBuilder psbd, spsbd;
	PRSumArrBuilder vals;

	std::deque<ValRange> svals, * ptr;
	uint64_t psum, sqpsum;
	int64_t lastv;

	unsigned int factor;
	int delta;

	static unsigned int precision(double d);
};

class RunLenSumArray6 : public RunLenSumArrIt<double>  {
public:
	RunLenSumArray6(): len(0) {}
	~RunLenSumArray6() { clear(); }
	size_t load(std::istream& fi);
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;

	double sum(uint32_t pos) const;

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
	typedef RunLenSumArrayBuilder6 BuilderTp;
private:
	unsigned int len;
	PNIntv itv;
	unsigned int factor;
	int delta;

	mscds::SDArraySml psum, sqrsum;
	PRSumArr vals;
	friend class RunLenSumArrayBuilder6;
};

}//namespace
