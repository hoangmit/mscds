#pragma once

/** \file

Run Len compressed integer array
*/

#include "sdarray_sml.h"
#include "huffarray.h"
namespace mscds {

class RunLenBuilder;

/// Run Len compressed integer array
class RunLenArr {
public:
	RunLenArr(): rcnt(0) {}
	unsigned int lookup(unsigned int idx) const {
		assert(rcnt > 0);
		if (idx == 0)
			return data.lookup(0);
		unsigned int p = runlen.rank(idx + 1);
		if (p > 0) return data.lookup(p-1);
		else return data.lookup(0);
	}
	unsigned int operator[](unsigned int idx) const { return this->lookup(idx); }
	size_t length() const {
		return runlen.total();
	}
	typedef GenericEnum<RunLenArr, unsigned> Enum;

	void getEnum(unsigned int i, Enum* e) const {
		e->_set(this, i);
	}
	/** save and load functions */
	void save(OutArchive& ar) const {
		ar.startclass("RunLenArray", 1);
		ar.var("length").save(rcnt);
		data.save(ar.var("bits"));
		runlen.save(ar.var("table"));
		ar.endclass();
	}
	void load(InpArchive& ar) {
		ar.loadclass("RunLenArray");
		ar.var("length").load(rcnt);
		data.load(ar.var("bits"));
		runlen.load(ar.var("table"));
		ar.endclass();
	}
	void clear() {
		rcnt = 0;
		data.clear();
		runlen.clear();
	}
	typedef RunLenBuilder BuilderTp;
private:
	friend class RunLenBuilder;
	unsigned int rcnt;
	HuffmanArray data;
	SDArraySml runlen;
};

class RunLenBuilder {
public:
	RunLenBuilder() { cnt = 0; last = ~(0u); }

	void add(unsigned int val) {
		if (val != last) {
			if (cnt > 0) {
				data.add(last);
				runlen.add(cnt);
			}
			last = val;
			cnt = 1;
		} else { cnt += 1; }
		rcnt += 1;
	}

	void build(RunLenArr* out) {
		if (cnt > 0) {
			data.add(last);
			runlen.add(cnt);
		}
		out->rcnt = rcnt;
		data.build(&(out->data));
		runlen.build(&(out->runlen));
	}
private:
	unsigned int rcnt;
	unsigned int last, cnt;
	HuffmanArrBuilder data;
	SDArraySmlBuilder runlen;
};

}//namespace
