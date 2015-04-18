#pragma once

/** \file

Interface of array of integers
*/

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
	virtual ~EnumeratorInt() {}
};

template<typename Base, typename T>
class GenericEnum: public EnumeratorInt<T> {
public:
	GenericEnum(): base(nullptr), i(0), sz(0) {}
	GenericEnum(const Base* b): base(b), i(0) {sz = base->length();}
	bool hasNext() const { return i < sz; }
	T next() { return base->lookup(i++); }
	void _set(const Base* b, size_t idx) { base = b; i = idx; sz = base->length(); }
private:
	size_t i, sz;
	const Base * base;
};

}//namespace