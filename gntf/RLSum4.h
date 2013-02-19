#pragma once

#include <deque>
#include <stdint.h>
#include "archive.h"

#include "rlsum_int.h"
#include "poly_sum.h"

//#include "sdarray/sdarray.h"
#include "intarray/sdarray_sml.h"
#include "codec/deltacoder.h"


namespace app_ds {

class RunLenSumArray4;

class RunLenSumArrayBuilder4 {
public:
	RunLenSumArrayBuilder4(): len(0), lastst(0), psum(0), sqpsum(0), lastv(0), cnt(0) {}
	void add(unsigned int st, unsigned int ed, unsigned int v);
	void build(RunLenSumArray4* arr);
	void build(mscds::OArchive& ar);
	void clear();
	typedef RunLenSumArray4 QueryTp;
private:
	unsigned int len, lastst, cnt;
	mscds::SDRankSelectBuilderSml stbd;
	mscds::SDArraySmlBuilder rlbd;
	mscds::SDArraySmlBuilder psbd, spsbd;
	PRSumArrBuilder vals;

	uint64_t psum, sqpsum;
	int64_t lastv;
};

class RunLenSumArray4 : public RunLenSumArrInt  {
public:
	RunLenSumArray4(): len(0) {};
	~RunLenSumArray4() { clear(); }
	size_t load(std::istream& fi);
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;

	uint64_t sum(uint32_t pos) const;
	int64_t sum_delta(uint32_t pos, int64_t delta) const;

	unsigned int length() const;
	void clear();

	unsigned int range_start(unsigned int i) const;
	uint64_t range_psum(unsigned int i) const;
	unsigned int range_len(unsigned int i) const;	
	unsigned int range_value(unsigned int i) const;
	unsigned int pslen(unsigned int i) const;
	unsigned int last_position() const { return length() > 0 ? range_start(length()-1) + range_len(length() - 1) : 0; }

	unsigned int count_range(unsigned int pos) const;

	/** \brief counts the number of non-zero numbers in the half-open range [0..p) */
	unsigned int countnz(unsigned int p) const;

	/** \brief returns the values of the integer at position `pos'. Note that
		 sequence index starts at 0 */
	unsigned int access(unsigned int) const;

	/** \brief returns largest position that is less than or equal to the input
		and its value is non-zero (returns -1 if cannot find) */
	int prev(unsigned int) const;

	/** \brief returns the smallest position that is greater than the input
		and its value is non-zero (return -1 if cannot find) */
	int next(unsigned int) const;
	typedef RunLenSumArrayBuilder4 BuilderTp;
private:
	unsigned int len;
	mscds::SDRankSelectSml start;
	mscds::SDArraySml rlen;

	mscds::SDArraySml psum, sqrsum;
	PRSumArr vals;
	friend class RunLenSumArrayBuilder4;
};

}//namespace
