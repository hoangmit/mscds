#pragma once

/**  \file
Variable length array
*/

#include "framework/archive.h"
#include "sdarray_sml.h"
#include <stdint.h>
#include <deque>


namespace mscds {

class VLenArrayBuilder;

/// Code length is stored using SDArray
class VLenArray {
public:
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	void clear() { codelen.clear(); code.clear(); opcode.clear(); op_bwidth = 0; }

	uint64_t length() const { return codelen.length(); }

	uint32_t lookup(unsigned int i) const;
	uint32_t operator[](unsigned int pos) const { return lookup(pos); }
	typedef GenericEnum<VLenArray, unsigned> Enum;

	void getEnum(unsigned int i, Enum* e) const {
		e->_set(this, i);
	}
	typedef VLenArrayBuilder BuilderTp;
private:
	SDArraySml codelen;
	BitArray code;
	std::vector<unsigned int> opcode;
	unsigned op_bwidth;
	friend class VLenArrayBuilder;
};

class VLenArrayBuilder {
public:
	typedef VLenArray QueryTp;
	void add(unsigned int val) {
		vals.push_back(val);
	}
    void build(QueryTp * out);
private:
	void _add(unsigned blen, unsigned value);

	OBitStream bout;
	SDArraySmlBuilder clen;

	std::deque<unsigned> vals;
};

}//namespace
