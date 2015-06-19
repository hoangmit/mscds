#pragma once
#include <stdint.h>

namespace mscds {

struct SDArrayInterface {
	typedef uint64_t ValueTp;

	virtual ValueTp prefixsum(ValueTp p) const = 0;
	virtual ValueTp lookup(ValueTp p) const = 0;
	virtual ValueTp rank(ValueTp val) const = 0;
	
	virtual ValueTp length() const = 0;
	virtual ValueTp total() const = 0;
	
	virtual ValueTp rank2(ValueTp p, ValueTp& select) const {
		auto ret = rank(p);
		select = prefixsum(ret);
		return ret;
	}
	virtual ValueTp lookup(unsigned int p, ValueTp& prev_sum) const {
		prev_sum = prefixsum(p);
		return lookup(p);
	}

};

	
}//namespace
