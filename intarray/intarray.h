#pragma once

#include "archive.h"

namespace mscds {

template<typename T, typename Query>
class BuilderInt {
public:
	virtual void add(T v) = 0;
	virtual void build(Query* q) = 0;
	virtual void build(OArchive& ar) = 0;
};

template<typename T>
class EnumeratorInt {
public:
	virtual bool hasNext() const = 0;
	virtual T next() = 0;
};


}//namespace