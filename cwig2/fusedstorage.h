#pragma once
#include "fusion/codec_block.h"
#include "fusion/ps_access_blk.h"
#include "cwig2/intv/nintv_fuse.h"

#include "float_int_map.h"
#include "cwig/valrange.h"

namespace app_ds {
class Storage;

class StorageBuilder {
	typedef mscds::LiftStBuilder<NIntvInterBlkBuilder,
		mscds::CodeInterBlkBuilder,
		mscds::PtrInterBlkBd,
		mscds::PtrInterBlkBd
	> BD;
public:
	void add(unsigned int st, unsigned int ed, double val);
	void build(Storage* qs);
	void build(mscds::OutArchive& ar);
	typedef Storage QueryTp;

	const static unsigned int SUM_GAP = 64;
private:
	std::deque<ValRange> data;
	FloatIntMapBuilder rvbd;
};

class Storage {
private:
	typedef mscds::LiftStQuery<app_ds::FuseNIntvInterBlock,
		mscds::CodeInterBlkQuery,
		mscds::PtrInterBlkQs,
		mscds::PtrInterBlkQs
	> QS;
	mutable QS data;
	FloatIntMapQuery fmap;
	friend class StorageBuilder;
public:
	typedef StorageBuilder BuilderTp;
	FuseNIntvInterBlock& itv;
	mscds::CodeInterBlkQuery& vals;
	mscds::PtrInterBlkQs& sumq;
	mscds::PtrInterBlkQs& sqrsum;
	
	Storage();

	void clear();
	void load(mscds::InpArchive &ar);
	void save(mscds::OutArchive &ar) const;

	double get_val(unsigned int idx) const;
	double get_sumq(unsigned int idx) const;
	double get_sqrsum(unsigned int idx) const;

	size_t length() const;

	class Enum: public mscds::EnumeratorInt<double> {
		mscds::CodeInterBlkQuery::Enum e;
		const Storage * self;
		friend class Storage;
	public:
		Enum() {}
		bool hasNext() const;
		double next();
	};
	const static unsigned int SUM_GAP = 64;
	void getEnum(size_t base, Enum *e) const;
};

}//namespace
