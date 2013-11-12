#pragma once

#include <stdint.h>
#include "framework/archive.h"

namespace app_ds {

template<typename T>
class RunLenSumArrIt {
public:
	virtual void load(mscds::IArchive& ar) = 0;

	virtual T sum(uint32_t pos) const = 0;

	virtual unsigned int length() const = 0;
	virtual void clear() = 0;

	virtual unsigned int range_start(unsigned int i) const = 0;
	virtual unsigned int range_len(unsigned int i) const = 0;
	virtual T range_value(unsigned int i) const = 0;
	virtual T range_psum(unsigned int i) const = 0;

	virtual unsigned int count_range(unsigned int pos) const = 0;

	/** \brief counts the number of non-zero numbers in the half-open range [0..p) */
	virtual unsigned int countnz(unsigned int p) const = 0;

	/** \brief returns the values of the integer at position `pos'. Note that
		 sequence index starts at 0 */
	virtual T access(unsigned int) const = 0;

	/** \brief returns largest position that is less than or equal to the input
		and its value is non-zero (returns -1 if cannot find) */
	virtual int prev(unsigned int) const = 0;

	/** \brief returns the smallest position that is greater than the input
		and its value is non-zero (return -1 if cannot find) */
	virtual int next(unsigned int) const = 0;
};

}//namespace