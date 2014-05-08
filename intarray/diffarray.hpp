#pragma once

/** 
Implement array of integers encoded using generic compression model to code 
the differences between two consecutive elements.

*/

//-----------------------------------------------------------------

#include <stdint.h>
#include "framework/archive.h"

#include "codec/deltacoder.h"
#include "intarray.h"
#include "utils/param.h"

namespace mscds {

template<typename IntArray>
class DiffArray;

template<typename IntArray>
class DiffArrayBuilder {
public:
	typedef DiffArray<IntArray> QueryTp;
	DiffArrayBuilder();
	void init(const Config * conf = NULL);
	void add(uint64_t val);
	void build(OutArchive& ar);
	void build(QueryTp * out);
	void clear();
private:
	unsigned int sample_rate, i;
	int64_t lastval;
	typename IntArray::BuilderTp bd;
};

template<typename IntArray>
class DiffArray {
public:
	DiffArray(){}
	uint64_t length() const;
	uint64_t lookup(uint64_t pos) const;
	uint64_t operator[](uint64_t pos) const { return lookup(pos); }

	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	typedef DiffArrayBuilder<IntArray> BuilderTp;
	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		bool hasNext() const;
		uint64_t next();
		Enum(const Enum& o): midx(o.midx), rate(o.rate), val(o.val) {}
	private:
		unsigned int midx, rate, len;
		int64_t val;
		typename IntArray::Enum e;
		friend class DiffArray<IntArray>;
	};
	void getEnum(uint64_t pos, Enum * e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	IntArray arr;
	//uint64_t len;
	unsigned int sample_rate;
	friend class DiffArrayBuilder<IntArray>;
};



}//namespace


namespace mscds {

template<typename IntArray>
mscds::DiffArrayBuilder<IntArray>::DiffArrayBuilder() {
	i = 0;
	lastval = 0;
	sample_rate = 64;
	Config conf;
	conf.add("SAMPLE_RATE", "64");
	bd.init(&conf);
}

template<typename IntArray>
void DiffArrayBuilder<IntArray>::init(const Config* conf) {
	clear();
	if (conf == NULL) {
		sample_rate = 64;
		Config conf;
		conf.add("SAMPLE_RATE", "64");
		bd.init(&conf);
	}
	else sample_rate = conf->getInt("SAMPLE_RATE", 64);
	bd.init(conf);
}

template<typename IntArray>
void DiffArrayBuilder<IntArray>::clear() {
	i = 0;
	lastval = 0;
	sample_rate = 0;
	bd.clear();
}

template<typename IntArray>
void DiffArrayBuilder<IntArray>::add(uint64_t val) {
	if (i % sample_rate == 0) {
		bd.add(val);
	} else {
		bd.add(coder::absmap((int64_t)val - lastval));
	}
	lastval = val;
	++i;
}

template<typename IntArray>
void DiffArrayBuilder<IntArray>::build(OutArchive &ar) {
	DiffArray<IntArray> tmp;
	build(&tmp);
	tmp.save(ar);
}

template<typename IntArray>
void DiffArrayBuilder<IntArray>::build(DiffArray<IntArray> *out) {
	out->clear();
	out->sample_rate = sample_rate;
	//out->len = i;
	bd.build(&(out->arr));
}
//-----------------------------------------
template<typename IntArray>
void DiffArray<IntArray>::getEnum(uint64_t pos, typename DiffArray<IntArray>::Enum *e) const {
	auto r = (pos % sample_rate);
	arr.getEnum(pos - r, &(e->e));
	assert(sample_rate > 0);

	e->val = e->e.next();
	for (unsigned int i = 0; i < r; ++i)
		e->val += coder::absunmap(e->e.next());

	e->midx = pos;
	e->len = length();
	e->rate = sample_rate;
}

template<typename IntArray>
bool DiffArray<IntArray>::Enum::hasNext() const {
	return midx <= len;
}

template<typename IntArray>
uint64_t DiffArray<IntArray>::Enum::next() {
	uint64_t ret = val;
	++midx;
	if (midx < len) {
		if (midx % rate != 0)
			val += coder::absunmap(e.next());
		else
			val = e.next();
	}
	return ret;
}

template<typename IntArray>
uint64_t DiffArray<IntArray>::lookup(uint64_t pos) const {
	Enum e;
	getEnum(pos, &e);
	return e.next();
}

template<typename IntArray>
uint64_t DiffArray<IntArray>::length() const { return arr.length(); }

template<typename IntArray>
void DiffArray<IntArray>::clear() {
	sample_rate = 0;
	arr.clear();
}

template<typename IntArray>
void DiffArray<IntArray>::save(OutArchive &ar) const {
	ar.startclass("diff_array", 1);
	ar.var("sample_rate").save(sample_rate);
	arr.save(ar.var("array"));
	ar.endclass();
}

template<typename IntArray>
void DiffArray<IntArray>::load(InpArchive &ar) {
	ar.loadclass("diff_array");
	ar.var("sample_rate").load(sample_rate);
	arr.load(ar.var("array"));
	ar.endclass();
}

template<typename IntArray>
void mscds::DiffArray<IntArray>::inspect(const std::string& cmd, std::ostream& out) const
{

}

}//namespace