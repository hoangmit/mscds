#pragma once

#include "bitarray/bitarray.h"
#include "archive.h"
#include "sdarray/sdarray_sml.h"

#include <string>
#include <deque>

namespace app_ds {

class StringArr;

class StringArrBuilder {
public:
	void add(const std::string& s);
	void build(StringArr* out);
	void build(mscds::OArchive& ar);
private:
	std::deque<std::string> store;
};

class StringArr {
public:
	StringArr();
	const char* get(unsigned int i) const;
	size_t str_len(unsigned int) const;
	size_t length() const { return cnt; }
	void load(mscds::IArchive& ar);
	void save(mscds::OArchive& ar) const;
	void clear();
private:
	size_t cnt, tlen;
	const char * ptrs;
	mscds::SharedPtr ba;
	mscds::SDArraySml start;
	friend class StringArrBuilder;
};

}//namespace