#pragma once

#include "valrange.h"


#include "intarray/blkgroup_array.h"
#include "intarray/fuse_blk_model.h"
#include "cwig/intv/nintv_fuse.h"
#include "intarray/ps_access_blk.h"


namespace app_ds {

class IntValQuery;
class IntValBuilder {
public:
	typedef mscds::LiftStBuilder<NIntvInterBlkBuilder,
			mscds::CodeInterBlkBuilder,
			mscds::PtrInterBlkBd,
			mscds::PtrInterBlkBd
		> BD;
	typedef mscds::LiftStQuery<FuseNIntvInterBlock,
		mscds::CodeInterBlkQuery,
		mscds::PtrInterBlkQs,
		mscds::PtrInterBlkQs
	> QS;

	void comp_transform(const std::deque<ValRange>& all);

	void build(const std::deque<ValRange>& all, IntValQuery* qs);
private:
	uint64_t factor;
	int64_t delta;
};

class IntValQuery {
public:
	static const unsigned int rate = 64;
	typedef IntValBuilder::QS QS;

	double access(unsigned int idx);

	double sum(unsigned int idx, unsigned int leftpos = 0);

	double sqrSum(unsigned int idx, unsigned int leftpos = 0);

private:
	friend class IntValBuilder;

	uint64_t factor;
	int64_t delta;
	QS data;
};

}//namespace
