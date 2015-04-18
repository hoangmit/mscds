#pragma once



#include "framework/archive.h"
#include "sdarray_sml.h"
#include <stdint.h>
#include <deque>


namespace mscds {

class VLenArrayBuilder;

class VLenArray {
public:
	void load(InpArchive& ar);
	void save(OutArchive& ar) const;

	void clear() { codelen.clear(); code.clear(); opcode.clear(); op_bwidth = 0; }

	uint64_t length() const { return codelen.length(); }

	uint32_t lookup(unsigned int i) const {
		uint64_t ps;
		unsigned w = codelen.lookup(i, ps);
		if (w==0) {
			return opcode[0];
		} else
		if (w <= op_bwidth) {
			unsigned idx = code.bits(ps, w);
			return opcode[idx + (1 << w) - 1];
		} else {
			return code.bits(ps, w);
		}
	}
	uint32_t operator[](unsigned int pos) const { return lookup(pos); }
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
