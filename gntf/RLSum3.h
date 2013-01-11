#pragma once

#include <deque>
#include <stdint.h>
#include "archive.h"

#include "sdarray/sdarray.h"
#include "sdarray/sdarray_sml.h"
#include "codec/deltacoder.h"


namespace app_ds {

class RunLenSumArray3;

class RunLenSumArrayBuilder3 {
public:
	RunLenSumArrayBuilder3(): len(0), lastst(0) {}
	void add(unsigned int st, unsigned int ed, unsigned int v);
	void build(RunLenSumArray3* arr);
	void build(mscds::OArchive& ar);
	void clear();
private:
	unsigned int len, lastst, cnt;
	mscds::SDArraySmlBuilder stbd, rlbd;
	mscds::SDArraySmlBuilder pspt;
	mscds::OBitStream enc;
	coder::DeltaCoder dc;
	uint64_t psum;
	int64_t lastv;
};

class RunLenSumArray3 {
public:
	RunLenSumArray3();
	~RunLenSumArray3() { clear(); }
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
private:
	unsigned int len;
	mscds::SDArraySml start, rlen;
	mscds::SDArraySml ptr;
	mscds::BitArray enc;
	friend class RunLenSumArrayBuilder3;
};

}//namespace
