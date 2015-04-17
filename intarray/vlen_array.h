#pragma once

#include <stdint.h>
#include "framework/archive.h"

namespace mscds {

class VLenArrayBuilder;

class VLenArray {
public:
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	uint64_t length() const;
	uint32_t lookup(unsigned int i) const;
	uint32_t operator[](unsigned int pos) const { return lookup(pos); }
	typedef VLenArrayBuilder BuilderTp;
private:
	friend class VLenArrayBuilder;
};

class VLenArrayBuilder {
public:
	typedef VLenArray QueryTp;
	void add(unsigned int val);
	void build(QueryTp * out);
private:
};

}//namespace
