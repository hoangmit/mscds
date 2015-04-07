#pragma once


/**  \file

Implement an array of blob (binary large object)
*/

#include "bitarray/bitarray.h"
#include "framework/archive.h"
#include "intarray/sdarray_sml.h"

#include "stringarr.h"

#include <string>
#include <deque>
#include <memory>

namespace mscds {

class BlobArr;

class BlobArrBuilder {
public:
	void add(const std::string& s);
	void build(BlobArr* out);
	void build(mscds::OutArchive& ar);
private:
	std::deque<std::string> store;
};

class BlobArr {
public:
	BlobArr();
	StringPtr get(unsigned int i) const;
	std::string get_str(unsigned int i) const;
	size_t length() const { return cnt; }
	void load(mscds::InpArchive& ar);
	void save(mscds::OutArchive& ar) const;
	void dump(std::ostream& fo) const;
	void clear();
private:
	size_t cnt, tlen;
	const char * ptrs;
	bool mapping;
	mutable mscds::StaticMemRegionPtr ba;
	mscds::SDArraySml start;
	friend class BlobArrBuilder;
};

}//namespace