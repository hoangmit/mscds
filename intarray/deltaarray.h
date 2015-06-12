#pragma once

/**  \file

Implement two structures:

  Array of integers encoded using Elias Delta code.

  Array of integers encoded using Elias Delta code of the differences 
  between two consecutive elements.

This uses "blkarray" and "diffarray"

*/

#include <stdint.h>
#include "framework/archive.h"

#include "codec/deltacoder.h"
#include "bitarray/bitstream.h"

#include "intarray/sdarray_sml.h"
#include "intarray/intarray.h"
#include "intarray/diffarray.hpp"

namespace mscds {

class DeltaCodeArr;

class DeltaCodeArrBuilder {
public:
	DeltaCodeArrBuilder();
	/* parameters: SAMPLE_RATE = 32 */
	void init(const Config* conf = NULL);
	void add(uint64_t val);
	void build(OutArchive& ar);
	void build(DeltaCodeArr * out);
	void clear();
	typedef DeltaCodeArr QueryTp;
private:
	coder::DeltaCoder dc;
	OBitStream enc;
	SDArraySmlBuilder ptrbd;
	unsigned int sample_rate, i;
};


/// Delta encoded array of integers
class DeltaCodeArr {
public:
	uint64_t lookup(uint64_t pos) const;
	uint64_t operator[](uint64_t pos) const { return lookup(pos); }
	uint64_t length() const { return len; }
	
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;
	void clear();
	typedef DeltaCodeArrBuilder BuilderTp;

	struct Enum: public EnumeratorInt<uint64_t> {
	public:
		Enum() {}
		Enum(const Enum& o): is(o.is), c(o.c) {}
		bool hasNext() const;
		uint64_t next();
	private:
		mscds::IWBitStream is;
		coder::DeltaCoder dc;
		mutable coder::CodePr c;
		friend class DeltaCodeArr;
	};
	void getEnum(uint64_t pos, Enum * e) const;
	void inspect(const std::string& cmd, std::ostream& out) const;
private:
	uint64_t len;
	unsigned int sample_rate;
	SDArraySml ptr;
	BitArray enc;
	friend class DeltaCodeArrBuilder;
};


typedef DiffArray<DeltaCodeArr> DiffDeltaArr;
typedef DiffArrayBuilder<DeltaCodeArr> DiffDeltaArrBuilder;


}