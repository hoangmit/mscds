#pragma once

#include "sdarray_sml.h"
#include "codec/deltacoder.h"
#include "diffarray.hpp"

namespace mscds {

class GammaArray;

class GammaArrayBuilder {
public:
	void init(unsigned int rate) {}
	void add(uint64_t val);
	void build(GammaArray * out);
	void build(OArchive& ar);
	void clear() { upper.clear(); lower.clear(); }
	typedef GammaArray QueryTp;
private:
	SDArraySmlBuilder upper;
	OBitStream lower;
};

class GammaArray {
public:
	uint64_t lookup(uint64_t p) const;
	void save(OArchive& ar) const;
	void load(IArchive& ar);
	void clear();
	uint64_t length() const;
	typedef GammaArrayBuilder BuilderTp;
	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		Enum(const Enum& o) {}
		bool hasNext() const;
		uint64_t next();
	private:
		friend class GammaArray;
		SDArraySml::Enum e;
		unsigned int lpos;
		const BitArray * lower;
	};
	void getEnum(unsigned int pos, Enum * e) const;

private:
	friend class GammaArrayBuilder;
	SDArraySml upper;
	BitArray lower;
};


typedef DiffArray<GammaArray> GammaDiffDtArray;
typedef DiffArrayBuilder<GammaArray> GammaDiffArrBuilder;

}