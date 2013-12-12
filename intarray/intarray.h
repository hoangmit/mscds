#pragma once

#include "framework/archive.h"
#include "utils/param.h"

namespace mscds {

template<typename T, typename Query>
class BuilderInt {
public:
	virtual void init(const Config * conf = NULL) {}
	virtual void add(T v) = 0;
	virtual void build(Query* q) = 0;
	virtual void build(OutArchive& ar) = 0;
};

template<typename T>
class IntArr {
	virtual T lookup(uint64_t p) const;
	virtual uint64_t length() const;
};

template<typename T>
class EnumeratorInt {
public:
	virtual bool hasNext() const = 0;
	virtual T next() = 0;
};


}//namespace