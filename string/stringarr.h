#pragma once

/**  \file

Implement an array of strings
*/

#include "bitarray/bitarray.h"
#include "framework/archive.h"
#include "intarray/sdarray_sml.h"

#include <string>
#include <deque>
#include <memory>

namespace mscds {

class StringArr;


class StringArrBuilder {
public:
	void add(const std::string& s);
	void build(StringArr* out);
	void build(OutArchive& ar);

	template<typename ContainerTp = std::vector<std::string> >
	static void save(const ContainerTp& container, OutArchive& ar);

	template<typename ContainerTp = std::vector<std::string> >
	static void load(InpArchive& ar, ContainerTp* out);
private:
	std::deque<std::string> store;
};

class StringInt {
public:
	virtual const char* c_str() = 0;
	virtual size_t length() const = 0;
	virtual std::string str() {
		const char* p = c_str();
		return std::string(p, p + length());
	}
	virtual ~StringInt() {}
};

typedef std::shared_ptr<StringInt> StringPtr;

/// data structure pad 0 at the end of each string for cstring functions
class StringArr {
public:
	StringArr();
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
	friend class StringArrBuilder;
};

template<typename ContainerTp >
inline void StringArrBuilder::save(const ContainerTp& container, OutArchive& ar) {
	StringArrBuilder bd;
	for (auto it = container.cbegin(); it != container.cend(); ++it)
		bd.add(*it);
	bd.build(ar);
}

template<typename ContainerTp >
inline void StringArrBuilder::load(InpArchive& ar, ContainerTp* out) {
	ContainerTp tp;
	StringArr sa;
	sa.load(ar);
	for (unsigned i = 0; i < sa.length(); ++i)
		out->push_back(sa.get(i)->c_str());
}

}//namespace
