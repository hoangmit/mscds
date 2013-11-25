#pragma once

#include "intarray/sdarray_sml.h"
#include "bitarray/rank6p.h"
#include "tree/RMQ_pm1.h"
#include "bitarray/bitarray.h"
#include "framework/archive.h"
#include <algorithm>

namespace app_ds {
class IntvLst;

class IntvLstBuilder {
public:
	typedef unsigned int PosType;
	IntvLstBuilder() { clear(); }

	void clear();

	void add(PosType st, PosType ed);
	
	void build(IntvLst* out);
	void build(mscds::OArchive& ar);
	
	typedef IntvLst QueryTp;
private:
	struct PosData {
		PosData(){}
		PosData(unsigned int _id, PosType _pos, char _type) : id(_id), pos(_pos), type(_type) {}
		unsigned int id;
		PosType pos;
		char type;

		unsigned int pos_order, span;
	};
	unsigned int cur_id;
	PosType last_st, last_ed;

	size_t len;

	std::vector<PosData> lst;
};

class IntvLst {
public:
	typedef unsigned int PosType;
	void save(mscds::OArchive& ar) const;
	void load(mscds::IArchive& ar);
	void clear();

	std::pair<PosType, PosType> get(unsigned int i) const;
private:
	mscds::Rank6p marks;
	mscds::SDArraySml pos;
	mscds::SDArraySml span;

	mscds::RMQ_pm1_minmax minmax_depth;

	size_t len;
	friend class IntvLstBuilder;
};

}//namespace

